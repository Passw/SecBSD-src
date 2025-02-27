/* Base configuration file for all SecBSD targets.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* Common SecBSD configuration.
   All SecBSD architectures include this file, which is intended as
   a repository for common defines.

   Some defines are common to all architectures, a few of them are
   triggered by SBSD_* guards, so that we won't override architecture
   defaults by mistakes.

   SBSD_HAS_CORRECT_SPECS:
      another mechanism provides correct specs already.
   SBSD_NO_DYNAMIC_LIBRARIES:
      no implementation of dynamic libraries.
   SBSD_OLD_GAS:
      older flavor of gas which needs help for PIC.
   SBSD_HAS_DECLARE_FUNCTION_NAME, SBSD_HAS_DECLARE_FUNCTION_SIZE,
   SBSD_HAS_DECLARE_OBJECT:
      PIC support, FUNCTION_NAME/FUNCTION_SIZE are independent, whereas
      the corresponding logic for OBJECTS is necessarily coupled.
  SBSD_HAS_CORRECT_ASM_OPS:
      another mechanism provides correct ASM_OP values already.

   There are also a few `default' defines such as ASM_WEAKEN_LABEL,
   intended as common ground for arch that don't provide
   anything suitable.  */

/* SECBSD_NATIVE is defined only when gcc is configured as part of
   the SecBSD source tree, specifically through Makefile.bsd-wrapper.

   In such a case the include path can be trimmed as there is no
   distinction between system includes and gcc includes.  */

/* This configuration method, namely Makefile.bsd-wrapper and
   SECBSD_NATIVE is NOT recommended for building cross-compilers.  */

/* SECBSD_CROSS is only recommended for building cross-compilers which
   target a SecBSD system (kernel/userland) */

#if defined(SECBSD_NATIVE) || defined(SECBSD_CROSS)

/* The compiler is configured with ONLY the gcc/g++ standard headers.  */
#undef INCLUDE_DEFAULTS
#ifdef CROSS_COMPILE
#define INCLUDE_DEFAULTS                       \
  {                                            \
    { GPLUSPLUS_INCLUDE_DIR, "G++", 1, 1, 0 },    \
    { GPLUSPLUS_TOOL_INCLUDE_DIR, "G++", 1, 1, 0 },\
    { GPLUSPLUS_BACKWARD_INCLUDE_DIR, "G++", 1, 1, 0 }, \
    { GPLUSPLUS_INCLUDE_DIR "/..", STANDARD_INCLUDE_COMPONENT, 0, 0, 0 }, \
    { 0, 0, 0, 0, 0 }                             \
  }
#else
#define INCLUDE_DEFAULTS			\
  {						\
    { GPLUSPLUS_INCLUDE_DIR, "G++", 1, 1, 0 },	\
    { GPLUSPLUS_TOOL_INCLUDE_DIR, "G++", 1, 1, 0 },\
    { GPLUSPLUS_BACKWARD_INCLUDE_DIR, "G++", 1, 1, 0 }, \
    { STANDARD_INCLUDE_DIR, STANDARD_INCLUDE_COMPONENT, 0, 0, 0 },	\
    { 0, 0, 0, 0, 0 }				\
  }
#endif

/* Under SecBSD, the normal location of the various *crt*.o files is the
   /usr/lib directory.  */
#undef STANDARD_STARTFILE_PREFIX
#define STANDARD_STARTFILE_PREFIX	"/usr/lib/"

#endif


/* Controlling the compilation driver.  */

/* TARGET_OS_CPP_BUILTINS() common to all SecBSD targets.  */
#define SECBSD_OS_CPP_BUILTINS_COMMON()	\
  do						\
    {						\
      builtin_define ("__SecBSD__");		\
      builtin_define ("__unix__");		\
      builtin_define ("__ANSI_COMPAT");		\
      builtin_assert ("system=unix");		\
      builtin_assert ("system=bsd");		\
      builtin_assert ("system=SecBSD");	\
    }						\
  while (0)

/* TARGET_OS_CPP_BUILTINS() common to all SecBSD ELF targets.  */
#define SECBSD_OS_CPP_BUILTINS_ELF()		\
  do						\
    {						\
      SECBSD_OS_CPP_BUILTINS_COMMON();		\
      builtin_define ("__ELF__");		\
    }						\
  while (0)

/* TARGET_OS_CPP_BUILTINS() common to all LP64 SecBSD targets.  */
#define SECBSD_OS_CPP_BUILTINS_LP64()		\
  do						\
    {						\
      builtin_define ("_LP64");			\
      builtin_define ("__LP64__");		\
    }						\
  while (0)

/* CPP_SPEC appropriate for SecBSD. We deal with -posix and -pthread.
   XXX the way threads are handled currently is not very satisfying,
   since all code must be compiled with -pthread to work.
   This two-stage defines makes it easy to pick that for targets that
   have subspecs.  */
#ifdef CPP_CPU_SPEC
#define SBSD_CPP_SPEC "%(cpp_cpu) %{posix:-D_POSIX_SOURCE} %{pthread:-D_REENTRANT}"
#else
#define SBSD_CPP_SPEC "%{posix:-D_POSIX_SOURCE} %{pthread:-D_REENTRANT}"
#endif

#ifndef SBSD_HAS_CORRECT_SPECS

#ifndef SBSD_NO_DYNAMIC_LIBRARIES
#undef SWITCH_TAKES_ARG
#define SWITCH_TAKES_ARG(CHAR) \
  (DEFAULT_SWITCH_TAKES_ARG (CHAR) \
   || (CHAR) == 'R')
#endif

#undef CPP_SPEC
#define CPP_SPEC SBSD_CPP_SPEC

#ifdef SBSD_OLD_GAS
/* ASM_SPEC appropriate for SecBSD.  For some architectures, SecBSD
   still uses a special flavor of gas that needs to be told when generating
   pic code.  */
#undef ASM_SPEC
#define ASM_SPEC "%{fpic:-k} %{fPIC:-k -K} %|"

#else
/* Since we use gas, stdin -> - is a good idea, but we don't want to
   override native specs just for that.  */
#ifndef ASM_SPEC
#define ASM_SPEC "%|"
#endif
#endif

/* LINK_SPEC appropriate for SecBSD.  Support for GCC options
   -static, -assert, and -nostdlib.  */
#undef LINK_SPEC
#ifdef SBSD_NO_DYNAMIC_LIBRARIES
#define LINK_SPEC \
  "%{g:%{!nostdlib:-L/usr/lib/debug}} %{!nostdlib:%{!r*:%{!e*:-e start}}} -dc -dp %{assert*}"
#else
#define LINK_SPEC \
  "%{g:%{!nostdlib:-L/usr/lib/debug}} %{!shared:%{!nostdlib:%{!r*:%{!e*:-e start}}}} %{shared:-Bshareable -x} -dc -dp %{R*} %{static:-Bstatic} %{assert*}"
#endif

#if defined(HAVE_LD_EH_FRAME_HDR)
#define LINK_EH_SPEC "%{!static:--eh-frame-hdr} "
#endif

#undef LIB_SPEC
#define LIB_SPEC SBSD_LIB_SPEC
#endif


/* Runtime target specification.  */

/* You must redefine CPP_PREDEFINES in any arch specific file.  */
#undef CPP_PREDEFINES

/* Implicit calls to library routines.  */

/* Use memcpy and memset instead of bcopy and bzero.  */
#ifndef TARGET_MEM_FUNCTIONS
#define TARGET_MEM_FUNCTIONS
#endif

/* Miscellaneous parameters.  */

/* Controlling debugging info: dbx options.  */

/* Don't use the `xsTAG;' construct in DBX output; SecBSD systems that
   use DBX don't support it.  */
#define DBX_NO_XREFS


/* Support of shared libraries, mostly imported from svr4.h through netbsd.  */
/* Two differences from svr4.h:
   - we use . - _func instead of a local label,
   - we put extra spaces in expressions such as
     .type _func , @function
     This is more readable for a human being and confuses c++filt less.  */

/* Assembler format: output and generation of labels.  */

/* Define the strings used for the .type and .size directives.
   These strings generally do not vary from one system running SecBSD
   to another, but if a given system needs to use different pseudo-op
   names for these, they may be overridden in the arch specific file.  */

/* SecBSD assembler is hacked to have .type & .size support even in a.out
   format object files.  Functions size are supported but not activated
   yet (look for GRACE_PERIOD_EXPIRED in gas/config/obj-aout.c).
   SET_ASM_OP is needed for attribute alias to work.  */

#ifndef	SBSD_HAS_CORRECT_ASM_OPS

#undef TYPE_ASM_OP
#undef SIZE_ASM_OP
#undef SET_ASM_OP
#undef GLOBAL_ASM_OP

#define TYPE_ASM_OP	"\t.type\t"
#define SIZE_ASM_OP	"\t.size\t"
#define SET_ASM_OP	"\t.set\t"
#define GLOBAL_ASM_OP	"\t.globl\t"

#endif

/* The following macro defines the format used to output the second
   operand of the .type assembler directive.  */
#undef TYPE_OPERAND_FMT
#define TYPE_OPERAND_FMT	"@%s"

/* Provision if extra assembler code is needed to declare a function's result
   (taken from svr4, not needed yet actually).  */
#ifndef ASM_DECLARE_RESULT
#define ASM_DECLARE_RESULT(FILE, RESULT)
#endif

/* These macros generate the special .type and .size directives which
   are used to set the corresponding fields of the linker symbol table
   entries under SecBSD.  These macros also have to output the starting
   labels for the relevant functions/objects.  */

#ifndef SBSD_HAS_DECLARE_FUNCTION_NAME
/* Extra assembler code needed to declare a function properly.
   Some assemblers may also need to also have something extra said
   about the function's return value.  We allow for that here.  */
#undef ASM_DECLARE_FUNCTION_NAME
#define ASM_DECLARE_FUNCTION_NAME(FILE, NAME, DECL)			\
  do {									\
    ASM_OUTPUT_TYPE_DIRECTIVE (FILE, NAME, "function");			\
    ASM_DECLARE_RESULT (FILE, DECL_RESULT (DECL));			\
    ASM_OUTPUT_LABEL(FILE, NAME);					\
  } while (0)
#endif

#ifndef SBSD_HAS_DECLARE_FUNCTION_SIZE
/* Declare the size of a function.  */
#undef ASM_DECLARE_FUNCTION_SIZE
#define ASM_DECLARE_FUNCTION_SIZE(FILE, FNAME, DECL)		\
  do {								\
    if (!flag_inhibit_size_directive)				\
      ASM_OUTPUT_MEASURED_SIZE (FILE, FNAME);			\
  } while (0)
#endif

#ifndef SBSD_HAS_DECLARE_OBJECT
/* Extra assembler code needed to declare an object properly.  */
#undef ASM_DECLARE_OBJECT_NAME
#define ASM_DECLARE_OBJECT_NAME(FILE, NAME, DECL)		\
  do {								\
      HOST_WIDE_INT size;					\
      ASM_OUTPUT_TYPE_DIRECTIVE (FILE, NAME, "object");		\
      size_directive_output = 0;				\
      if (!flag_inhibit_size_directive				\
	  && (DECL) && DECL_SIZE (DECL))			\
	{							\
	  size_directive_output = 1;				\
	  size = int_size_in_bytes (TREE_TYPE (DECL));		\
	  ASM_OUTPUT_SIZE_DIRECTIVE (FILE, NAME, size);		\
	}							\
      ASM_OUTPUT_LABEL (FILE, NAME);				\
  } while (0)

/* Output the size directive for a decl in rest_of_decl_compilation
   in the case where we did not do so before the initializer.
   Once we find the error_mark_node, we know that the value of
   size_directive_output was set by ASM_DECLARE_OBJECT_NAME
   when it was run for the same decl.  */
#undef ASM_FINISH_DECLARE_OBJECT
#define ASM_FINISH_DECLARE_OBJECT(FILE, DECL, TOP_LEVEL, AT_END)	 \
do {									 \
     const char *name = XSTR (XEXP (DECL_RTL (DECL), 0), 0);		 \
     HOST_WIDE_INT size;						 \
     if (!flag_inhibit_size_directive && DECL_SIZE (DECL)		 \
         && ! AT_END && TOP_LEVEL					 \
	 && DECL_INITIAL (DECL) == error_mark_node			 \
	 && !size_directive_output)					 \
       {								 \
	 size_directive_output = 1;					 \
	 size = int_size_in_bytes (TREE_TYPE (DECL));			 \
	 ASM_OUTPUT_SIZE_DIRECTIVE (FILE, name, size);			 \
       }								 \
   } while (0)
#endif


/* Those are `generic' ways to weaken/globalize a label. We shouldn't need
   to override a processor specific definition. Hence, #ifndef ASM_*
   In case overriding turns out to be needed, one can always #undef ASM_*
   before including this file.  */

/* Tell the assembler that a symbol is weak.  */
/* Note: netbsd arm32 assembler needs a .globl here. An override may
   be needed when/if we go for arm32 support.  */
#ifndef ASM_WEAKEN_LABEL
#define ASM_WEAKEN_LABEL(FILE,NAME) \
  do { fputs ("\t.weak\t", FILE); assemble_name (FILE, NAME); \
       fputc ('\n', FILE); } while (0)
#endif

/* Storage layout.  */


/* bug work around: we don't want to support #pragma weak, but the current
   code layout needs HANDLE_PRAGMA_WEAK asserted for __attribute((weak)) to
   work.  On the other hand, we don't define HANDLE_PRAGMA_WEAK directly,
   as this depends on a few other details as well...  */
#define HANDLE_SYSV_PRAGMA 1

/* Disable the use of unsafe builtin functions, (strcat, strcpy), making
 * them easier to spot in the object files.
 */
#define NO_UNSAFE_BUILTINS

/* pick up defines for mprotect (used in TRANSFER_FROM_TRAMPOLINE) */
#include <sys/types.h>
#include <sys/mman.h>
