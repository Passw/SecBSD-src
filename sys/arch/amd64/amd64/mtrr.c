/* $OpenBSD: mtrr.c,v 1.5 2024/04/03 02:01:21 guenther Exp $ */
/*-
 * Copyright (c) 1999 Michael Smith <msmith@freebsd.org>
 * Copyright (c) 1999 Brian Fundakowski Feldman
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/param.h>
#include <sys/memrange.h>
#include <sys/systm.h>

#include <machine/specialreg.h>

extern struct mem_range_ops mrops;

void
mem_range_attach(void)
{
	struct cpu_info *ci = &cpu_info_primary;
	int family, model, step;

	family = (cpu_id >> 8) & 0xf;
	model  = (cpu_id >> 4) & 0xf;
	step   = (cpu_id >> 0) & 0xf;

	/* Try for i686 MTRRs */
	if ((ci->ci_vendor == CPUV_AMD ||
	     ci->ci_vendor == CPUV_INTEL ||
	     ci->ci_vendor == CPUV_VIA) &&
	    (family == 0x6 || family == 0xf) &&
	    cpu_feature & CPUID_MTRR) {
		mem_range_softc.mr_op = &mrops;
	}

	/* Initialise memory range handling */
	if (mem_range_softc.mr_op != NULL)
		mem_range_softc.mr_op->init(&mem_range_softc);
}

