#ifndef A65EVAL_H
#define A65EVAL_H

/*
			  6502 Cross-Assembler in Portable C

		   Copyright (c) 1986 William C. Colley, III

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
