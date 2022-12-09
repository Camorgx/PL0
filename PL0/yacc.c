#include "error.h"
#include "lex.h"
#include "PL0.h"
#include "table.h"
#include "yacc.h"

#include <stdlib.h>

char* mnemonic[MAXINS] = {
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC", "PRT", "LODA", "LEA", "STOA"
};

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index

//////////////////////////////////////////////////////////////////////
void block(symset fsys) {
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	dx = 3;
	block_dx = dx;
	mk = (mask*)&table[tx];
	mk->address = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL) {
		error(32); // There are too many levels.
	}
	do {
		if (sym == SYM_CONST) { // constant declarations
			getsym();
			do {
				constdeclaration();
				while (sym == SYM_COMMA) {
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON) {
					getsym();
				}
				else {
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if

		if (sym == SYM_VAR) { // variable declarations
			getsym();
			do {
				vardeclaration();
				while (sym == SYM_COMMA) {
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON) {
					getsym();
				}
				else {
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if
		block_dx = dx; //save dx before handling procedure call!
		while (sym == SYM_PROCEDURE) { // procedure declarations
			getsym();
			if (sym == SYM_IDENTIFIER) {
				enter(ID_PROCEDURE);
				getsym();
			}
			else {
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}


			if (sym == SYM_SEMICOLON) {
				getsym();
			}
			else {
				error(5); // Missing ',' or ';'.
			}

			level++;
			savedTx = tx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			level--;

			if (sym == SYM_SEMICOLON) {
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else {
				error(5); // Missing ',' or ';'.
			}
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	} while (inset(sym, declbegsys));

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
} // block

//////////////////////////////////////////////////////////////////////
void constdeclaration(void) {
	if (sym == SYM_IDENTIFIER) {
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES) {
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER) {
				enter(ID_CONSTANT);
				getsym();
			}
			else {
				error(2); // There must be a number to follow '='.
			}
		}
		else {
			error(3); // There must be an '=' to follow the identifier.
		}
	}
	else	error(4);
  // There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

void dim_declaration(int dimension) {
	if (dimension > MAX_ARRAY_DIM) {
		error(26); // NEW ERROR
	}
	getsym();
	if (sym == SYM_IDENTIFIER) {
		int i;
		if ((i = position(id)) == 0) {
			error(11); // Undeclared identifier.
		}
		else {
			switch (table[i].kind) {
				case ID_CONSTANT:
					table[tx].dimension[dimension] = table[i].value;
					break;
				case ID_VARIABLE:
					error(26); // NEW ERROR
					break;
				case ID_PROCEDURE:
					error(26); // NEW ERROR
					break;
			} // switch
		}
	}
	else if (sym == SYM_NUMBER) {
		if (num >= MAX_ARRAY_DIM_LEN) error(26); // NEW ERROR
		else {
			table[tx].dimension[dimension] = num;
		}
	}
	else error(26); // NEW ERROR

	getsym();
	if (sym == SYM_RBRACK) {
		getsym();
		if (sym == SYM_LBRACK)
			dim_declaration(dimension + 1);
		else if (sym == SYM_COMMA || sym == SYM_SEMICOLON) {
			table[tx].dimension[dimension + 1] = 0;
			int array_size = 1;
			for (int i = 0; table[tx].dimension[i]; ++i)
				array_size *= table[tx].dimension[i];
			dx += array_size;
		}
		else error(26); // NEW ERROR
	}
}

//////////////////////////////////////////////////////////////////////
void vardeclaration(void) {
	if (sym == SYM_IDENTIFIER) {
		getsym();
		if (sym == SYM_COMMA || sym == SYM_SEMICOLON) {
			enter(ID_VARIABLE);
		}
		else if (sym == SYM_LBRACK) {
			enter(ID_ARRAY);
			dim_declaration(0);
		}
		else error(26); // NEW ERROR
	}
	else {
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
void statement(symset fsys) {
	int i, cx1, cx2;
	symset set1, set;

	if (sym == SYM_IDENTIFIER) { // variable assignment
		mask* mk;
		if (!(i = position(id))) {
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind == ID_VARIABLE) {
			getsym();
			if (sym == SYM_BECOMES) {
				getsym();
			}
			else {
				error(13); // ':=' expected.
			}
			expression(fsys);
			mk = (mask*)&table[i];
			if (i) {
				gen(STO, level - mk->level, mk->address);
			}
		}
		else if (table[i].kind == ID_ARRAY) {
			getsym();
			if (sym == SYM_LBRACK) {
				mk = (mask*)&table[i];
				gen(LIT, 0, 0);
				dim_position(fsys, i, 0);
				gen(LEA, level - mk->level, mk->address);
				gen(OPR, 0, OPR_ADD);
				
				if (sym == SYM_BECOMES) {
					getsym();
				}
				else {
					error(13); // ':=' expected.
				}
				expression(fsys);
				if (i) {
					gen(STOA, 0, 0);
				}
			}
			else error(26); // NEW ERROR
		}
		else {
			error(12); // Illegal assignment.
			i = 0;
		}
	}
	else if (sym == SYM_CALL) { // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER) {
			error(14); // There must be an identifier to follow the 'call'.
		}
		else {
			if (!(i = position(id))) {
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE) {
				mask* mk;
				mk = (mask*)&table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else {
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	}
	else if (sym == SYM_IF) { // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN) {
			getsym();
		}
		else {
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		code[cx1].a = cx;
	}
	else if (sym == SYM_BEGIN) { // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys)) {
			if (sym == SYM_SEMICOLON) {
				getsym();
			}
			else {
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END) {
			getsym();
		}
		else {
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE) { // while statement
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO) {
			getsym();
		}
		else {
			error(18); // 'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
	}
	else if (sym == SYM_PRINT) {
		getsym();
		if (sym == SYM_LPAREN) {
			getsym();
			if (sym == SYM_RPAREN) {
				gen(PRT, 255, 0);
			}
			else {
				set1 = createset(SYM_RPAREN, SYM_COMMA, SYM_NULL);
				set = uniteset(set1, fsys);
				expression(set);
				gen(PRT, 0, 0);
				while (sym == SYM_COMMA) {
					getsym();
					expression(set);
					gen(PRT, 0, 0);
				}
				destroyset(set1);
				destroyset(set);
				if (sym == SYM_RPAREN) gen(PRT, 255, 0);
				else error(26); // NEW ERROR
			}
			getsym();
		}
		else error(26); // NEW ERROR
	}
	test(fsys, phi, 19);
} // statement

//////////////////////////////////////////////////////////////////////
void condition(symset fsys) {
	int relop;
	symset set;

	if (sym == SYM_ODD) {
		getsym();
		expression(fsys);
		gen(OPR, 0, SYM_ODD);
	}
	else {
		set = uniteset(relset, fsys);
		expression(set);
		destroyset(set);
		if (!inset(sym, relset)) {
			error(20);
		}
		else {
			relop = sym;
			getsym();
			expression(fsys);
			switch (relop) {
				case SYM_EQU:
					gen(OPR, 0, OPR_EQU);
					break;
				case SYM_NEQ:
					gen(OPR, 0, OPR_NEQ);
					break;
				case SYM_LES:
					gen(OPR, 0, OPR_LES);
					break;
				case SYM_GEQ:
					gen(OPR, 0, OPR_GEQ);
					break;
				case SYM_GTR:
					gen(OPR, 0, OPR_GTR);
					break;
				case SYM_LEQ:
					gen(OPR, 0, OPR_LEQ);
					break;
			} // switch
		} // else
	} // else
} // condition

//////////////////////////////////////////////////////////////////////
void expression(symset fsys) {
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));

	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS) {
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS) {
			gen(OPR, 0, OPR_ADD);
		}
		else {
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
} // expression

//////////////////////////////////////////////////////////////////////
void term(symset fsys) {
	int mulop;
	symset set;

	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH) {
		mulop = sym;
		getsym();
		factor(set);
		if (mulop == SYM_TIMES) {
			gen(OPR, 0, OPR_MUL);
		}
		else {
			gen(OPR, 0, OPR_DIV);
		}
	} // while
	destroyset(set);
} // term

void dim_position(symset fsys, int i, int dimension) {
	if (table[i].dimension[dimension] == 0)
		error(26); // NEW ERROR
	getsym();
	symset set = uniteset(fsys, createset(SYM_RBRACK, SYM_NULL));
	expression(set);
	destroyset(set);
	if (sym == SYM_RBRACK) {
		gen(OPR, 0, OPR_ADD);
		if (table[i].dimension[dimension + 1] == 0)
			gen(LIT, 0, 1);
		else gen(LIT, 0, table[i].dimension[dimension + 1]);
		gen(OPR, 0, OPR_MUL);
		
		getsym();
		if (sym == SYM_LBRACK)
			dim_position(fsys, i, dimension + 1);
	}
}

//////////////////////////////////////////////////////////////////////
void factor(symset fsys) {
	void expression(symset fsys);
	int i;
	symset set;

	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys)) {
		if (sym == SYM_IDENTIFIER) {
			if ((i = position(id)) == 0) {
				error(11); // Undeclared identifier.
			}
			else {
				switch (table[i].kind) {
					mask* mk;
					case ID_CONSTANT:
						gen(LIT, 0, table[i].value);
						getsym();
						break;
					case ID_VARIABLE:
						mk = (mask*)&table[i];
						gen(LOD, level - mk->level, mk->address);
						getsym();
						break;
					case ID_ARRAY:
						getsym();
						if (sym == SYM_LBRACK) {
							mk = (mask*)&table[i];
							gen(LIT, 0, 0);
							dim_position(fsys, i, 0);
							gen(LEA, level - mk->level, mk->address);
							gen(OPR, 0, OPR_ADD);
							gen(LODA, 0, 0);
						}
						else error(26); // NEW ERROR
						break;
					case ID_PROCEDURE:
						error(21); // Procedure identifier can not be in an expression.
						getsym();
						break;
				} // switch
			}
		}
		else if (sym == SYM_NUMBER) {
			if (num > MAXADDRESS) {
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN) {
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN) {
				getsym();
			}
			else {
				error(22); // Missing ')'.
			}
		}
		else if (sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{
			getsym();
			factor(fsys);
			gen(OPR, 0, OPR_NEG);
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // if
} // factor

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
void gen(int x, int y, int z) {
	if (cx > CXMAX) {
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

//////////////////////////////////////////////////////////////////////
void listcode(int from, int to) {
	int i;

	printf("\n");
	for (i = from; i < to; i++) {
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode
