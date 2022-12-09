#include "error.h"
#include "lex.h"
#include "PL0.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char line[80];

// ¹Ø¼ü×Ö
char* word[NRW + 1] = {
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while",
	"for", "print", "else", "setjmp", "longjmp" // NEW
};

// ¹Ø¼ü×Ö
int wsym[NRW + 1] = {
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE,
	SYM_FOR, SYM_PRINT, SYM_ELSE, SYM_SETJMP, SYM_LONGJMP // NEW
};

// ²Ù×÷·û
int ssym[NSYM + 1] = {
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON,
	SYM_LBRACK, SYM_RBRACK, SYM_COLON // NEW
};

// ²Ù×÷·û
char csym[NSYM + 1] = {
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';', '[', ']', ':'
};

//////////////////////////////////////////////////////////////////////
void getch(void) {
	if (cc == ll) {
		if (feof(infile)) {
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ((!feof(infile)) // added & modified by alex 01-02-09
			&& ((ch = getc(infile)) != '\n')) {
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
void getsym(void) {
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' ' || ch == '\t')
		getch();

	if (isalpha(ch)) { // symbol is a reserved word or an identifier.
		k = 0;
		do {
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		} while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy_s(id, MAXIDLEN + 1, a);
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	else if (isdigit(ch)) { // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do {
			num = num * 10 + ch - '0';
			k++;
			getch();
		} while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == ':') {
		getch();
		if (ch == '=') {
			sym = SYM_BECOMES; // :=
			getch();
		}
		else {
			sym = SYM_COLON;       // : in for
		}
	}
	else if (ch == '>') {
		getch();
		if (ch == '=') {
			sym = SYM_GEQ;     // >=
			getch();
		}
		else {
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<') {
		getch();
		if (ch == '=') {
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>') {
			sym = SYM_NEQ;     // <>
			getch();
		}
		else {
			sym = SYM_LES;     // <
		}
	}
	else { // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i) {
			sym = ssym[i];
			getch();
		}
		else {
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym
