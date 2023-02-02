/*
		      6502 Cross-Assembler in Portable C

		   Copyright (c) 1986 William C. Colley, III

This module contains the following utility packages:

	1)  symbol table building and searching

	2)  opcode and operator table searching

	3)  listing file output

	4)  hex file output

	5)  error flagging
*/

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*  Get global goodies:  */

#include "a65.h"
#include "a65eval.h"
#include "a65util.h"

/*  Get access to global mailboxes defined in A65.C:			*/

extern char errcode, line[], title[];
extern int eject, filesp, listhex, pass;
extern unsigned address, bytes, errors, listleft, obj[], pagelen;
extern FILE_INFO filestk[];

/*  The symbol table is a binary tree of variable-length blocks drawn	*/
/*  from the heap with the calloc() function.  The root pointer lives	*/
/*  here:								*/

static SYMBOL *sroot = NULL;

/* Static function declarations: */
static OPCODE *bsearchtbl(OPCODE *lo, OPCODE *hi, char *nam);
static int ustrcmp(char *s, char *t);
static void list_sym(SYMBOL *sp);
static void check_page();
static void record();

/*  Add new symbol to symbol table.  Returns pointer to symbol even if	*/
/*  the symbol already exists.  If there's not enough memory to store	*/
/*  the new symbol, a fatal error occurs.				*/

SYMBOL *new_symbol(char *nam) {
    SCRATCH int i;
    SCRATCH SYMBOL **p, *q;
    void fatal_error();

    for (p = &sroot; (q = *p) && (i = strcmp(nam,q -> sname)); )
		p = i < 0 ? &(q -> left) : &(q -> right);
    if (!q) {
		if (!(*p = q = (SYMBOL *)calloc(1,sizeof(SYMBOL) + strlen(nam))))
			fatal_error(SYMBOLS);
		strcpy(q -> sname,nam);
    }
    return q;
}

/*  Look up symbol in symbol table.  Returns pointer to symbol or NULL	*/
/*  if symbol not found.						*/

SYMBOL *find_symbol(char *nam) {
    SCRATCH int i;
    SCRATCH SYMBOL *p;

    for (p = sroot; p && (i = strcmp(nam,p -> sname));
		p = i < 0 ? p -> left : p -> right);
    return p;
}

/*  Opcode table search routine.  This routine pats down the opcode	*/
/*  table for a given opcode and returns either a pointer to it or	*/
/*  NULL if the opcode doesn't exist.					*/

