#include "PL0.h"
#include "error.h"
#include "execute.h"
#include "lex.h"
#include "set.h"
#include "yacc.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////
int main(void) {
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	printf("Please input source file name: "); // get file name to be compiled
	scanf_s("%s", s, 80);
	if (fopen_s(&infile, s, "r")) {
		printf("File %s can't be opened.\n", s);
		return 1;
	}

	phi = create_set(SYM_NULL);
	relset = create_set(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);

	// create begin symbol sets
	declbegsys = create_set(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = create_set(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_NULL);
	facbegsys = create_set(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = create_set(SYM_PERIOD, SYM_NULL);
	set2 = unite_set(declbegsys, statbegsys);
	set = unite_set(set1, set2);
	block(set);
	destroy_set(set1);
	destroy_set(set2);
	destroy_set(set);
	destroy_set(phi);
	destroy_set(relset);
	destroy_set(declbegsys);
	destroy_set(statbegsys);
	destroy_set(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0) {
		fopen_s(&hbin, "hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
	return 0;
} // main
