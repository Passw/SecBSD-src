/*	$OpenBSD: C.c,v 1.15 2014/12/08 03:58:56 jsg Exp $	*/
/*	$NetBSD: C.c,v 1.3 1995/03/26 20:14:02 glass Exp $	*/

/*
 * Copyright (c) 1987, 1993, 1994
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

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "ctags.h"

static int	func_entry(void);
static void	hash_entry(void);
static void	skip_string(int);
static int	str_entry(int);

/*
 * c_entries --
 *	read .c and .h files and call appropriate routines
 */
void
c_entries(void)
{
	int	c;			/* current character */
	int	level;			/* brace level */
	int	token;			/* if reading a token */
	int	t_def;			/* if reading a typedef */
	int	t_level;		/* typedef's brace level */
	char	*sp;			/* buffer pointer */
	char	tok[MAXTOKEN];		/* token buffer */

	lineftell = ftell(inf);
	sp = tok; token = t_def = NO; t_level = -1; level = 0; lineno = 1;
	while (GETC(!=, EOF)) {
		switch (c) {
		/*
		 * Here's where it DOESN'T handle: {
		 *	foo(a)
		 *	{
		 *	#ifdef notdef
		 *		}
		 *	#endif
		 *		if (a)
		 *			puts("hello, world");
		 *	}
		 */
		case '{':
			++level;
			goto endtok;
		case '}':
			/*
			 * if level goes below zero, try and fix
			 * it, even though we've already messed up
			 */
			if (--level < 0)
				level = 0;
			goto endtok;

		case '\n':
			SETLINE;
			/*
			 * the above 3 cases are similar in that they
			 * are special characters that also end tokens.
			 */
endtok:			if (sp > tok) {
				*sp = EOS;
				token = YES;
				sp = tok;
			}
			else
				token = NO;
			continue;

		/*
		 * We ignore quoted strings and character constants
		 * completely.
		 */
		case '"':
		case '\'':
			(void)skip_string(c);
			break;

		/*
		 * comments can be fun; note the state is unchanged after
		 * return, in case we found:
		 *	"foo() XX comment XX { int bar; }"
		 */
		case '/':
			if (GETC(==, '*')) {
				skip_comment(c);
				continue;
			} else if (c == '/') {
				skip_comment(c);
				continue;
			}
			(void)ungetc(c, inf);
			c = '/';
			goto storec;

		/* hash marks flag #define's. */
		case '#':
			if (sp == tok) {
				hash_entry();
				break;
			}
			goto storec;

		/*
		 * if we have a current token, parenthesis on
		 * level zero indicates a function.
		 */
		case '(':
			do {
				if (GETC(==, EOF))
					return;
			} while (iswhite(c));
			if (c == '*')
				break;
			else
				ungetc(c, inf);
			if (!level && token) {
				int	curline;

				if (sp != tok)
					*sp = EOS;
				/*
				 * grab the line immediately, we may
				 * already be wrong, for example,
				 *	foo\n
				 *	(arg1,
				 */
				get_line();
				curline = lineno;
				if (func_entry()) {
					++level;
					pfnote(tok, curline);
				}
				break;
			}
			goto storec;

		/*
		 * semi-colons indicate the end of a typedef; if we find a
		 * typedef we search for the next semi-colon of the same
		 * level as the typedef.  Ignoring "structs", they are
		 * tricky, since you can find:
		 *
		 *	"typedef int time_t;"
		 *	"typedef unsigned int u_int;"
		 *	"typedef unsigned int u_int [10];"
		 *
		 * If looking at a typedef, we save a copy of the last token
		 * found.  Then, when we find the ';' we take the current
		 * token if it starts with a valid token name, else we take
		 * the one we saved.  There's probably some reasonable
		 * alternative to this...
		 */
		case ';':
			if (t_def && level == t_level) {
				t_def = NO;
				get_line();
				if (sp != tok)
					*sp = EOS;
				pfnote(tok, lineno);
				break;
			}
			goto storec;

		/*
		 * store characters until one that can't be part of a token
		 * comes along; check the current token against certain
		 * reserved words.
		 */
		default:
			/*
			 * to treat following function.
			 * func      (arg) {
			 * ....
			 * }
			 */
			if (c == ' ' || c == '\t') {
				int save = c;
				while (GETC(!=, EOF) && (c == ' ' || c == '\t'))
					;
				if (c == EOF)
					return;
				(void)ungetc(c, inf);
				c = save;
			}
	storec:		if (!intoken(c)) {
				if (sp == tok)
					break;
				*sp = EOS;
				/* no typedefs inside typedefs */
				if (!t_def &&
					   !memcmp(tok, "typedef",8)) {
					t_def = YES;
					t_level = level;
					break;
				}
				/* catch "typedef struct" */
				if ((!t_def || t_level < level)
				    && (!memcmp(tok, "struct", 7)
				    || !memcmp(tok, "union", 6)
				    || !memcmp(tok, "enum", 5))) {
					/*
					 * get line immediately;
					 * may change before '{'
					 */
					get_line();
					if (str_entry(c))
						++level;
					break;
					/* } */
				}
				sp = tok;
			}
			else if (sp != tok || begtoken(c)) {
				/* hell... truncate it */
				if (sp == tok + sizeof tok - 1)
					*sp = EOS;
				else
					*sp++ = c;
				token = YES;
			}
			continue;
		}

		sp = tok;
		token = NO;
	}
}

/*
 * func_entry --
 *	handle a function reference
 */
static int
func_entry(void)
{
	int	c;			/* current character */
	int	level = 0;		/* for matching '()' */
	static char attribute[] = "__attribute__";
	char maybe_attribute[sizeof attribute + 1];
	char *anext;

	/*
	 * Find the end of the assumed function declaration.
	 * Note that ANSI C functions can have type definitions so keep
	 * track of the parentheses nesting level.
	 */
	while (GETC(!=, EOF)) {
		switch (c) {
		case '\'':
		case '"':
			/* skip strings and character constants */
			skip_string(c);
			break;
		case '/':
			/* skip comments */
			if (GETC(==, '*'))
				skip_comment(c);
			else if (c == '/')
				skip_comment(c);
			break;
		case '(':
			level++;
			break;
		case ')':
			if (level == 0)
				goto fnd;
			level--;
			break;
		case '\n':
			SETLINE;
		}
	}
	return (NO);
fnd:
	/*
	 * we assume that the character after a function's right paren
	 * is a token character if it's a function and a non-token
	 * character if it's a declaration.  Comments don't count...
	 */
	for (anext = maybe_attribute;;) {
		while (GETC(!=, EOF) && iswhite(c))
			if (c == '\n')
				SETLINE;
		if (c == EOF)
			return NO;
		/*
		 * Recognize the GNU __attribute__ extension, which would
		 * otherwise make the heuristic test DTWT
		 */
		if (anext == maybe_attribute) {
			if (intoken(c)) {
				*anext++ = c;
				continue;
			}
		} else {
			if (intoken(c)) {
				if (anext - maybe_attribute < (int)(sizeof attribute - 1))
					*anext++ = c;
				else
					break;
				continue;
			} else {
				*anext++ = '\0';
				if (strcmp(maybe_attribute, attribute) == 0) {
					(void)ungetc(c, inf);
					return NO;
				}
				break;
			}
		}
		if (intoken(c) || c == '{')
			break;
		if (c == '/' && GETC(==, '*'))
			skip_comment(c);
		else if (c == '/')
			skip_comment(c);
		else {				/* don't ever "read" '/' */
			(void)ungetc(c, inf);
			return (NO);
		}
	}
	if (c != '{')
		(void)skip_key('{');
	return (YES);
}

/*
 * hash_entry --
 *	handle a line starting with a '#'
 */
static void
hash_entry(void)
{
	int	c;			/* character read */
	int	curline;		/* line started on */
	char	*sp;			/* buffer pointer */
	char	tok[MAXTOKEN];		/* storage buffer */

	/*
	 * to treat following macro.
	 * #     macro(arg)        ....
	 */
	while (GETC(!=, EOF) && (c == ' ' || c == '\t'))
		;
	(void)ungetc(c, inf);

	curline = lineno;
	for (sp = tok;;) {		/* get next token */
		if (GETC(==, EOF))
			return;
		if (iswhite(c))
			break;
		/* hell... truncate it */
		if (sp == tok + sizeof tok - 1)
			*sp = EOS;
		else
			*sp++ = c;
	}
	*sp = EOS;
	if (memcmp(tok, "define", 6))	/* only interested in #define's */
		goto skip;
	for (;;) {			/* this doesn't handle "#define \n" */
		if (GETC(==, EOF))
			return;
		if (!iswhite(c))
			break;
	}
	for (sp = tok;;) {		/* get next token */
		/* hell... truncate it */
		if (sp == tok + sizeof tok - 1)
			*sp = EOS;
		else
			*sp++ = c;
		if (GETC(==, EOF))
			return;
		/*
		 * this is where it DOESN'T handle
		 * "#define \n"
		 */
		if (!intoken(c))
			break;
	}
	*sp = EOS;
	if (dflag || c == '(') {	/* only want macros */
		get_line();
		pfnote(tok, curline);
	}
skip:	if (c == '\n') {		/* get rid of rest of define */
		SETLINE
		if (*(sp - 1) != '\\')
			return;
	}
	(void)skip_key('\n');
}

/*
 * str_entry --
 *	handle a struct, union or enum entry
 */
static int
str_entry(int c)
{
	int	curline;		/* line started on */
	char	*sp;			/* buffer pointer */
	char	tok[LINE_MAX];		/* storage buffer */

	curline = lineno;
	while (iswhite(c))
		if (GETC(==, EOF))
			return (NO);
	if (c == '{')		/* it was "struct {" */
		return (YES);
	for (sp = tok;;) {		/* get next token */
		/* hell... truncate it */
		if (sp == tok + sizeof tok - 1)
			*sp = EOS;
		else
			*sp++ = c;
		if (GETC(==, EOF))
			return (NO);
		if (!intoken(c))
			break;
	}
	switch (c) {
		case '{':		/* it was "struct foo{" */
			--sp;
			break;
		case '\n':		/* it was "struct foo\n" */
			SETLINE;
			/*FALLTHROUGH*/
		default:		/* probably "struct foo " */
			while (GETC(!=, EOF))
				if (!iswhite(c))
					break;
			if (c != '{') {
				(void)ungetc(c, inf);
				return (NO);
			}
	}
	*sp = EOS;
	pfnote(tok, curline);
	return (YES);
}

/*
 * skip_comment --
 *	skip over comment
 */
void
skip_comment(int commenttype)
{
	int	c;			/* character read */
	int	star;			/* '*' flag */

	for (star = 0; GETC(!=, EOF);)
		switch(c) {
		/* comments don't nest, nor can they be escaped. */
		case '*':
			star = YES;
			break;
		case '/':
			if (commenttype == '*' && star)
				return;
			break;
		case '\n':
			if (commenttype == '/') {
				/* We don't really parse C, so sometimes it
				 * is necessary to see the newline
				 */
				ungetc(c, inf);
				return;
			}
			SETLINE;
			/*FALLTHROUGH*/
		default:
			star = NO;
			break;
		}
}

/*
 * skip_string --
 *	skip to the end of a string or character constant.
 */
static void
skip_string(int key)
{
	int	c,
		skip;

	for (skip = NO; GETC(!=, EOF); )
		switch (c) {
		case '\\':		/* a backslash escapes anything */
			skip = !skip;	/* we toggle in case it's "\\" */
			break;
		case '\n':
			SETLINE;
			/*FALLTHROUGH*/
		default:
			if (c == key && !skip)
				return;
			skip = NO;
		}
}

/*
 * skip_key --
 *	skip to next char "key"
 */
int
skip_key(int key)
{
	int	c,
		skip,
		retval;

	for (skip = retval = NO; GETC(!=, EOF);)
		switch(c) {
		case '\\':		/* a backslash escapes anything */
			skip = !skip;	/* we toggle in case it's "\\" */
			break;
		case ';':		/* special case for yacc; if one */
		case '|':		/* of these chars occurs, we may */
			retval = YES;	/* have moved out of the rule */
			break;		/* not used by C */
		case '\'':
		case '"':
			/* skip strings and character constants */
			skip_string(c);
			break;
		case '/':
			/* skip comments */
			if (GETC(==, '*')) {
				skip_comment(c);
				break;
			} else if (c == '/') {
				skip_comment(c);
				break;
			}
			(void)ungetc(c, inf);
			c = '/';
			goto norm;
		case '\n':
			SETLINE;
			/*FALLTHROUGH*/
		default:
		norm:
			if (c == key && !skip)
				return (retval);
			skip = NO;
		}
	return (retval);
}
