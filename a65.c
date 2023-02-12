/*
		      6502 Cross-Assembler in Portable C

		   Copyright (c) 1986 William C. Colley, III

Revision History:

Ver		Date		Description

0.0		NOV 1986	Derived from my 6800/6801 cross-assembler.  WCC3.

0.1		AUG 1988	Fixed a bug in the command line parser that puts it
					into a VERY long loop if the user types a command line
					like "A65 FILE.ASM -L".  WCC3 per Alex Cameron.

0.2n	JAN 2023	Changed the FCB, FCC, and FDB pseudo-ops to DB,
					DS, and DW, respectively. Made the DB pseudo-op
					able to take a string as an argument. Made labels
					able to optionally be followed with a colon character.
					Changed the assembler to output binary files instead
					of Intel HEX. Added the "!" operator to force the
					assembler to output the non-zero page version of the
					opcode. Added the "MSG" pseudo-op to output arbitrary
					text and symbol values during the assembly process.
					Made the assembler print out errors to stderr as well
					as the listing file. NPM.

This file contains the main program and line assembly routines for the
assembler.  The main program parses the command line, feeds the source lines to
the line assembly routine, and sends the results to the listing and object file
output routines.  It also coordinates the activities of everything.  The line
assembly routines uses the expression analyzer and the lexical analyzer to
parse the source line and convert it into the object bytes that it represents.
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*  Get global goodies:  */

#include "a65.h"
#include "a65eval.h"
#include "a65util.h"

/*  Define global mailboxes for all modules:				*/

char errcode, line[MAXLINE + 1], title[MAXLINE];

/* the name of the last global label parsed by the program */
char lastglobal[MAXLINE];
int pass = 0;
int eject, filesp, forwd, forceabs, listhex;
unsigned address, argattr, bytes, errors, listleft, obj[65536], pagelen, pc;
FILE_INFO filestk[FILES];
FILE *source;
TOKEN token;

/* Static function definitions: */
static void asm_line();
static void flush();
static void do_label();
static void normal_op();
static void pseudo_op();

/*  Mainline routine.  This routine parses the command line, sets up	*/
/*  the assembler at the beginning of each pass, feeds the source text	*/
/*  to the line assembler, feeds the result to the listing and hex file	*/
/*  drivers, and cleans everything up at the end of the run.		*/

static int done, ifsp, off;

int main(int argc, char **argv) {
    SCRATCH unsigned *o;

    /* printf("6502 Cross-Assembler (Portable) Ver 0.2n\n"); */
	printf("6502 Cross-Assembler (Portable), built %s\n", __DATE__);
    printf("Copyright (c) 1986 William C. Colley, III\n\n");

    while (--argc > 0) {
		if (**++argv == '-') {
			switch (toupper(*++*argv)) {
			case 'E':
				if (!*++ * argv) {
					if (!--argc) { warning(NOEXP);  break; }
					else ++argv;
				}
				eopen(*argv);
				break;

			case 'L':   
				if (!*++*argv) {
					if (!--argc) { warning(NOLST);  break; }
					else ++argv;
				}
				lopen(*argv);
				break;

			case 'O':
				if (!*++*argv) {
					if (!--argc) { warning(NOHEX);  break; }
					else ++argv;
				}
				bopen(*argv);
				break;

			default:
				warning(BADOPT);
			}
		}
		else if (filestk[0].fp) warning(TWOASM);
		else {
			filestk[0].fp = fopen(*argv, "r");
			if (!filestk[0].fp) {
				fatal_error(ASMOPEN);
			}
			strcpy(filestk[0].filename, *argv);
			filestk[0].linenum = 0;
		}
    }
    if (!filestk[0].fp) fatal_error(NOASM);

    while (++pass < 3) {
		fseek(source = filestk[0].fp,0L,0);  done = off = FALSE;
		filestk[0].linenum = 0;
		errors = filesp = ifsp = pagelen = pc = 0;  title[0] = '\0';
		while (!done) {
			errcode = ' ';
			if (newline()) {
				error('*');
				strcpy(line,"\tEND\n");
				done = eject = TRUE;  listhex = FALSE;
				bytes = 0;
			}
			else asm_line();
			pc = word(pc + bytes);
			if (pass == 2) {
				lputs();
				for (o = obj; bytes--; bputc(*o++));
			}
		}
    }

	fclose(filestk[0].fp);  eclose();  lclose();  bclose();

    if (errors) printf("%d Error(s)\n",errors);
    else printf("No Errors\n");

    exit(errors);
}

/*  Line assembly routine.  This routine gets the contents of the	*/
/*  argument field from the source file using the expression evaluator	*/
/*  and lexical analyzer.  It makes all validity checks on the		*/
/*  arguments validity, fills a buffer with the machine code bytes and	*/
/*  returns nothing.							*/

static char label[MAXLINE];
static int ifstack[IFDEPTH] = { ON };

static OPCODE *opcod;

static void asm_line() {
    SCRATCH int i;

    address = pc;  bytes = 0;  eject = forwd = forceabs = listhex = FALSE;
    for (i = 0; i < BIGINST; obj[i++] = NOP);

    label[0] = '\0';
    if ((i = popc()) != ' ' && i != '\n') {
		if (isalph(i)) {
			pushc(i);  pops(label);
			if (find_operator(label)) { label[0] = '\0';  error('L'); }
		}
		else {
			error('L');
			while ((i = popc()) != ' ' && i != '\n');
		}
    }

    trash();  opcod = NULL;
    if ((i = popc()) != '\n') {
		if (!isalph(i)) error('S');
		else {
			pushc(i);  pops(token.sval);
			if (!(opcod = find_code(token.sval))) error('O');
		}
		if (!opcod) { listhex = TRUE;  bytes = BIGINST; }
    }

    if (opcod && opcod -> attr & ISIF) { if (label[0]) error('L'); }
    else if (off) { listhex = FALSE;  flush();  return; }

    if (!opcod) { do_label();  flush(); }
    else {
		listhex = TRUE;
		if (opcod -> attr & PSEUDO) pseudo_op();
		else normal_op();
		while ((i = popc()) != '\n') if (i != ' ') error('T');
    }
    source = filestk[filesp].fp;
    return;
}

static void flush() {
    while (popc() != '\n');
}

static char labelname[MAXLINE * 2];

static void do_label() {
    SCRATCH SYMBOL *l;
	char *ch;

    if (label[0]) {
		listhex = TRUE;

		/* strip off the trailing colon if it exists */
		ch = label;
		while (*ch) {
			if ((ch[0] == ':') && (ch[1] == '\0')) {
				ch[0] = '\0';
			}
			ch++;
		}

		/* handle local labels */
		if (label[0] == '.') {
			strcpy(labelname, lastglobal);
			strcat(labelname, label);
			printf("%s\n", labelname);
		}
		else {
			strcpy(lastglobal, label);
			strcpy(labelname, label);
		}

		if (pass == 1) {
			/* add the label to the symbol tree */
			if (!((l = new_symbol(labelname)) -> attr)) {
				l -> attr = FORWD + VAL;
				l -> valu = pc;
			}
		}
		else {
			if ((l = find_symbol(labelname))) {
				l -> attr = VAL;
				if (l -> valu != pc) error('M');
			}
			else error('P');
		}
    }
}

