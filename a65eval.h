#ifndef A65EVAL_H
#define A65EVAL_H

/*
	HEADER:		CUG219;
	TITLE:		6502 Cross-Assembler (Portable);
	FILENAME:	a65eval.h;
	VERSION:	0.1;
	DATE:		08/27/1988;

	DESCRIPTION:	"This program lets you use your computer to assemble
			code for the MOS Technology 6502 microprocessors.  The
			program is written in portable C rather than BDS C.
			All assembler features are supported except relocation
			linkage, and macros.";

	KEYWORDS:	Software Development, Assemblers, Cross-Assemblers,
			MOS Technology, 6502;

	SYSTEM:		CP/M-80, CP/M-86, HP-UX, MSDOS, PCDOS, QNIX;
	COMPILERS:	Aztec C86, Aztec CII, CI-C86, Eco-C, Eco-C88, HP-UX,
			Lattice C, Microsoft C,	QNIX C;

	WARNINGS:	"This program is written in as portable C as possible.
			A port to BDS C would be extremely difficult, but see
			volume CUG113.  A port to Toolworks C is untried."

	AUTHORS:	William C. Colley III;
*/

/*
			  6502 Cross-Assembler in Portable C

		   Copyright (c) 1986 William C. Colley, III

Revision History:

Ver	Date		Description

0.0	NOV 1986	Derived from my 6800/6801 cross-assembler.  WCC3.

0.1	AUG 1988	Fixed a bug in the command line parser that puts it
			into a VERY long loop if the user types a command line
			like "A65 FILE.ASM -L".  WCC3 per Alex Cameron.

This header file contains the function definitions for the assembler's
expression evaluator and lexical analyzer.
*/

#include "a65.h"

/*  Machine opcode argument field parsing routine.  The token stream	*/
/*  from the lexical analyzer is processed to extract addressing mode	*/
/*  information and (possibly) an actual address that can be reduced to */
/*  an unsigned value.  If an error occurs during the evaluation, the	*/
/*  global flag forwd is set to indicate to the line assembler that it	*/
/*  should not base certain decisions on the result of the evaluation.	*/
/*  The address is passed back as the return value of the function.		*/
/*  The addressing mode information is passed back through the global	*/
/*  mailbox argattr.													*/

unsigned do_args();


/*  Expression analysis routine.  The token stream from the lexical		*/
/*  analyzer is processed as an arithmetic expression and reduced to an	*/
/*  unsigned value.  If an error occurs during the evaluation, the		*/
/*  global flag	forwd is set to indicate to the line assembler that it	*/
/*  should not base certain decisions on the result of the evaluation.	*/

unsigned expr();


/*  Lexical analyzer.  The source input character stream is chopped up	*/
/*  into its component parts and the pieces are evaluated.  Symbols are	*/
/*  looked up, operators are looked up, etc.  Everything gets reduced	*/
/*  to an attribute word, a numeric value, and (possibly) a string		*/
/*  value.																*/

TOKEN* lex();


int isalph(char c);


/*  Push back the current token into the input stream.  One level of	*/
/*  pushback is supported.												*/

void unlex();


/*  Get an alphanumeric string into the string value part of the		*/
/*  current token.  Leading blank space is trashed.						*/

void pops(char *s);


/*  Trash blank space and push back the character following it.			*/

void trash();


/*  Get character from input stream.  This routine does a number of		*/
/*  other things while it's passing back characters.  All control		*/
/*  characters except \t and \n are ignored.  \t is mapped into ' '.	*/
/*  Semicolon is mapped to \n.  In addition, a copy of all input is set	*/
/*  up in a line buffer for the benefit of the listing.					*/

int popc();


/*  Push character back onto input stream.  Only one level of push-back	*/
/*  supported.  \0 cannot be pushed back, but nobody would want to.		*/

void pushc(char c);


/*  Begin new line of source input.  This routine returns non-zero if	*/
/*  EOF	has been reached on the main source file, zero otherwise.		*/

int newline();

#endif
