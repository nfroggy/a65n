#ifndef A65_UTIL_H
#define A65_UTIL_H

/*
	HEADER:		CUG219;
	TITLE:		6502 Cross-Assembler (Portable);
	FILENAME:	a65util.h;
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

This header file contains the function definitions for the following 
utility packages:

	1)  symbol table building and searching

	2)  opcode and operator table searching

	3)  listing file output

	4)  hex file output

	5)  error flagging
*/

#include "a65.h"

/*  Add new symbol to symbol table.  Returns pointer to symbol even if	*/
/*  the symbol already exists.  If there's not enough memory to store	*/
/*  the new symbol, a fatal error occurs.								*/

SYMBOL *new_symbol(char *nam);


/*  Look up symbol in symbol table.  Returns pointer to symbol or NULL	*/
/*  if symbol not found.												*/

SYMBOL *find_symbol(char *nam);


/*  Opcode table search routine.  This routine pats down the opcode		*/
/*  table for a given opcode and returns either a pointer to it or		*/
/*  NULL if the opcode doesn't exist.									*/

OPCODE *find_code(char *nam);


/*  Operator table search routine.  This routine pats down the			*/
/*  operator table for a given operator and returns either a pointer	*/
/*  to it or NULL if the opcode doesn't exist.							*/

OPCODE *find_operator(char *nam);


/*  Listing file open routine.  If a listing file is already open, a	*/
/*  warning occurs.  If the listing file doesn't open correctly, a		*/
/*  fatal error occurs.  If no listing file is open, all calls to		*/
/*  lputs() and lclose() have no effect.								*/

void lopen(char *nam);


/*  Listing file line output routine.  This routine processes the		*/
/*  source line saved by popc() and the output of the line assembler in	*/
/*  buffer obj into a line of the listing.  If the disk fills up, a		*/
/*  fatal error occurs.													*/

void lputs();


/*  Listing file close routine.  The symbol table is appended to the	*/
/*  listing in alphabetic order by symbol name, and the listing file is	*/
/*  closed.  If the disk fills up, a fatal error occurs.				*/

void lclose();


/*  Binary file open routine.  If the file is already open, a warning	*/
/*  occurs.  If the file doesn't open correctly, a fatal error occurs.	*/
/*  If no binary file is open, all calls to bputc(), bseek(), and		*/
/*  bclose() have no effect.											*/

void bopen(char *nam);


/*  Binary file write routine.  The data byte is appended to the output	*/
/*  buffer.  If the buffer fills up, it gets written to disk. 			*/

void bputc(unsigned c);


/*  Binary file address set routine. Note that this can only be used to */
/*  seek forwards in the file. Seeking backwards will cause an error.	*/

void bseek(unsigned a);


/*  Binary file close routine. All buffered data is written to disk,	*/
/*  and the output file is closed.										*/

void bclose();


/*  Error handler routine.  If the current error code is non-blank,		*/
/*  the error code is filled in and the	number of lines with errors		*/
/*  is adjusted.														*/

void error(char code);


/*  Fatal error handler routine.  A message gets printed on the stderr	*/
/*  device, and the program bombs.										*/

void fatal_error(char *msg);


/*  Non-fatal error handler routine.  A message gets printed on the		*/
/*  stderr device, and the routine returns.								*/

void warning(char *msg);

#endif