OPCODE *find_code(char *nam) {
	static OPCODE opctbl[] = {
		{ TWOOP,			0x61,	"ADC"	},
		{ PSEUDO,			ALIGN,	"ALIGN"	},
		{ TWOOP,			0x21,	"AND"	},
		{ LOGOP,			0x06,	"ASL"	},
		{ INHOP,			0x0a,	"ASLA"	},
		{ PSEUDO,			BASE,	"BASE"	},
		{ RELBR,			0x90,	"BCC"	},
		{ RELBR,			0xb0,	"BCS"	},
		{ RELBR,			0xf0,	"BEQ"	},
		{ BITOP,			0x24,	"BIT"	},
		{ RELBR,			0x30,	"BMI"	},
		{ RELBR,			0xd0,	"BNE"	},
		{ RELBR,			0x10,	"BPL"	},
		{ INHOP,			0x00,	"BRK"	},
		{ RELBR,			0x50,	"BVC"	},
		{ RELBR,			0x70,	"BVS"	},
		{ INHOP,			0x18,	"CLC"	},
		{ INHOP,			0xd8,	"CLD"	},
		{ INHOP,			0x58,	"CLI"	},
		{ INHOP,			0xb8,	"CLV"	},
		{ TWOOP,			0xc1,	"CMP"	},
		{ CPXY,				0xe0,	"CPX"	},
		{ CPXY,				0xc0,	"CPY"	},
		{ PSEUDO,			DB,		"DB"	},
		{ INCOP,			0xc6,	"DEC"	},
		{ INHOP,			0xca,	"DEX"	},
		{ INHOP,			0x88,	"DEY"	},
		{ PSEUDO,			DS,		"DS"	},
		{ PSEUDO,			DW,		"DW"	},
		{ PSEUDO + ISIF,	ELSE,	"ELSE"	},
		{ PSEUDO,			END,	"END"	},
		{ PSEUDO + ISIF,	ENDI,	"ENDI"	},
		{ TWOOP,			0x41,	"EOR"	},
		{ PSEUDO,			EQU,	"EQU"	},
		{ PSEUDO,			EXP,	"EXP"	},
		{ PSEUDO + ISIF,	IF,		"IF"	},
		{ INCOP,			0xe6,	"INC"	},
		{ PSEUDO,			INCB,	"INCB"	},
		{ PSEUDO,			INCL,	"INCL"	},
		{ INHOP,			0xe8,	"INX"	},
		{ INHOP,			0xc8,	"INY"	},
		{ JUMP,				0x4c,	"JMP"	},
		{ CALL,				0x20,	"JSR"	},
		{ TWOOP,			0xa1,	"LDA"	},
		{ LDXY,				0xa2,	"LDX"	},
		{ LDXY,				0xa0,	"LDY"	},
		{ LOGOP,			0x46,	"LSR"	},
		{ INHOP,			0x4a,	"LSRA"	},
		{ PSEUDO,			MSG,	"MSG"	},
		{ INHOP,			0xea,	"NOP"	},
		{ TWOOP,			0x01,	"ORA"	},
		{ PSEUDO,			ORG,	"ORG"	},
		{ PSEUDO,			PAGE,	"PAGE"	},
		{ INHOP,			0x48,	"PHA"	},
		{ INHOP,			0x08,	"PHP"	},
		{ INHOP,			0x68,	"PLA"	},
		{ INHOP,			0x28,	"PLP"	},
		{ PSEUDO,			RMB,	"RMB"	},
		{ LOGOP,			0x26,	"ROL"	},
		{ INHOP,			0x2a,	"ROLA"	},
		{ LOGOP,			0x66,	"ROR"	},
		{ INHOP,			0x6a,	"RORA"	},
		{ INHOP,			0x40,	"RTI"	},
		{ INHOP,			0x60,	"RTS"	},
		{ TWOOP,			0xe1,	"SBC"	},
		{ INHOP,			0x38,	"SEC"	},
		{ INHOP,			0xf8,	"SED"	},
		{ INHOP,			0x78,	"SEI"	},
		{ PSEUDO,			SET,	"SET"	},
		{ TWOOP,			0x81,	"STA"	},
		{ STXY,				0x86,	"STX"	},
		{ STXY,				0x84,	"STY"	},
		{ INHOP,			0xaa,	"TAX"	},
		{ INHOP,			0xa8,	"TAY"	},
		{ PSEUDO,			TITL,	"TITL"	},
		{ INHOP,			0xba,	"TSX"	},
		{ INHOP,			0x8a,	"TXA"	},
		{ INHOP,			0x9a,	"TXS"	},
		{ INHOP,			0x98,	"TYA"	}
    };

    return bsearchtbl(opctbl,opctbl + (sizeof(opctbl) / sizeof(OPCODE)),nam);
}

/*  Operator table search routine.  This routine pats down the		*/
/*  operator table for a given operator and returns either a pointer	*/
/*  to it or NULL if the opcode doesn't exist.				*/

OPCODE *find_operator(char *nam) {
    static OPCODE oprtbl[] = {
		{ REG,						'A',		"A"		},
		{ BINARY + LOG1  + OPR,		AND,		"AND"	},
		{ BINARY + RELAT + OPR,		'=',		"EQ"	},
		{ BINARY + RELAT + OPR,		GE,			"GE"	},
		{ BINARY + RELAT + OPR,		'>',		"GT"	},
		{ UNARY  + UOP3  + OPR,		HIGH,		"HIGH"	},
		{ BINARY + RELAT + OPR,		LE,			"LE"	},
		{ UNARY  + UOP3  + OPR,		LOW,		"LOW"	},
		{ BINARY + RELAT + OPR,		'<',		"LT"	},
		{ BINARY + MULT  + OPR,		MOD,		"MOD"	},
		{ BINARY + RELAT + OPR,		NE,			"NE"	},
		{ UNARY  + UOP2  + OPR,		NOT,		"NOT"	},
		{ BINARY + LOG2  + OPR,		OR,			"OR"	},
		{ BINARY + MULT  + OPR,		SHL,		"SHL"	},
		{ BINARY + MULT  + OPR,		SHR,		"SHR"	},
		{ REG,						'X',		"X"		},
		{ BINARY + LOG2  + OPR,		XOR,		"XOR"	},
		{ REG,						'Y',		"Y"		},
    };

    return bsearchtbl(oprtbl,oprtbl + (sizeof(oprtbl) / sizeof(OPCODE)),nam);
}

static OPCODE *bsearchtbl(OPCODE *lo, OPCODE *hi, char *nam) {
    SCRATCH int i;
    SCRATCH OPCODE *chk;

    for (;;) {
		chk = lo + (hi - lo) / 2;
		if (!(i = ustrcmp(chk -> oname,nam))) return chk;
		if (chk == lo) return NULL;
		if (i < 0) lo = chk;
		else hi = chk;
    }
}

static int ustrcmp(char *s, char *t) {
    SCRATCH int i;

    while (!(i = toupper(*s++) - toupper(*t)) && *t++);
    return i;
}

/* export file pointer */
static FILE *export = NULL;

/*  Export file open routine.  If an export file is already open, a		*/
/*  warning occurs.  If the export file doesn't open correctly, a		*/
/*  fatal error occurs.  If no export file is open, all calls to		*/
/*  eputs() and eclose() will produce fatal errors.						*/

void eopen(char *nam) {
	if (export) warning(TWOEXP);
	else if (!(export = fopen(nam, "w"))) fatal_error(EXPOPEN);
	else {
		fprintf(export, "; Autogenerated export file - do not modify!\n\n");
		if (ferror(export)) fatal_error(DSKFULL);
	}
}

/*  Export file line output routine.  This routine writes the value		*/
/*  of a single symbol to the currently open export file. If the export	*/
/*  file is not open, or the disk is full, a fatal error occurs.		*/

void eputs(SYMBOL *sym) {
	if (export) {
		fprintf(export, "%s\tequ\t$%X\n", sym->sname, sym->valu);
		if (ferror(export)) fatal_error(DSKFULL);
	}
	else fatal_error(NOEXP);
}

/*  Export file close routine. */
void eclose() {
	if (export) {
		fclose(export);
	}
}


/*  Buffer storage for line listing routine.  This allows the listing	*/
/*  output routines to do all operations without the main routine		*/
/*  having to fool with it.												*/

static FILE *list = NULL;

/*  Listing file open routine.  If a listing file is already open, a	*/
/*  warning occurs.  If the listing file doesn't open correctly, a		*/
/*  fatal error occurs.  If no listing file is open, all calls to		*/
/*  lputs() and lclose() have no effect.								*/

void lopen(char *nam) {
    if (list) warning(TWOLST);
    else if (!(list = fopen(nam,"w"))) fatal_error(LSTOPEN);
    return;
}

/*  Listing file line output routine.  This routine processes the		*/
/*  source line saved by popc() and the output of the line assembler in	*/
/*  buffer obj into a line of the listing.  If the disk fills up, a		*/
/*  fatal error occurs.													*/

void lputs() {
    SCRATCH int i, j;
    SCRATCH unsigned *o;

    if (list) {
		i = bytes;  o = obj;
		do {
			fprintf(list,"%c  ",errcode);
			if (listhex) {
				fprintf(list,"%04x  ",address);
				for (j = 4; j; --j) {
					if (i) { --i;  ++address;  fprintf(list," %02x",*o++); }
					else fprintf(list,"   ");
				}
			}
			else fprintf(list,"%18s","");
			fprintf(list,"   %s",line);  strcpy(line,"\n");
			check_page();
			if (ferror(list)) fatal_error(DSKFULL);
		} while (listhex && i);
    }
    return;
}

/*  Listing file close routine.  The symbol table is appended to the	*/
/*  listing in alphabetic order by symbol name, and the listing file is	*/
/*  closed.  If the disk fills up, a fatal error occurs.				*/

static int col = 0;

void lclose() {
    if (list) {
		if (sroot) {
			list_sym(sroot);
			if (col) fprintf(list,"\n");
		}
		fprintf(list,"\f");
		if (ferror(list) || fclose(list) == EOF) fatal_error(DSKFULL);
    }
    return;
}

static void list_sym(SYMBOL *sp) {
    if (sp) {
		list_sym(sp -> left);
		fprintf(list,"%04x  %-10s",sp -> valu,sp -> sname);
		if ((++col) % SYMCOLS) fprintf(list,"    ");
		else {
			fprintf(list,"\n");
			if (sp -> right) check_page();
		}
		list_sym(sp -> right);
    }
    return;
}

