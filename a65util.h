#ifndef A65_UTIL_H
#define A65_UTIL_H

/*
			  6502 Cross-Assembler in Portable C

		   Copyright (c) 1986 William C. Colley, III

This header file contains the function definitions for the following 
utility packages:

	1)  symbol table building and searching

	2)  opcode and operator table searching

	3)  listing file output

	4)  hex file output

	5)  error flagging
*/

#include "a65.h"

/* Error code descriptions */
#define ERR_STATEMENT	"Illegal or missing statement"
#define ERR_PAREN		"Parenthesis imbalance"
#define ERR_QUOTE		"Missing quotation mark"
#define ERR_A			"Illegal addressing mode"
#define ERR_B			"Branch target too distant"
#define ERR_D			"Illegal digit"
#define ERR_E			"Illegal expression"
#define ERR_I			"IF-ENDI imbalance"
#define ERR_L			"Illegal label"
#define ERR_M			"Multiply defined label"
#define ERR_O			"Illegal opcode"
#define ERR_P			"Phasing error"
#define ERR_R			"Illegal register"
#define ERR_S			"Illegal syntax"
#define ERR_T			"Too many arguments"
#define ERR_U			"Undefined label"
#define ERR_V			"Illegal value"
#define ERR_UNKNOWN		"Unknown error"

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


/*  Export file open routine.  If an export file is already open, a		*/
/*  warning occurs.  If the export file doesn't open correctly, a		*/
/*  fatal error occurs.  If no export file is open, all calls to		*/
/*  eputs() and eclose() will produce fatal errors.						*/
void eopen(char *nam);


/*  Export file line output routine.  This routine writes the value		*/
/*  of a single symbol to the currently open export file. If the export	*/
/*  file is not open, or the disk is full, a fatal error occurs.		*/
void eputs(SYMBOL *sym);


/*  Export file close routine. */
void eclose();


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