static void normal_op() {
    SCRATCH unsigned opcode, operand;

    opcode = opcod -> valu;  bytes = BIGINST;
    do_label();  operand = do_args();
    switch (opcod -> attr) {
	case CPXY:  
		if (argattr & ARGIMM) goto do_immediate;
		opcode += 0x04;

	case BITOP: 
		if (argattr != ARGNUM) { error('A');  return; }
	    goto do_zero_page;

	case INHOP:
		if (argattr) error('T');
	    bytes = 1;  break;

	case JUMP:  
		if (argattr == (ARGIND + ARGNUM)) {
			opcode += 0x20;  break;
		}

	case CALL:  
		if (argattr != ARGNUM) { error('A');  return; }
		break;

	case LOGOP: 
		if (!(argattr & ARGA)) goto do_inc_op;
	    opcode += 0x04;  bytes = 1;  break;

	case RELBR: 
		if (argattr != ARGNUM) { error('A');  return; }
	    bytes = 2;  operand -= pc + 2;
		if (clamp(operand) > 0x007f && operand < 0xff80) {
			error('B');  operand = 0xfffe;
		}
		break;

	case STXY:  
		if ((argattr & (opcode == 0x86 ? ~ARGY : ~ARGX)) != ARGNUM) { 
			error('A');
			return;
		}
		if (argattr & (ARGX + ARGY)) {
			if (operand > 0x00ff) {	error('V');  operand = 0; }
			opcode += 0x10;  bytes = 2;  break;
		}
		goto do_zero_page;

	case TWOOP: 
		if (!(argattr & ARGNUM)) { error('A');  return; }
		if (argattr & ARGIMM) {
			if (opcode == 0x81) { error('A');  return; }
			opcode += 0x08;  goto do_immediate;
		}
		if (argattr & ARGIND) {
			if (argattr & ARGY) opcode += 0x10;
			else if (!(argattr & ARGX)) { error('A');  return; }
			if (operand > 0x00ff) { error('V');  operand = 0; }
			bytes = 2;
		}
		else if (argattr & ARGY) opcode += 0x18;
		else { opcode += 0x04;  goto do_indexed_x; }
		break;

	case LDXY:
		if (!(argattr & ARGIMM)) {
			if (opcode == 0xa2 && (argattr & ARGY))
				argattr ^= (ARGX + ARGY);
			opcode += 0x04;  goto do_inc_op;
		}
do_immediate:
		if (operand > 0x00ff && operand < 0xff80) {
			error('V');  operand = 0;
		}
		bytes = 2;  break;

do_inc_op:
	case INCOP: 
		if ((argattr & ~ARGX) != ARGNUM) { error('A');  return; }
do_indexed_x:
		if (argattr & ARGX) opcode += 0x10;
do_zero_page:
		if (!forceabs && !forwd && (operand <= 0x00ff)) bytes = 2;
		else opcode += 0x08;
		break;
    }
    obj[2] = high(operand);  obj[1] = low(operand);  obj[0] = opcode;
    return;
}

