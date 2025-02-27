/* $OpenBSD: fuse_device.c,v 1.40 2023/12/16 22:17:08 mvs Exp $ */
/*
 * Copyright (c) 2012-2013 Sylvestre Gallon <ccna.syl@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/event.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/rwlock.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/vnode.h>
#include <sys/fusebuf.h>

#include "fusefs_node.h"
#include "fusefs.h"

#ifdef	FUSE_DEBUG
#define	DPRINTF(fmt, arg...)	printf("%s: " fmt, "fuse", ##arg)
#else
#define	DPRINTF(fmt, arg...)
#endif

/*
 * Locks used to protect struct members and global data
 *	l	fd_lock
 */

SIMPLEQ_HEAD(fusebuf_head, fusebuf);

struct fuse_d {
	struct rwlock fd_lock;
	struct fusefs_mnt *fd_fmp;
	int fd_unit;

	/*fusebufs queues*/
	struct fusebuf_head fd_fbufs_in;	/* [l] */
	struct fusebuf_head fd_fbufs_wait;

	/* kq fields */
	struct klist fd_rklist;			/* [l] */
	LIST_ENTRY(fuse_d) fd_list;
};

int stat_fbufs_in = 0;
int stat_fbufs_wait = 0;
int stat_opened_fusedev = 0;

LIST_HEAD(, fuse_d) fuse_d_list;
struct fuse_d *fuse_lookup(int);

void	fuseattach(int);
int	fuseopen(dev_t, int, int, struct proc *);
int	fuseclose(dev_t, int, int, struct proc *);
int	fuseioctl(dev_t, u_long, caddr_t, int, struct proc *);
int	fuseread(dev_t, struct uio *, int);
int	fusewrite(dev_t, struct uio *, int);
int	fusekqfilter(dev_t dev, struct knote *kn);
int	filt_fuse_read(struct knote *, long);
void	filt_fuse_rdetach(struct knote *);
int	filt_fuse_modify(struct kevent *, struct knote *);
int	filt_fuse_process(struct knote *, struct kevent *);

static const struct filterops fuse_rd_filtops = {
	.f_flags	= FILTEROP_ISFD | FILTEROP_MPSAFE,
	.f_attach	= NULL,
	.f_detach	= filt_fuse_rdetach,
	.f_event	= filt_fuse_read,
	.f_modify	= filt_fuse_modify,
	.f_process	= filt_fuse_process,
};

#ifdef FUSE_DEBUG
static void
fuse_dump_buff(char *buff, int len)
{
	char text[17];
	int i;

	if (len < 0) {
		printf("invalid len: %d", len);
		return;
	}
	if (buff == NULL) {
		printf("invalid buff");
		return;
	}

	memset(text, 0, 17);
	for (i = 0; i < len; i++) {
		if (i != 0 && (i % 16) == 0) {
			printf(": %s\n", text);
			memset(text, 0, 17);
		}

		printf("%.2x ", buff[i] & 0xff);

		if (buff[i] > ' ' && buff[i] < '~')
			text[i%16] = buff[i] & 0xff;
		else
			text[i%16] = '.';
	}

	if ((i % 16) != 0)
		while ((i % 16) != 0) {
			printf("   ");
			i++;
		}

	printf(": %s\n", text);
}
#endif

struct fuse_d *
fuse_lookup(int unit)
{
	struct fuse_d *fd;

	LIST_FOREACH(fd, &fuse_d_list, fd_list)
		if (fd->fd_unit == unit)
			return (fd);
	return (NULL);
}

/*
 * Cleanup all msgs from sc_fbufs_in and sc_fbufs_wait.
 */
void
fuse_device_cleanup(dev_t dev)
{
	struct fuse_d *fd;
	struct fusebuf *f, *ftmp, *lprev;

	fd = fuse_lookup(minor(dev));
	if (fd == NULL)
		return;

	/* clear FIFO IN */
	lprev = NULL;
	rw_enter_write(&fd->fd_lock);
	SIMPLEQ_FOREACH_SAFE(f, &fd->fd_fbufs_in, fb_next, ftmp) {
		DPRINTF("cleanup unprocessed msg in sc_fbufs_in\n");
		if (lprev == NULL)
			SIMPLEQ_REMOVE_HEAD(&fd->fd_fbufs_in, fb_next);
		else
			SIMPLEQ_REMOVE_AFTER(&fd->fd_fbufs_in, lprev,
			    fb_next);

		stat_fbufs_in--;
		f->fb_err = ENXIO;
		wakeup(f);
		lprev = f;
	}
	rw_exit_write(&fd->fd_lock);

	/* clear FIFO WAIT*/
	lprev = NULL;
	SIMPLEQ_FOREACH_SAFE(f, &fd->fd_fbufs_wait, fb_next, ftmp) {
		DPRINTF("umount unprocessed msg in sc_fbufs_wait\n");
		if (lprev == NULL)
			SIMPLEQ_REMOVE_HEAD(&fd->fd_fbufs_wait, fb_next);
		else
			SIMPLEQ_REMOVE_AFTER(&fd->fd_fbufs_wait, lprev,
			    fb_next);

		stat_fbufs_wait--;
		f->fb_err = ENXIO;
		wakeup(f);
		lprev = f;
	}
}

