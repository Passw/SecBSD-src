/*	$OpenBSD: sigcode.S,v 1.14 2023/12/12 07:37:20 deraadt Exp $	*/
/*	$NetBSD: sigcode.S,v 1.6 2003/10/05 19:44:58 matt Exp $	*/

/*
 * Copyright (C) 1994-1997 Mark Brinicombe
 * Copyright (C) 1994 Brini
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Brini.
 * 4. The name of Brini may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY BRINI ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL BRINI BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "assym.h"

#include <sys/syscall.h>

#include <machine/asm.h>

/*
 * Signal trampoline;
 */

	.section .rodata
	.align	2
	.globl	sigcode
	.type	x,_ASM_TYPE_FUNCTION
sigcode:
/*
 * The kernel arranges for the handler to be invoked directly.  This
 * trampoline is used only to return from the signal.
 *
 * The stack pointer points to the saved sigcontext.
 */
/*	mov	r0, sp */
	add	r0, sp, #SIGF_SC
	mov	r12, #SYS_sigreturn
	.globl	sigcodecall
sigcodecall:
	swi	0
	dsb	nsh
	isb
	.globl  sigcoderet
sigcoderet:
	.global esigcode
esigcode:
	/* FALLTHROUGH */
	.globl	sigfill
sigfill:
	udf	#0
esigfill:

	.data
	.align	2
	.globl	sigfillsiz
sigfillsiz:
	.word	esigfill - sigfill

	.text
