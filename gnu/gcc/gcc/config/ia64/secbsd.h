/* Definitions for ia64-linux target.  */

/* This macro is a C statement to print on `stderr' a string describing the
   particular machine description choice.  */

#define TARGET_VERSION fprintf (stderr, " (SecBSD/ia64)");

/* Target OS builtins.  */
#define TARGET_OS_CPP_BUILTINS()		\
  do						\
    {						\
	builtin_define ("__unix__");		\
	builtin_define ("__SecBSD__");		\
	builtin_assert ("system=unix");		\
	builtin_assert ("system=bsd");		\
	builtin_assert ("system=SecBSD");	\
    }						\
  while (0)