void
fuse_device_queue_fbuf(dev_t dev, struct fusebuf *fbuf)
{
	struct fuse_d *fd;

	fd = fuse_lookup(minor(dev));
	if (fd == NULL)
		return;

	rw_enter_write(&fd->fd_lock);
	SIMPLEQ_INSERT_TAIL(&fd->fd_fbufs_in, fbuf, fb_next);
	knote_locked(&fd->fd_rklist, 0);
	rw_exit_write(&fd->fd_lock);
	stat_fbufs_in++;
}

void
fuse_device_set_fmp(struct fusefs_mnt *fmp, int set)
{
	struct fuse_d *fd;

	fd = fuse_lookup(minor(fmp->dev));
	if (fd == NULL)
		return;

	fd->fd_fmp = set ? fmp : NULL;
}

void
fuseattach(int num)
{
	LIST_INIT(&fuse_d_list);
}

int
fuseopen(dev_t dev, int flags, int fmt, struct proc * p)
{
	struct fuse_d *fd;
	int unit = minor(dev);

	if (flags & O_EXCL)
		return (EBUSY); /* No exclusive opens */

	if ((fd = fuse_lookup(unit)) != NULL)
		return (EBUSY);

	fd = malloc(sizeof(*fd), M_DEVBUF, M_WAITOK | M_ZERO);
	fd->fd_unit = unit;
	SIMPLEQ_INIT(&fd->fd_fbufs_in);
	SIMPLEQ_INIT(&fd->fd_fbufs_wait);
	rw_init(&fd->fd_lock, "fusedlk");
	klist_init_rwlock(&fd->fd_rklist, &fd->fd_lock);

	LIST_INSERT_HEAD(&fuse_d_list, fd, fd_list);

	stat_opened_fusedev++;
	return (0);
}

int
fuseclose(dev_t dev, int flags, int fmt, struct proc *p)
{
	struct fuse_d *fd;
	int error;

	fd = fuse_lookup(minor(dev));
	if (fd == NULL)
		return (EINVAL);

	if (fd->fd_fmp) {
		printf("fuse: device close without umount\n");
		fd->fd_fmp->sess_init = 0;
		fuse_device_cleanup(dev);
		if ((vfs_busy(fd->fd_fmp->mp, VB_WRITE | VB_NOWAIT)) != 0)
			goto end;
		error = dounmount(fd->fd_fmp->mp, MNT_FORCE, p);
		if (error)
			printf("fuse: unmount failed with error %d\n", error);
		fd->fd_fmp = NULL;
	}

end:
	LIST_REMOVE(fd, fd_list);
	free(fd, M_DEVBUF, sizeof(*fd));
	stat_opened_fusedev--;
	return (0);
}

/*
 * FIOCGETFBDAT		Get fusebuf data from kernel to user
 * FIOCSETFBDAT		Set fusebuf data from user to kernel
 */
int
fuseioctl(dev_t dev, u_long cmd, caddr_t addr, int flags, struct proc *p)
{
	struct fb_ioctl_xch *ioexch;
	struct fusebuf *lastfbuf;
	struct fusebuf *fbuf;
	struct fuse_d *fd;
	int error = 0;

	fd = fuse_lookup(minor(dev));
	if (fd == NULL)
		return (ENXIO);

	switch (cmd) {
	case FIOCGETFBDAT:
		ioexch = (struct fb_ioctl_xch *)addr;

		/* Looking for uuid in fd_fbufs_in */
		rw_enter_write(&fd->fd_lock);
		SIMPLEQ_FOREACH(fbuf, &fd->fd_fbufs_in, fb_next) {
			if (fbuf->fb_uuid == ioexch->fbxch_uuid)
				break;

			lastfbuf = fbuf;
		}
		if (fbuf == NULL) {
			rw_exit_write(&fd->fd_lock);
			printf("fuse: Cannot find fusebuf\n");
			return (EINVAL);
		}

		/* Remove the fbuf from fd_fbufs_in */
		if (fbuf == SIMPLEQ_FIRST(&fd->fd_fbufs_in))
			SIMPLEQ_REMOVE_HEAD(&fd->fd_fbufs_in, fb_next);
		else
			SIMPLEQ_REMOVE_AFTER(&fd->fd_fbufs_in, lastfbuf,
			    fb_next);
		rw_exit_write(&fd->fd_lock);

		stat_fbufs_in--;

		/* Do not handle fbufs with bad len */
		if (fbuf->fb_len != ioexch->fbxch_len) {
			printf("fuse: Bad fusebuf len\n");
			return (EINVAL);
		}

		/* Update the userland fbuf */
		error = copyout(fbuf->fb_dat, ioexch->fbxch_data,
		    ioexch->fbxch_len);
		if (error) {
			printf("fuse: cannot copyout\n");
			return (error);
		}

#ifdef FUSE_DEBUG
		fuse_dump_buff(fbuf->fb_dat, fbuf->fb_len);
#endif

		/* Adding fbuf in fd_fbufs_wait */
		free(fbuf->fb_dat, M_FUSEFS, fbuf->fb_len);
		fbuf->fb_dat = NULL;
		SIMPLEQ_INSERT_TAIL(&fd->fd_fbufs_wait, fbuf, fb_next);
		stat_fbufs_wait++;
		break;

	case FIOCSETFBDAT:
		DPRINTF("SET BUFFER\n");
		ioexch = (struct fb_ioctl_xch *)addr;

		/* looking for uuid in fd_fbufs_wait */
		SIMPLEQ_FOREACH(fbuf, &fd->fd_fbufs_wait, fb_next) {
			if (fbuf->fb_uuid == ioexch->fbxch_uuid)
				break;

			lastfbuf = fbuf;
		}
		if (fbuf == NULL) {
			printf("fuse: Cannot find fusebuf\n");
			return (EINVAL);
		}

		/* Do not handle fbufs with bad len */
		if (fbuf->fb_len != ioexch->fbxch_len) {
			printf("fuse: Bad fusebuf size\n");
			return (EINVAL);
		}

		/* fetching data from userland */
		fbuf->fb_dat = malloc(ioexch->fbxch_len, M_FUSEFS,
		    M_WAITOK | M_ZERO);
		error = copyin(ioexch->fbxch_data, fbuf->fb_dat,
		    ioexch->fbxch_len);
		if (error) {
			printf("fuse: Cannot copyin\n");
			free(fbuf->fb_dat, M_FUSEFS, fbuf->fb_len);
			fbuf->fb_dat = NULL;
			return (error);
		}

#ifdef FUSE_DEBUG
		fuse_dump_buff(fbuf->fb_dat, fbuf->fb_len);
#endif

		/* Remove fbuf from fd_fbufs_wait */
		if (fbuf == SIMPLEQ_FIRST(&fd->fd_fbufs_wait))
			SIMPLEQ_REMOVE_HEAD(&fd->fd_fbufs_wait, fb_next);
		else
			SIMPLEQ_REMOVE_AFTER(&fd->fd_fbufs_wait, lastfbuf,
			    fb_next);
		stat_fbufs_wait--;
		wakeup(fbuf);
		break;
	default:
		error = EINVAL;
	}

	return (error);
}

int
fuseread(dev_t dev, struct uio *uio, int ioflag)
{
	struct fuse_d *fd;
	struct fusebuf *fbuf;
	struct fb_hdr hdr;
	void *tmpaddr;
	int error = 0;

	/* We get the whole fusebuf or nothing */
	if (uio->uio_resid != FUSEBUFSIZE)
		return (EINVAL);

	fd = fuse_lookup(minor(dev));
	if (fd == NULL)
		return (ENXIO);

	rw_enter_write(&fd->fd_lock);

	if (SIMPLEQ_EMPTY(&fd->fd_fbufs_in)) {
		if (ioflag & O_NONBLOCK)
			error = EAGAIN;
		goto end;
	}
	fbuf = SIMPLEQ_FIRST(&fd->fd_fbufs_in);

	/* Do not send kernel pointers */
	memcpy(&hdr.fh_next, &fbuf->fb_next, sizeof(fbuf->fb_next));
	memset(&fbuf->fb_next, 0, sizeof(fbuf->fb_next));
	tmpaddr = fbuf->fb_dat;
	fbuf->fb_dat = NULL;
	error = uiomove(fbuf, FUSEBUFSIZE, uio);
	if (error)
		goto end;

#ifdef FUSE_DEBUG
	fuse_dump_buff((char *)fbuf, FUSEBUFSIZE);
#endif
	/* Restore kernel pointers */
	memcpy(&fbuf->fb_next, &hdr.fh_next, sizeof(fbuf->fb_next));
	fbuf->fb_dat = tmpaddr;

	/* Remove the fbuf if it does not contains data */
	if (fbuf->fb_len == 0) {
		SIMPLEQ_REMOVE_HEAD(&fd->fd_fbufs_in, fb_next);
		stat_fbufs_in--;
		SIMPLEQ_INSERT_TAIL(&fd->fd_fbufs_wait, fbuf, fb_next);
		stat_fbufs_wait++;
	}

end:
	rw_exit_write(&fd->fd_lock);
	return (error);
}

