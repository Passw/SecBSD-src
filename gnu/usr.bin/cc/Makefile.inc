# $FreeBSD: src/gnu/usr.bin/cc/Makefile.inc,v 1.66.8.1 2009/04/15 03:14:26 kensmith Exp $
# $OpenBSD: Makefile.inc,v 1.7 2017/07/24 19:22:49 robert Exp $

.include "../Makefile.inc"

.if ${COMPILER_VERSION:L} == "clang"
CC=	gcc
CXX=	g++
.endif

# Sometimes this is .include'd several times...
.if !defined(__CC_MAKEFILE_INC__)
__CC_MAKEFILE_INC__= ${MFILE}

.if defined(CROSSDIR)
TOOLS_PREFIX=${CROSSDIR}
.endif

GCCLIB= ${.CURDIR}/../../../gcc
GCCDIR=	${GCCLIB}/gcc

.include "Makefile.tgt"

# Machine description.
MD_FILE=	${GCCDIR}/config/${GCC_CPU}/${GCC_CPU}.md
GCC_TARGET=	${TARGET_ARCH}-unknown-secbsd${OSREV}

NOPIE=		Yes

CFLAGS+=	-DIN_GCC -DHAVE_CONFIG_H
CFLAGS+=	-DPREFIX=\"${TOOLS_PREFIX}/usr\"
#CFLAGS+=	-DWANT_COMPILER_INVARIANTS
CFLAGS+=	${PIE_DEFAULT}

# If building 64-bit longs for the i386, "_LARGE_LONG" should also be defined
# to get the proper sizes in limits.h
.if defined(LONG_TYPE_SIZE)
CFLAGS+=	-DLONG_TYPE_SIZE=${LONG_TYPE_SIZE}
.endif

.if ${TARGET_ARCH} != ${MACHINE_ARCH}
CFLAGS+=	-DCROSS_COMPILE
.endif

.if defined(WANT_FORCE_OPTIMIZATION_DOWNGRADE)
CFLAGS+= -DFORCE_OPTIMIZATION_DOWNGRADE=${WANT_FORCE_OPTIMIZATION_DOWNGRADE}
.endif

.if exists(${.OBJDIR}/../cc_tools)
CFLAGS+=	-I${.OBJDIR}/../cc_tools
.endif
CFLAGS+=	-I${.CURDIR}/../cc_tools
# This must go after the -I for cc_tools to resolve ambiguities for hash.h
# correctly.
CFLAGS+=	-I${GCCDIR} -I${GCCDIR}/config

CFLAGS+=	-I${GCCLIB}/include
CFLAGS+=	-I${GCCLIB}/libcpp/include
CFLAGS+=	-I${GCCLIB}/libdecnumber

.if exists(${.OBJDIR}/../cc_int)
LIBBACKEND=	${.OBJDIR}/../cc_int/libbackend.a
.else
LIBBACKEND=	${.CURDIR}/../cc_int/libbackend.a
.endif

LIBIBERTY=	/usr/lib/libiberty.a

.if exists(${.OBJDIR}/../libcpp)
LIBCPP=		${.OBJDIR}/../libcpp/libcpp.a
.else
LIBCPP=		${.CURDIR}/../libcpp/libcpp.a
.endif

.if exists(${.OBJDIR}/../libdecnumber)
LIBDECNUMBER=	${.OBJDIR}/../libdecnumber/libdecnumber.a
.else
LIBDECNUMBER=	${.CURDIR}/../libdecnumber/libdecnumber.a
.endif

.endif # !__CC_MAKEFILE_INC__
