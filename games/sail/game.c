/*	$OpenBSD: game.c,v 1.5 2016/01/08 20:26:33 mestre Exp $	*/
/*	$NetBSD: game.c,v 1.3 1995/04/22 10:36:56 cgd Exp $	*/

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 */

#include "extern.h"

int
maxturns(struct ship *ship, char *af)
{
	int turns;

	turns = ship->specs->ta;
	if ((*af = (ship->file->drift > 1 && turns)) != 0) {
		turns--;
		if (ship->file->FS == 1)
			turns = 0;
	}
	return turns;
}

int
maxmove(struct ship *ship, int dir, int fs)
{
	int riggone = 0, Move, flank = 0;

	Move = ship->specs->bs;
	if (!ship->specs->rig1)
		riggone++;
	if (!ship->specs->rig2)
		riggone++;
	if (!ship->specs->rig3)
		riggone++;
	if (!ship->specs->rig4)
		riggone++;
	if ((ship->file->FS || fs) && fs != -1) {
		flank = 1;
		Move = ship->specs->fs;
	}
	if (dir == winddir)
		Move -= 1 + WET[windspeed][ship->specs->class-1].B;
	else if (dir == winddir + 2 || dir == winddir - 2 || dir == winddir - 6 || dir == winddir + 6)
		Move -= 1 + WET[windspeed][ship->specs->class-1].C;
	else if (dir == winddir + 3 || dir == winddir - 3 || dir == winddir - 5 || dir == winddir + 5)
		Move = (flank ? 2 : 1) - WET[windspeed][ship->specs->class-1].D;
	else if (dir == winddir + 4 || dir == winddir - 4)
		Move = 0;
	else
		Move -= WET[windspeed][ship->specs->class-1].A;
	Move -= riggone;
	Move = Move < 0 ? 0 : Move;
	return(Move);
}
