#!/bin/sh -
#
#	$OpenBSD: mkdep.gcc.sh,v 1.18 2017/06/05 14:33:42 espie Exp $
#	$NetBSD: mkdep.gcc.sh,v 1.9 1994/12/23 07:34:59 jtc Exp $
#
# Copyright (c) 1991, 1993
#	The Regents of the University of California.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
#	@(#)mkdep.gcc.sh	8.1 (Berkeley) 6/6/93
#

#
# Scan for a -o option in the arguments and record the filename given.
# This is needed, since "cc -M -o out" writes to the file "out", not to
# stdout.
#
scanfordasho() {
	while [ $# != 0 ]
	do case "$1" in
		-o)
			file="$2"; shift; shift ;;
		-o*)
			file="${1#-o}"; shift ;;
		*)
			shift ;;
		esac
	done
}

D=.depend			# default dependency file is .depend
append=0
pflag=

while :
	do case "$1" in
		# -a appends to the depend file
		-a)
			append=1
			shift ;;

		# -f allows you to select a makefile name
		-f)
			D=$2
			shift; shift ;;

		# the -p flag produces "program: program.c" style dependencies
		# so .o's don't get produced
		-p)
			pflag=p
			shift ;;
		*)
			break ;;
	esac
done

if [ $# = 0 ] ; then
	echo 'usage: mkdep [-ap] [-f file] [flags] file ...'
	exit 1
fi

scanfordasho "$@"

TMP=`mktemp /tmp/mkdep.XXXXXXXXXX` || exit 1

trap 'rm -f $TMP ; trap 2 ; kill -2 $$' 1 2 3 13 15

if [ "x$file" = x ]; then
	${CC:-cc} -M -w "$@" > $TMP
else
	${CC:-cc} -M -w "$@" && cat -- "$file" > $TMP
fi

if [ $? != 0 ]; then
	echo 'mkdep: compile failed.'
	rm -f $TMP
	exit 1
fi

postproc() {
	in=$1
	if [ x$pflag = x ]; then
		sed -e 's; \./; ;g' $in
	else
		sed -e 's;\.o[ ]*:; :;' -e 's; \./; ;g' $in
	fi
}

if [ $append = 1 ]; then
	postproc $TMP >> $D
	if [ $? != 0 ]; then
		echo 'mkdep: append failed.'
		rm -f $TMP
		exit 1
	fi
else
	postproc $TMP > $D
	if [ $? != 0 ]; then
		echo 'mkdep: rename failed.'
		rm -f $TMP
		exit 1
	fi
fi

rm -f $TMP
exit 0
