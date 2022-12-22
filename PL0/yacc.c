#include "error.h"
#include "lex.h"
#include "PL0.h"
#include "table.h"
#include "yacc.h"

#include <stdlib.h>

char* mnemonic[MAXINS] = {
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC", 
	"PRT", "LODA", "LEA", "STOA", "LIFT"
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
			set1 = create_set(SYM_SEMICOLON, SYM_NULL);
			set = unite_set(set1, fsys);
			block(set);
			destroy_set(set1);
			destroy_set(set);
			tx = savedTx;
			level--;

			if (sym == SYM_SEMICOLON) {
				getsym();
				set1 = create_set(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = unite_set(statbegsys, set1);
				test(set, fsys, 6);
				destroy_set(set1);
				destroy_set(set);
			}
			else {
				error(5); // Missing ',' or ';'.
			}
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = create_set(SYM_IDENTIFIER, SYM_NULL);
		set = unite_set(statbegsys, set1);
		test(set, declbegsys, 7);
		destroy_set(set1);
		destroy_set(set);
	} while (in_set(sym, declbegsys));

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);
	set1 = create_set(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = unite_set(set1, fsys);
	statement(set);
	destroy_set(set1);
	destroy_set(set);
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
		set1 = create_set(SYM_THEN, SYM_DO, SYM_NULL);
		set = unite_set(set1, fsys);
		condition(set);
		destroy_set(set1);
		destroy_set(set);
		if (sym == SYM_THEN) {
			getsym();
		}
		else {
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		int cx3 = cx;
		gen(JMP, 0, 0);
		getsym();
		if (sym == SYM_ELSE) {
			cx2 = cx;
			getsym();
			statement(fsys);
		}
		else cx2 = cx;
		code[cx1].a = cx2;
		code[cx3].a = cx;
	}
	else if (sym == SYM_BEGIN) { // block
		getsym();
		set1 = create_set(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = unite_set(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || in_set(sym, statbegsys)) {
			if (sym == SYM_SEMICOLON) {
				getsym();
			}
			else {
				error(10);
			}
			statement(set);
		} // while
		destroy_set(set1);
		destroy_set(set);
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
		set1 = create_set(SYM_DO, SYM_NULL);
		set = unite_set(set1, fsys);
		condition(set);
		destroy_set(set1);
		destroy_set(set);
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
	else if (sym == SYM_FOR) {
		getsym();
		if (sym == SYM_LPAREN) {
			getsym();
			mask* mk = NULL;
			if (sym == SYM_VAR) {
				getsym();
				if (sym == SYM_IDENTIFIER) {
					getsym();
					if (sym == SYM_COLON) {
						enter(ID_VARIABLE);
						getsym();
						if (sym == SYM_LPAREN) {
							set1 = create_set(SYM_COMMA, SYM_NULL);
							set = unite_set(fsys, set1);
							getsym();
							expression(set); // low
							mk = (mask*)(table + tx);
							gen(STO, level - mk->level, mk->address);
							destroy_set(set);
							destroy_set(set1);
							if (sym == SYM_COMMA) {
								set1 = create_set(SYM_COMMA, SYM_RPAREN, SYM_NULL);
								set = unite_set(fsys, set1);
								getsym();
								expression(set); // 向栈顶压入 up
								destroy_set(set);
								destroy_set(set1);
								if (sym == SYM_COMMA) {
									set1 = create_set(SYM_RPAREN, SYM_NULL);
									set = unite_set(fsys, set1);
									getsym();
									expression(set); // 向栈顶压入 step
									destroy_set(set);
									destroy_set(set1);
								}
								else if (sym == SYM_RPAREN) {
									gen(LIT, 0, 1);
								}
							}
							else error(26); // NEW ERROR
							if (sym != SYM_RPAREN)
								error(26); // NEW ERROR
						}
						else error(26); // NEW ERROR
					}
					else error(26); // NEW ERROR
				}
				else error(26); // NEW ERROR
			}
			else error(26); // NEW ERROR
			getsym();
			if (sym != SYM_RPAREN) error(26); // NEW ERROR
			/*
				if step < 0 // id >= high exec
					tmp = (id >= high);
				else // id <= high exec
					tmp = (id <= high);
				if tmp == 0
					jmp 
			*/
			if (!mk) goto TEST;
			cx1 = cx;
			gen(LOD, level - mk->level, mk->address);
			gen(LIFT, 0, 2); // stack[top] = high
			gen(LIFT, 0, 2); // copy step to top
			gen(LIT, 0, 0);
			gen(OPR, 0, OPR_LES); // if step < 0
			gen(JPC, 0, cx + 3); // 跳转到 tmp = (id <= high);
			// 下面执行 tmp = (id >= high);
			gen(OPR, 0, OPR_GEQ); // stack[top] = tmp
			gen(JMP, 0, cx + 2);
			// 下面执行 tmp = (id <= high);
			gen(OPR, 0, OPR_LEQ); // stack[top] = tmp
			cx2 = cx;
			gen(JPC, 0, 0); //跳出循环
			getsym();
			statement(fsys);
			gen(LOD, level - mk->level, mk->address);
			gen(LIFT, 0, 1); // copy step to top
			gen(OPR, 0, OPR_ADD);
			gen(STO, level - mk->level, mk->address);
			gen(JMP, 0, cx1);
			code[cx2].a = cx;
			gen(INT, 0, -2); // 从栈顶弹出 high 和 step
			--tx;
		}
		else error(26); // NEW ERROR
	}
	else if (sym == SYM_PRINT) {
		getsym();
		if (sym == SYM_LPAREN) {
			getsym();
			if (sym == SYM_RPAREN) {
				gen(PRT, 255, 0);
			}
			else {
				set1 = create_set(SYM_RPAREN, SYM_COMMA, SYM_NULL);
				set = unite_set(set1, fsys);
				expression(set);
				gen(PRT, 0, 0);
				while (sym == SYM_COMMA) {
					getsym();
					expression(set);
					gen(PRT, 0, 0);
				}
				destroy_set(set1);
				destroy_set(set);
				if (sym == SYM_RPAREN) gen(PRT, 255, 0);
				else error(26); // NEW ERROR
			}
			getsym();
		}
		else error(26); // NEW ERROR
	}
	TEST: test(fsys, phi, 19);
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
		set = unite_set(relset, fsys);
		expression(set);
		destroy_set(set);
		if (!in_set(sym, relset)) {
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

	set = unite_set(fsys, create_set(SYM_PLUS, SYM_MINUS, SYM_NULL));

	if (sym == SYM_IDENTIFIER) { // variable assignment
		int i;
		if (!(i = position(id))) {
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind == ID_VARIABLE) {
			getsym();
			mask* mk = (mask*)&table[i];
			if (sym == SYM_BECOMES) {
				getsym();
				expression(fsys);
				if (i) {
					gen(LIFT, 0, 0);
					gen(STO, level - mk->level, mk->address);
				}
				goto END_EXPRESSION;
			}
			else {
				gen(LOD, level - mk->level, mk->address);
				goto NOT_BECOME_EXPRESSION;
			}
		}
		else if (table[i].kind == ID_ARRAY) {
			getsym();
			mask* mk = (mask*)&table[i];
			if (sym == SYM_LBRACK) {
				gen(LIT, 0, 0);
				dim_position(fsys, i, 0);
				gen(LEA, level - mk->level, mk->address);
				gen(OPR, 0, OPR_ADD);

				if (sym == SYM_BECOMES) {
					getsym();
					expression(fsys);
					if (i) {
						gen(STOA, 0, 0);
						gen(LIFT, 0, -2); // 把刚刚弹出栈的其中一个值放回到栈顶
					}
					goto END_EXPRESSION;
				}
				else {
					gen(LODA, 0, 0);
					goto NOT_BECOME_EXPRESSION;
				}
			}
			else error(26); // NEW ERROR
		}
		else {
			error(12); // Illegal assignment.
			i = 0;
		}
	}
	// 114

	term(set);
NOT_BECOME_EXPRESSION:
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

END_EXPRESSION:
	destroy_set(set);
} // expression

//////////////////////////////////////////////////////////////////////
void term(symset fsys) {
	int mulop;
	symset set;

	set = unite_set(fsys, create_set(SYM_TIMES, SYM_SLASH, SYM_NULL));
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
	destroy_set(set);
} // term

void dim_position(symset fsys, int i, int dimension) {
	if (table[i].dimension[dimension] == 0)
		error(26); // NEW ERROR
	getsym();
	symset set = unite_set(fsys, create_set(SYM_RBRACK, SYM_NULL));
	expression(set);
	destroy_set(set);
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

	if (in_set(sym, facbegsys)) {
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
			set = unite_set(create_set(SYM_RPAREN, SYM_NULL), fsys);
			expression(set);
			destroy_set(set);
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
		test(fsys, create_set(SYM_LPAREN, SYM_NULL), 23);
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