int
fusewrite(dev_t dev, struct uio *uio, int ioflag)
{
	struct fusebuf *lastfbuf;
	struct fuse_d *fd;
	struct fusebuf *fbuf;
	struct fb_hdr hdr;
	int error = 0;

	fd = fuse_lookup(minor(dev));
	if (fd == NULL)
		return (ENXIO);

	/* We get the whole fusebuf or nothing */
	if (uio->uio_resid != FUSEBUFSIZE)
		return (EINVAL);

	if ((error = uiomove(&hdr, sizeof(hdr), uio)) != 0)
		return (error);

	/* looking for uuid in fd_fbufs_wait */
	SIMPLEQ_FOREACH(fbuf, &fd->fd_fbufs_wait, fb_next) {
		if (fbuf->fb_uuid == hdr.fh_uuid)
			break;

		lastfbuf = fbuf;
	}
	if (fbuf == NULL)
		return (EINVAL);

	/* Update fb_hdr */
	fbuf->fb_len = hdr.fh_len;
	fbuf->fb_err = hdr.fh_err;
	fbuf->fb_ino = hdr.fh_ino;

	/* Check for corrupted fbufs */
	if ((fbuf->fb_len && fbuf->fb_err) ||
	    SIMPLEQ_EMPTY(&fd->fd_fbufs_wait)) {
		printf("fuse: dropping corrupted fusebuf\n");
		error = EINVAL;
		goto end;
	}

	/* Get the missing data from the fbuf */
	error = uiomove(&fbuf->FD, uio->uio_resid, uio);
	if (error)
		return error;
	fbuf->fb_dat = NULL;
#ifdef FUSE_DEBUG
	fuse_dump_buff((char *)fbuf, FUSEBUFSIZE);
#endif

	switch (fbuf->fb_type) {
	case FBT_INIT:
		fd->fd_fmp->sess_init = 1;
		break ;
	case FBT_DESTROY:
		fd->fd_fmp = NULL;
		break ;
	}
end:
	/* Remove the fbuf if it does not contains data */
	if (fbuf->fb_len == 0) {
		if (fbuf == SIMPLEQ_FIRST(&fd->fd_fbufs_wait))
			SIMPLEQ_REMOVE_HEAD(&fd->fd_fbufs_wait, fb_next);
		else
			SIMPLEQ_REMOVE_AFTER(&fd->fd_fbufs_wait, lastfbuf,
			    fb_next);
		stat_fbufs_wait--;
		if (fbuf->fb_type == FBT_INIT)
			fb_delete(fbuf);
		else
			wakeup(fbuf);
	}

	return (error);
}

int
fusekqfilter(dev_t dev, struct knote *kn)
{
	struct fuse_d *fd;
	struct klist *klist;

	fd = fuse_lookup(minor(dev));
	if (fd == NULL)
		return (EINVAL);

	switch (kn->kn_filter) {
	case EVFILT_READ:
		klist = &fd->fd_rklist;
		kn->kn_fop = &fuse_rd_filtops;
		break;
	case EVFILT_WRITE:
		return (seltrue_kqfilter(dev, kn));
	default:
		return (EINVAL);
	}

	kn->kn_hook = fd;

	klist_insert(klist, kn);

	return (0);
}

void
filt_fuse_rdetach(struct knote *kn)
{
	struct fuse_d *fd = kn->kn_hook;
	struct klist *klist = &fd->fd_rklist;

	klist_remove(klist, kn);
}

int
filt_fuse_read(struct knote *kn, long hint)
{
	struct fuse_d *fd = kn->kn_hook;
	int event = 0;

	rw_assert_wrlock(&fd->fd_lock);

	if (!SIMPLEQ_EMPTY(&fd->fd_fbufs_in))
		event = 1;

	return (event);
}

int
filt_fuse_modify(struct kevent *kev, struct knote *kn)
{
	struct fuse_d *fd = kn->kn_hook;
	int active;

	rw_enter_write(&fd->fd_lock);
	active = knote_modify(kev, kn);
	rw_exit_write(&fd->fd_lock);

	return (active);
}

int
filt_fuse_process(struct knote *kn, struct kevent *kev)
{
	struct fuse_d *fd = kn->kn_hook;
	int active;

	rw_enter_write(&fd->fd_lock);
	active = knote_process(kn, kev);
	rw_exit_write(&fd->fd_lock);

	return (active);
}
