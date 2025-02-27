#	$OpenBSD: install.md,v 1.57 2024/04/09 11:13:51 kettenis Exp $
#
# Copyright (c) 1996 The NetBSD Foundation, Inc.
# All rights reserved.
#
# This code is derived from software contributed to The NetBSD Foundation
# by Jason R. Thorpe.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
#
# machine dependent section of installation/upgrade script.
#

MOUNT_ARGS_msdos="-o-l"

md_installboot() {
	local _disk=$1 _mdec _plat

	case $(sysctl -n hw.product) in
	*AM335x*)			_plat=am335x;;
	*OMAP4*)			_plat=panda;;
	*Cubox-i*|*HummingBoard*)	_plat=cubox;;
	*Wandboard*)			_plat=wandboard;;
	*Nitrogen6*|*'SABRE Lite'*)	_plat=nitrogen;;
	*)				;; # XXX: Handle unknown platform?
	esac

	if ! installboot -r /mnt ${1}; then
		echo "\nFailed to install bootblocks."
		echo "You will not be able to boot SecBSD from ${1}."
		exit
	fi

	# Apply some final tweaks on selected platforms
	mount ${MOUNT_ARGS_msdos} /dev/${_disk}i /mnt/mnt

	_mdec=/usr/mdec/$_plat

	case $_plat in
	am335x|panda)
		cp $_mdec/{MLO,u-boot.img,*.dtb} /mnt/mnt/
		;;
	cubox|wandboard)
		cp $_mdec/*.dtb /mnt/mnt/
		dd if=$_mdec/SPL of=/dev/${_disk}c bs=1024 seek=1 \
		    status=none
		dd if=$_mdec/u-boot.img of=/dev/${_disk}c bs=1024 seek=69 \
		    status=none
		;;
	nitrogen)
		cp $_mdec/*.dtb /mnt/mnt/
		cat > /tmp/i/boot.cmd<<-__EOT
			setenv fdtfile imx6q-sabrelite.dtb ;
			load ${dtype} ${disk}:1 ${fdtaddr} ${fdtfile} ;
			load ${dtype} ${disk}:1 ${loadaddr} efi/boot/bootarm.efi ;
			bootefi ${loadaddr} ${fdtaddr} ;
		__EOT
		mkuboot -t script -a arm -o linux /tmp/i/boot.cmd \
		    /mnt/mnt/6x_bootscript
		;;
	esac

	umount /mnt/mnt
}

md_prep_fdisk() {
	local _disk=$1 _d

	local bootparttype="C"
	local bootsectorstart="32768"
	local bootsectorsize="32768"
	local bootfstype="msdos"

	while :; do
		_d=whole
		if disk_has $_disk mbr; then
			fdisk $_disk
		else
			echo "MBR has invalid signature; not showing it."
		fi
		ask "Use (W)hole disk or (E)dit the MBR?" "$_d"
		case $resp in
		[wW]*)
			echo -n "Creating a ${bootfstype} partition and an SecBSD partition for rest of $_disk..."
			fdisk -iy -b "${bootsectorsize}@${bootsectorstart}:${bootparttype}" ${_disk} >/dev/null
			echo "done."
			installboot -p $_disk
			return ;;
		[eE]*)
			# Manually configure the MBR.
			cat <<__EOT

You will now create one MBR partition to contain your SecBSD data
and one MBR partition on which the SecBSD boot program is located.
Neither partition will overlap any other partition.

The SecBSD MBR partition will have an id of 'A6' and the boot MBR
partition will have an id of '${bootparttype}' (${bootfstype}).
The boot partition will be at least 16MB and be the first 'MSDOS'
partition on the disk.

$(fdisk ${_disk})
__EOT
			fdisk -e ${_disk}
			disk_has $_disk mbr openbsd && return
			echo No SecBSD partition in MBR, try again. ;;
		esac
	done
}

md_prep_disklabel() {
	local _disk=$1 _f=/tmp/i/fstab.$1

	md_prep_fdisk $_disk

	disklabel_autolayout $_disk $_f || return
	[[ -s $_f ]] && return

	# Edit disklabel manually.
	# Abandon all hope, ye who enter here.
	disklabel -F $_f -E $_disk
}

md_congrats() {
}

md_consoleinfo() {
}