static void pseudo_op() {
    SCRATCH char *s;
    SCRATCH unsigned count, *o, result, u;
    SCRATCH SYMBOL *l;
	FILE *binfp;
	size_t size;

    o = obj;
    switch (opcod -> valu) {
	case DB:
		do_label();
		do {
			if ((lex()->attr & TYPE) == STR) {
				for (s = token.sval; *s; *o++ = *s++) {
					bytes++;
				}
				if ((lex()->attr & TYPE) != SEP) unlex();
			}
			else {
				unlex();
				if ((u = expr()) > 0xff && u < 0xff80) {
					u = 0;  error('V');
				}
				*o++ = low(u);  ++bytes;
			}
		} while ((token.attr & TYPE) == SEP);
		break;

	case DS:
		do_label();
		while ((lex()->attr & TYPE) != EOL) {
			if ((token.attr & TYPE) == STR) {
				for (s = token.sval; *s; *o++ = *s++)
					++bytes;
				if ((lex()->attr & TYPE) != SEP) unlex();
			}
			else error('S');
		}
		break;

	case DW:
		do_label();
		do {
			if ((lex()->attr & TYPE) == SEP) u = 0;
			else { unlex();  u = expr(); }
			*o++ = low(u);  *o++ = high(u);
			bytes += 2;
		} while ((token.attr & TYPE) == SEP);
		break;

	case ELSE:  
		listhex = FALSE;
		if (ifsp) off = (ifstack[ifsp] = -ifstack[ifsp]) != ON;
		else error('I');
		break;

	case END:   
		do_label();
		if (filesp) { listhex = FALSE;  error('*'); }
		else {
			done = eject = TRUE;
			/*
			if (pass == 2 && (lex() -> attr & TYPE) != EOL) {
				unlex();  bseek(address = expr());
			}*/
			if (ifsp) error('I');
		}
		break;

	case ENDI:  
		listhex = FALSE;
		if (ifsp) off = ifstack[--ifsp] != ON;
		else error('I');
		break;

	case EQU:   
		if (label[0]) {
			if (pass == 1) {
				if (!((l = new_symbol(label)) -> attr)) {
					l -> attr = FORWD + VAL;
					address = expr();
					if (!forwd) l -> valu = address;
				}
			}
			else {
				if ((l = find_symbol(label))) {
					l -> attr = VAL;
					address = expr();
					if (forwd) error('P');
					if (l -> valu != address) error('M');
				}
				else error('P');
			}
		}
		else error('L');
		break;

	case EXP:	/* label export */
		do_label();
		if (pass == 2) {
			if ((lex()->attr & TYPE) == VAL) {
				if ((l = find_symbol(token.sval))) {
					eputs(l);
				}
				else error('V');
			}
		}

		break;

	case IF:   
		if (++ifsp == IFDEPTH) fatal_error(IFOFLOW);
		address = expr();
		if (forwd) { error('P');  address = TRUE; }
		if (off) { listhex = FALSE;  ifstack[ifsp] = 0; }
		else {
			ifstack[ifsp] = address ? ON : OFF;
			if (!address) off = TRUE;
		}
		break;

	case INCB:	/* include binary */
		do_label();
		if ((lex()->attr & TYPE) == STR) {
			binfp = fopen(token.sval, "rb");
			if (!binfp) {
				error('V');
			}
			else {
				while ((*o++ = fgetc(binfp)) != EOF) {
					bytes++;
				}
				fclose(binfp);
			}
		}
		else error('S');

		break;

	case INCL:	/* include source file */
		listhex = FALSE;  do_label();
		if ((lex() -> attr & TYPE) == STR) {
			if (++filesp == FILES) fatal_error(FLOFLOW);
			filestk[filesp].fp = fopen(token.sval, "r");
			if (!filestk[filesp].fp) {
				--filesp; error('V');
			}
			else {
				strcpy(filestk[filesp].filename, token.sval);
				filestk[filesp].linenum = 0;
			}
		}
		else error('S');
		break;

	case MSG:
		do_label();
		if (pass == 2) {
			do {
				if ((lex()->attr & TYPE) == STR) {
					fputs(token.sval, stdout);
					if ((lex()->attr & TYPE) != SEP) unlex();
				}
				else {
					unlex();
					u = expr();
					printf("%d", u);
				}
			} while ((token.attr & TYPE) == SEP);
			putchar('\n');
		}
		break;

	case ALIGN:
		u = expr();
		if (forwd) error('P');
		else {
			/* calculate amount to pad the file */
			if (pc % u) u -= pc % u;
			else u = 0;
			if (pass == 2) bpad(u);
			pc += u;
			address = pc;
		}
		do_label();
		break;

	case BASE:
		u = expr();
		if (forwd) error('P');
		else pc = address = u;
		do_label();
		break;

	case ORG:   
		u = expr();
		if (forwd) error('P');
		else {
			count = u - pc;
			/* only pad if we're not at the initial offset */
			if ((pass == 2) && (pc != 0)) bpad(count);
			pc = address = u;
		}
		do_label();
		break;

	case PAGE:  
		listhex = FALSE;  do_label();
		if ((lex() -> attr & TYPE) != EOL) {
			unlex();  pagelen = expr();
			if (pagelen > 0 && pagelen < 3) {
				pagelen = 0;  error('V');
			}
		}
		eject = TRUE;
		break;

	case RMB:   
		do_label();
		u = expr();
		if (forwd) error('P');
		else {
			pc = u;
			if (pass == 2) bpad(u);
		}
		break;

	case SET:   
		if (label[0]) {
			if (pass == 1) {
				if (!((l = new_symbol(label)) -> attr) || (l -> attr & SOFT)) {
					l -> attr = FORWD + SOFT + VAL;
					address = expr();
					if (!forwd) l -> valu = address;
				}
			}
			else {
				if ((l = find_symbol(label))) {
					address = expr();
					if (forwd) error('P');
					else if (l -> attr & SOFT) {
						l -> attr = SOFT + VAL;
						l -> valu = address;
					}
					else error('M');
				}
				else error('P');
			}
		}
		else error('L');
		break;

	case TITL:  
		listhex = FALSE;  do_label();
		if ((lex() -> attr & TYPE) == EOL) title[0] = '\0';
		else if ((token.attr & TYPE) != STR) error('S');
		else strcpy(title,token.sval);
		break;
    }
    return;
}
