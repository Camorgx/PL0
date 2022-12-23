#include "error.h"
#include "lex.h"
#include "PL0.h"

//////////////////////////////////////////////////////////////////////
char* err_msg[] = {
	/*  0 */    "",
	/*  1 */    "Found ':=' when expecting '='.",
	/*  2 */    "There must be a number to follow '='.",
	/*  3 */    "There must be an '=' to follow the identifier.",
	/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
	/*  5 */    "Missing ',' or ';'.",
	/*  6 */    "Incorrect procedure name.",
	/*  7 */    "Statement expected.",
	/*  8 */    "Follow the statement is an incorrect symbol.",
	/*  9 */    "'.' expected.",
	/* 10 */    "';' expected.",
	/* 11 */    "Undeclared identifier.",
	/* 12 */    "Illegal assignment.",
	/* 13 */    "':=' expected.",
	/* 14 */    "There must be an identifier to follow the 'call'.",
	/* 15 */    "A constant or variable can not be called.",
	/* 16 */    "'then' expected.",
	/* 17 */    "';' or 'end' expected.",
	/* 18 */    "'do' expected.",
	/* 19 */    "Incorrect symbol.",
	/* 20 */    "Relative operators expected.",
	/* 21 */    "Procedure identifier can not be in an expression.",
	/* 22 */    "Missing ')'.",
	/* 23 */    "The symbol can not be followed by a factor.",
	/* 24 */    "The symbol can not be as the beginning of an expression.",
	/* 25 */    "The number is too great.",
	/* 26 */    "NEW ERROR",
	/* 27 */    "Array dimension exceeded.",
	/* 28 */    "Lengthes of array must be constant.",
	/* 29 */    "The maximum length of each dimension of the array exceeded.",
	/* 30 */    "The symbol can not follow an array declaration.",
	/* 31 */    "You cannot assign an array directly.",
	/* 32 */    "There are too many levels.",
	/* 33 */	"Missing '('.",
	/* 34 */	"Missing ':'.",
	/* 35 */	"Missing 'var'.",
	/* 36 */	"Missing ','.",
	/* 37 */    "You cannot access an array directly.",
};

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n) {
	symset s;

	if (!in_set(sym, s1)) {
		error(n);
		s = unite_set(s1, s2);
		while (!in_set(sym, s))
			getsym();
		destroy_set(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n) {
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error
