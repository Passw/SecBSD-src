/*	$OpenBSD: isa_machdep.c,v 1.31 2020/09/29 03:06:34 guenther Exp $	*/
/*	$NetBSD: isa_machdep.c,v 1.22 1997/06/12 23:57:32 thorpej Exp $	*/

#define ISA_DMA_STATS

/*-
 * Copyright (c) 1996, 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe of the Numerical Aerospace Simulation Facility,
 * NASA Ames Research Center.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*-
 * Copyright (c) 1993, 1994, 1996, 1997
 *	Charles M. Hannum.  All rights reserved.
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)isa.c	7.2 (Berkeley) 5/13/91
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/syslog.h>
#include <sys/malloc.h>
#include <sys/proc.h>

#include <uvm/uvm_extern.h>

#include "ioapic.h"

#if NIOAPIC > 0
#include <machine/i82093var.h>
#include <machine/mpbiosvar.h>
#endif

#include <machine/intr.h>
#include <machine/i8259.h>

#include <dev/isa/isavar.h>

#include "isadma.h"

extern	paddr_t avail_end;

#if NISADMA > 0
int	_isa_bus_dmamap_create(bus_dma_tag_t, bus_size_t, int,
	    bus_size_t, bus_size_t, int, bus_dmamap_t *);
void	_isa_bus_dmamap_destroy(bus_dma_tag_t, bus_dmamap_t);
int	_isa_bus_dmamap_load(bus_dma_tag_t, bus_dmamap_t, void *,
	    bus_size_t, struct proc *, int);
int	_isa_bus_dmamap_load_mbuf(bus_dma_tag_t, bus_dmamap_t,
	    struct mbuf *, int);
int	_isa_bus_dmamap_load_uio(bus_dma_tag_t, bus_dmamap_t,
	    struct uio *, int);
int	_isa_bus_dmamap_load_raw(bus_dma_tag_t, bus_dmamap_t,
	    bus_dma_segment_t *, int, bus_size_t, int);
void	_isa_bus_dmamap_unload(bus_dma_tag_t, bus_dmamap_t);
void	_isa_bus_dmamap_sync(bus_dma_tag_t, bus_dmamap_t,
	    bus_addr_t, bus_size_t, int);

int	_isa_bus_dmamem_alloc(bus_dma_tag_t, bus_size_t, bus_size_t,
	    bus_size_t, bus_dma_segment_t *, int, int *, int);

int	_isa_dma_check_buffer(void *, bus_size_t, int, bus_size_t,
	    struct proc *);
int	_isa_dma_alloc_bouncebuf(bus_dma_tag_t, bus_dmamap_t,
	    bus_size_t, int);
void	_isa_dma_free_bouncebuf(bus_dma_tag_t, bus_dmamap_t);

/*
 * Entry points for ISA DMA.  These are mostly wrappers around
 * the generic functions that understand how to deal with bounce
 * buffers, if necessary.
 */
struct bus_dma_tag isa_bus_dma_tag = {
	NULL,			/* _cookie */
	_isa_bus_dmamap_create,
	_isa_bus_dmamap_destroy,
	_isa_bus_dmamap_load,
	_isa_bus_dmamap_load_mbuf,
	_isa_bus_dmamap_load_uio,
	_isa_bus_dmamap_load_raw,
	_isa_bus_dmamap_unload,
	_isa_bus_dmamap_sync,
	_isa_bus_dmamem_alloc,
	_bus_dmamem_alloc_range,
	_bus_dmamem_free,
	_bus_dmamem_map,
	_bus_dmamem_unmap,
	_bus_dmamem_mmap,
};
#endif /* NISADMA > 0 */

int intrtype[ICU_LEN], intrlevel[ICU_LEN];
struct intrhand *intrhand[ICU_LEN];

#define	LEGAL_IRQ(x)	((x) >= 0 && (x) < ICU_LEN && (x) != 2)

int
isa_intr_alloc(isa_chipset_tag_t ic, int mask, int type, int *irq)
{
	int i, bestirq, count;
	int tmp;
	struct intrhand **p, *q;

	if (type == IST_NONE)
		panic("intr_alloc: bogus type");

	bestirq = -1;
	count = -1;

	/* some interrupts should never be dynamically allocated */
	mask &= 0xdef8;

	/*
	 * XXX some interrupts will be used later (6 for fdc, 12 for pms).
	 * the right answer is to do "breadth-first" searching of devices.
	 */
	mask &= 0xefbf;

	for (i = 0; i < ICU_LEN; i++) {
		if (LEGAL_IRQ(i) == 0 || (mask & (1<<i)) == 0)
			continue;

		switch(intrtype[i]) {
		case IST_NONE:
			/*
			 * if nothing's using the irq, just return it
			 */
			*irq = i;
			return (0);

		case IST_EDGE:
		case IST_LEVEL:
			if (type != intrtype[i])
				continue;
			/*
			 * if the irq is shareable, count the number of other
			 * handlers, and if it's smaller than the last irq like
			 * this, remember it
			 *
			 * XXX We should probably also consider the
			 * interrupt level and stick IPL_TTY with other
			 * IPL_TTY, etc.
			 */
			for (p = &intrhand[i], tmp = 0; (q = *p) != NULL;
			     p = &q->ih_next, tmp++)
				;
			if ((bestirq == -1) || (count > tmp)) {
				bestirq = i;
				count = tmp;
			}
			break;

		case IST_PULSE:
			/* this just isn't shareable */
			continue;
		}
	}

	if (bestirq == -1)
		return (1);

	*irq = bestirq;

	return (0);
}

/*
 * Just check to see if an IRQ is available/can be shared.
 * 0 = interrupt not available
 * 1 = interrupt shareable
 * 2 = interrupt all to ourself
 */
int
isa_intr_check(isa_chipset_tag_t ic, int irq, int type)
{
	if (!LEGAL_IRQ(irq) || type == IST_NONE)
		return (0);

	switch (intrtype[irq]) {
	case IST_NONE:
		return (2);
		break;
	case IST_LEVEL:
		if (type != intrtype[irq])
			return (0);
		return (1);
		break;
	case IST_EDGE:
	case IST_PULSE:
		if (type != IST_NONE)
			return (0);
	}
	return (1);
}

/*
 * Set up an interrupt handler to start being called.
 * XXX PRONE TO RACE CONDITIONS, UGLY, 'INTERESTING' INSERTION ALGORITHM.
 */
void *
isa_intr_establish(isa_chipset_tag_t ic, int irq, int type, int level,
    int (*ih_fun)(void *), void *ih_arg, char *ih_what)
{
	struct pic *pic = &i8259_pic;
	int pin = irq;

#if NIOAPIC > 0
	struct mp_intr_map *mip;

 	if (mp_busses != NULL) {
		if (mp_isa_bus == NULL)
			panic("no isa bus");

		for (mip = mp_isa_bus->mb_intrs; mip != NULL;
		    mip = mip->next) {
 			if (mip->bus_pin == pin) {
				pin = APIC_IRQ_PIN(mip->ioapic_ih);
				pic = &mip->ioapic->sc_pic;
 				break;
 			}
 		}
 	}
#endif

	KASSERT(pic);

	return intr_establish(irq, pic, pin, type, level, NULL, ih_fun,
	    ih_arg, ih_what);
}

/*
 * Deregister an interrupt handler.
 */
void
isa_intr_disestablish(isa_chipset_tag_t ic, void *arg)
{
	intr_disestablish(arg);
	return;
}

void
isa_attach_hook(struct device *parent, struct device *self,
    struct isabus_attach_args *iba)
{
	extern int isa_has_been_seen;

	/*
	 * Notify others that might need to know that the ISA bus
	 * has now been attached.
	 */
	if (isa_has_been_seen)
		panic("isaattach: ISA bus already seen!");
	isa_has_been_seen = 1;
}

#if NISADMA > 0
/**********************************************************************
 * bus.h dma interface entry points
 **********************************************************************/

#ifdef ISA_DMA_STATS
#define	STAT_INCR(v)	(v)++
#define	STAT_DECR(v)	do { \
		if ((v) == 0) \
			printf("%s:%d -- Already 0!\n", __FILE__, __LINE__); \
		else \
			(v)--; \
		} while (0)
u_long	isa_dma_stats_loads;
u_long	isa_dma_stats_bounces;
u_long	isa_dma_stats_nbouncebufs;
#else
#define	STAT_INCR(v)
#define	STAT_DECR(v)
#endif

/*
 * Create an ISA DMA map.
 */
int
_isa_bus_dmamap_create(bus_dma_tag_t t, bus_size_t size, int nsegments,
    bus_size_t maxsegsz, bus_size_t boundary, int flags, bus_dmamap_t *dmamp)
{
	struct isa_dma_cookie *cookie;
	bus_dmamap_t map;
	int error, cookieflags;
	void *cookiestore;
	size_t cookiesize;

	/* Call common function to create the basic map. */
	error = _bus_dmamap_create(t, size, nsegments, maxsegsz, boundary,
	    flags, dmamp);
	if (error)
		return (error);

	map = *dmamp;
	map->_dm_cookie = NULL;

	cookiesize = sizeof(struct isa_dma_cookie);

	/*
	 * ISA only has 24-bits of address space.  This means
	 * we can't DMA to pages over 16M.  In order to DMA to
	 * arbitrary buffers, we use "bounce buffers" - pages
	 * in memory below the 16M boundary.  On DMA reads,
	 * DMA happens to the bounce buffers, and is copied into
	 * the caller's buffer.  On writes, data is copied into
	 * the bounce buffer, and the DMA happens from those
	 * pages.  To software using the DMA mapping interface,
	 * this looks simply like a data cache.
	 *
	 * If we have more than 16M of RAM in the system, we may
	 * need bounce buffers.  We check and remember that here.
	 *
	 * There are exceptions, however.  VLB devices can do
	 * 32-bit DMA, and indicate that here.
	 *
	 * ...or, there is an opposite case.  The most segments
	 * a transfer will require is (maxxfer / NBPG) + 1.  If
	 * the caller can't handle that many segments (e.g. the
	 * ISA DMA controller), we may have to bounce it as well.
	 */
	cookieflags = 0;
	if ((avail_end > ISA_DMA_BOUNCE_THRESHOLD &&
	    (flags & ISABUS_DMA_32BIT) == 0) ||
	    ((map->_dm_size / NBPG) + 1) > map->_dm_segcnt) {
		cookieflags |= ID_MIGHT_NEED_BOUNCE;
		cookiesize += (sizeof(bus_dma_segment_t) * map->_dm_segcnt);
	}

	/*
	 * Allocate our cookie.
	 */
	if ((cookiestore = malloc(cookiesize, M_DEVBUF,
	    (flags & BUS_DMA_NOWAIT) ?
	        (M_NOWAIT|M_ZERO) : (M_WAITOK|M_ZERO))) == NULL) {
		error = ENOMEM;
		goto out;
	}
	cookie = (struct isa_dma_cookie *)cookiestore;
	cookie->id_flags = cookieflags;
	map->_dm_cookie = cookie;

	if (cookieflags & ID_MIGHT_NEED_BOUNCE) {
		/*
		 * Allocate the bounce pages now if the caller
		 * wishes us to do so.
		 */
		if ((flags & BUS_DMA_ALLOCNOW) == 0)
			goto out;

		error = _isa_dma_alloc_bouncebuf(t, map, size, flags);
	}

 out:
	if (error) {
		free(map->_dm_cookie, M_DEVBUF, cookiesize);
		_bus_dmamap_destroy(t, map);
	}
	return (error);
}

/*
 * Destroy an ISA DMA map.
 */
void
_isa_bus_dmamap_destroy(bus_dma_tag_t t, bus_dmamap_t map)
{
	struct isa_dma_cookie *cookie = map->_dm_cookie;

	/*
	 * Free any bounce pages this map might hold.
	 */
	if (cookie->id_flags & ID_HAS_BOUNCE)
		_isa_dma_free_bouncebuf(t, map);

	free(cookie, M_DEVBUF, 0);
	_bus_dmamap_destroy(t, map);
}

/*
 * Load an ISA DMA map with a linear buffer.
 */
int
_isa_bus_dmamap_load(bus_dma_tag_t t, bus_dmamap_t map, void *buf,
    bus_size_t buflen, struct proc *p, int flags)
{
	struct isa_dma_cookie *cookie = map->_dm_cookie;
	int error;

	STAT_INCR(isa_dma_stats_loads);

	/*
	 * Check to see if we might need to bounce the transfer.
	 */
	if (cookie->id_flags & ID_MIGHT_NEED_BOUNCE) {
		/*
		 * Check if all pages are below the bounce
		 * threshold.  If they are, don't bother bouncing.
		 */
		if (_isa_dma_check_buffer(buf, buflen,
		    map->_dm_segcnt, map->_dm_boundary, p) == 0)
			return (_bus_dmamap_load(t, map, buf, buflen,
			    p, flags));

		STAT_INCR(isa_dma_stats_bounces);

		/*
		 * Allocate bounce pages, if necessary.
		 */
		if ((cookie->id_flags & ID_HAS_BOUNCE) == 0) {
			error = _isa_dma_alloc_bouncebuf(t, map, buflen,
			    flags);
			if (error)
				return (error);
		}

		/*
		 * Cache a pointer to the caller's buffer and
		 * load the DMA map with the bounce buffer.
		 */
		cookie->id_origbuf = buf;
		cookie->id_origbuflen = buflen;
		error = _bus_dmamap_load(t, map, cookie->id_bouncebuf,
		    buflen, p, flags);

		if (error) {
			/*
			 * Free the bounce pages, unless our resources
			 * are reserved for our exclusive use.
			 */
			if ((map->_dm_flags & BUS_DMA_ALLOCNOW) == 0)
				_isa_dma_free_bouncebuf(t, map);
		}

		/* ...so _isa_bus_dmamap_sync() knows we're bouncing */
		cookie->id_flags |= ID_IS_BOUNCING;
	} else {
		/*
		 * Just use the generic load function.
		 */
		error = _bus_dmamap_load(t, map, buf, buflen, p, flags);
	}

	return (error);
}

/*
 * Like _isa_bus_dmamap_load(), but for mbufs.
 */
int
_isa_bus_dmamap_load_mbuf(bus_dma_tag_t t, bus_dmamap_t map, struct mbuf *m,
    int flags)
{

	panic("_isa_bus_dmamap_load_mbuf: not implemented");
}

/*
 * Like _isa_bus_dmamap_load(), but for uios.
 */
int
_isa_bus_dmamap_load_uio(bus_dma_tag_t t, bus_dmamap_t map, struct uio *uio,
    int flags)
{

	panic("_isa_bus_dmamap_load_uio: not implemented");
}

/*
 * Like _isa_bus_dmamap_load(), but for raw memory allocated with
 * bus_dmamem_alloc().
 */
int
_isa_bus_dmamap_load_raw(bus_dma_tag_t t, bus_dmamap_t map,
    bus_dma_segment_t *segs, int nsegs, bus_size_t size, int flags)
{

	panic("_isa_bus_dmamap_load_raw: not implemented");
}

/*
 * Unload an ISA DMA map.
 */
void
_isa_bus_dmamap_unload(bus_dma_tag_t t, bus_dmamap_t map)
{
	struct isa_dma_cookie *cookie = map->_dm_cookie;

	/*
	 * If we have bounce pages, free them, unless they're
	 * reserved for our exclusive use.
	 */
	if ((cookie->id_flags & ID_HAS_BOUNCE) &&
	    (map->_dm_flags & BUS_DMA_ALLOCNOW) == 0)
		_isa_dma_free_bouncebuf(t, map);

	cookie->id_flags &= ~ID_IS_BOUNCING;

	/*
	 * Do the generic bits of the unload.
	 */
	_bus_dmamap_unload(t, map);
}

/*
 * Synchronize an ISA DMA map.
 */
void
_isa_bus_dmamap_sync(bus_dma_tag_t t, bus_dmamap_t map, bus_addr_t offset,
    bus_size_t len, int op)
{
	struct isa_dma_cookie *cookie = map->_dm_cookie;

#ifdef DEBUG
	if ((op & (BUS_DMASYNC_PREWRITE|BUS_DMASYNC_POSTREAD)) != 0) {
		if (offset >= map->dm_mapsize)
			panic("_isa_bus_dmamap_sync: bad offset");
		if (len == 0 || (offset + len) > map->dm_mapsize)
			panic("_isa_bus_dmamap_sync: bad length");
	}
#endif
#ifdef DIAGNOSTIC
	if ((op & (BUS_DMASYNC_PREREAD | BUS_DMASYNC_PREWRITE)) != 0 &&
	    (op & (BUS_DMASYNC_POSTREAD | BUS_DMASYNC_POSTWRITE)) != 0)
		panic("_isa_bus_dmamap_sync: mix PRE and POST");
#endif /* DIAGNOSTIC */

	/* PREREAD and POSTWRITE are no-ops */
	if (op & BUS_DMASYNC_PREWRITE) {
		/*
		 * If we're bouncing this transfer, copy the
		 * caller's buffer to the bounce buffer.
		 */
		if (cookie->id_flags & ID_IS_BOUNCING)
			memcpy(cookie->id_bouncebuf + offset,
			    cookie->id_origbuf + offset, len);
	}

	_bus_dmamap_sync(t, map, offset, len, op);

	if (op & BUS_DMASYNC_POSTREAD) {
		/*
		 * If we're bouncing this transfer, copy the
		 * bounce buffer to the caller's buffer.
		 */
		if (cookie->id_flags & ID_IS_BOUNCING)
			memcpy(cookie->id_origbuf + offset,
			    cookie->id_bouncebuf + offset, len);
	}
}

/*
 * Allocate memory safe for ISA DMA.
 */
int
_isa_bus_dmamem_alloc(bus_dma_tag_t t, bus_size_t size, bus_size_t alignment,
    bus_size_t boundary, bus_dma_segment_t *segs, int nsegs, int *rsegs,
    int flags)
{
	int error;

	/* Try in ISA addressable region first */
	error = _bus_dmamem_alloc_range(t, size, alignment, boundary,
	    segs, nsegs, rsegs, flags, 0, ISA_DMA_BOUNCE_THRESHOLD);
	if (!error)
		return (error);

	/* Otherwise try anywhere (we'll bounce later) */
	error = _bus_dmamem_alloc_range(t, size, alignment, boundary,
	    segs, nsegs, rsegs, flags, (bus_addr_t)0, (bus_addr_t)-1);
	return (error);
}

/**********************************************************************
 * ISA DMA utility functions
 **********************************************************************/

/*
 * Return 0 if all pages in the passed buffer lie within the DMA'able
 * range RAM.
 */
int
_isa_dma_check_buffer(void *buf, bus_size_t buflen, int segcnt,
    bus_size_t boundary, struct proc *p)
{
	vaddr_t vaddr = (vaddr_t)buf;
	vaddr_t endva;
	paddr_t pa, lastpa;
	u_long pagemask = ~(boundary - 1);
	pmap_t pmap;
	int nsegs;

	endva = round_page(vaddr + buflen);

	nsegs = 1;
	lastpa = 0;

	if (p != NULL)
		pmap = p->p_vmspace->vm_map.pmap;
	else
		pmap = pmap_kernel();

	for (; vaddr < endva; vaddr += NBPG) {
		/*
		 * Get physical address for this segment.
		 */
		pmap_extract(pmap, (vaddr_t)vaddr, &pa);
		pa = trunc_page(pa);

		/*
		 * Is it below the DMA'able threshold?
		 */
		if (pa > ISA_DMA_BOUNCE_THRESHOLD)
			return (EINVAL);

		if (lastpa) {
			/*
			 * Check excessive segment count.
			 */
			if (lastpa + NBPG != pa) {
				if (++nsegs > segcnt)
					return (EFBIG);
			}

			/*
			 * Check boundary restriction.
			 */
			if (boundary) {
				if ((lastpa ^ pa) & pagemask)
					return (EINVAL);
			}
		}
		lastpa = pa;
	}

	return (0);
}

int
_isa_dma_alloc_bouncebuf(bus_dma_tag_t t, bus_dmamap_t map, bus_size_t size,
    int flags)
{
	struct isa_dma_cookie *cookie = map->_dm_cookie;
	int error = 0;

	cookie->id_bouncebuflen = round_page(size);
	error = _bus_dmamem_alloc_range(t, cookie->id_bouncebuflen,
	    NBPG, map->_dm_boundary, cookie->id_bouncesegs,
	    map->_dm_segcnt, &cookie->id_nbouncesegs, flags,
	    0, ISA_DMA_BOUNCE_THRESHOLD);
	if (error)
		goto out;
	error = _bus_dmamem_map(t, cookie->id_bouncesegs,
	    cookie->id_nbouncesegs, cookie->id_bouncebuflen,
	    (caddr_t *)&cookie->id_bouncebuf, flags);

 out:
	if (error) {
		_bus_dmamem_free(t, cookie->id_bouncesegs,
		    cookie->id_nbouncesegs);
		cookie->id_bouncebuflen = 0;
		cookie->id_nbouncesegs = 0;
	} else {
		cookie->id_flags |= ID_HAS_BOUNCE;
		STAT_INCR(isa_dma_stats_nbouncebufs);
	}

	return (error);
}

void
_isa_dma_free_bouncebuf(bus_dma_tag_t t, bus_dmamap_t map)
{
	struct isa_dma_cookie *cookie = map->_dm_cookie;

	STAT_DECR(isa_dma_stats_nbouncebufs);

	_bus_dmamem_unmap(t, cookie->id_bouncebuf,
	    cookie->id_bouncebuflen);
	_bus_dmamem_free(t, cookie->id_bouncesegs,
	    cookie->id_nbouncesegs);
	cookie->id_bouncebuflen = 0;
	cookie->id_nbouncesegs = 0;
	cookie->id_flags &= ~ID_HAS_BOUNCE;
}
#endif /* NISADMA > 0 */