static void check_page() {
    if (pagelen && !--listleft) eject = TRUE;
    if (eject) {
		eject = FALSE;  listleft = pagelen;  fprintf(list,"\f");
		if (title[0]) { listleft -= 2;  fprintf(list,"%s\n\n",title); }
    }
    return;
}

/*  Buffer storage for binary output file.  This allows the file	*/
/*  output routines to do all of the required buffering and record	*/
/*  forming without the	main routine having to fool with it.		*/

static FILE *outfile = NULL;
static unsigned cnt = 0;
static unsigned addr = 0;
static uint8_t buf[HEXSIZE];

/*  Binary file open routine.  If the file is already open, a warning	*/
/*  occurs.  If the file doesn't open correctly, a fatal error occurs.	*/
/*  If no binary file is open, all calls to bputc(), bseek(), and		*/
/*  bclose() have no effect.											*/

void bopen(char *filename) {
	if (outfile) {
		warning(TWOHEX);
	}
	else {
		outfile = fopen(filename, "wb");
		if (!outfile) {
			fatal_error(HEXOPEN);
		}
	}
}

/*  Binary file write routine.  The data byte is appended to the output	*/
/*  buffer.  If the buffer fills up, it gets written to disk. 			*/

void bputc(unsigned c) {
	if (outfile) {
		buf[cnt++] = c;
		if (cnt == HEXSIZE) record();
	}
}

/*  Binary file address set routine. Note that this can only be used to */
/*  seek forwards in the file. Seeking backwards will cause an error.	*/

void bseek(unsigned a) {
	unsigned cursor = (addr + cnt) & 0xFFFF;
	unsigned difference;
	int i;

	if (outfile) {
		/* initial ORG statement */
		if (cursor == 0) {
			addr = a;
		}
		/* don't allow seeking backwards */
		else if (cursor > a) {
			error('V');
		}
		/* pad the file to make up the difference */
		else {
			difference = a - cursor;
			for (i = 0; i < difference; i++) {
				bputc(0);
			}
		}
	}
}

/*  Binary file close routine. All buffered data is written to disk,	*/
/*  and the output file is closed.										*/

void bclose() {
	if (outfile) {
		if (cnt) record();
		fclose(outfile);
	}
}

static void record() {
	if (fwrite(buf, 1, cnt, outfile) != cnt) {
		fatal_error(DSKFULL);
	}

	addr += cnt;
	cnt = 0;
}

/*  Error handler routine.  If the current error code is non-blank,	*/
/*  the error code is filled in and the	number of lines with errors	*/
/*  is adjusted.							*/

void error(char code) {
	char *description;

    if (errcode == ' ') {
		errcode = code;
		++errors;
		if (pass == 2) {
			switch (code) {
			case '*':	description = ERR_STATEMENT;	break;
			case '(':	description = ERR_PAREN;		break;
			case '"':	description = ERR_QUOTE;		break;
			case 'A':	description = ERR_A;			break;
			case 'B':	description = ERR_B;			break;
			case 'D':	description = ERR_D;			break;
			case 'E':	description = ERR_E;			break;
			case 'I':	description = ERR_I;			break;
			case 'L':	description = ERR_L;			break;
			case 'M':	description = ERR_M;			break;
			case 'O':	description = ERR_O;			break;
			case 'P':	description = ERR_P;			break;
			case 'R':	description = ERR_R;			break;
			case 'S':	description = ERR_S;			break;
			case 'T':	description = ERR_T;			break;
			case 'U':	description = ERR_U;			break;
			case 'V':	description = ERR_V;			break;
			default:	description = ERR_UNKNOWN;		break;
			}

			fprintf(stderr, "%s:%d: %c -- %s\n", filestk[filesp].filename, filestk[filesp].linenum, code, description);
		}
	}
    return;
}

/*  Fatal error handler routine.  A message gets printed on the stderr	*/
/*  device, and the program bombs.					*/

void fatal_error(char *msg) {
    printf("Fatal Error -- %s\n",msg);
    exit(-1);
}

/*  Non-fatal error handler routine.  A message gets printed on the	*/
/*  stderr device, and the routine returns.				*/

void warning(char *msg) {
    printf("Warning -- %s\n",msg);
    return;
}
