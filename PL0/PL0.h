#ifndef PL0_H
#define PL0_H

#include <stdio.h>

#define NRW        16     // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       13     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      500    // size of code array

#define MAXSYM     30     // maximum number of symbols  

#define STACKSIZE  1000   // maximum storage

#define MAXINS   12 // ���ָ����

#define MAX_ARRAY_DIM_LEN 32 // ����ÿһά����󳤶�
#define MAX_ARRAY_DIM 8 // ��������ά��

enum symtype {
	SYM_NULL, // ��
	SYM_IDENTIFIER, // ��ʶ��
	SYM_NUMBER, // ����
	SYM_PLUS, // +
	SYM_MINUS, // -
	SYM_TIMES, // *
	SYM_SLASH, // /
	SYM_ODD, // % 2
	SYM_EQU, // =
	SYM_NEQ, // !=
	SYM_LES, // <
	SYM_LEQ, // <=
	SYM_GTR, // >
	SYM_GEQ, // >=
	SYM_LPAREN, // (
	SYM_RPAREN, // )
	SYM_COMMA, // ,
	SYM_SEMICOLON, // ;
	SYM_PERIOD, // .
	SYM_BECOMES, // :=
	SYM_BEGIN, // begin
	SYM_END, // end
	SYM_IF, // if
	SYM_THEN, // then
	SYM_WHILE, // while
	SYM_DO, // do
	SYM_CALL, // call
	SYM_CONST,	// const
	SYM_VAR,	//var
	SYM_PROCEDURE,	//procedure
	SYM_FOR,	// for
	SYM_PRINT,	// print
	SYM_ELSE,	// else
	SYM_SETJMP, // setjmp
	SYM_LONGJMP, // longjmp
	SYM_LBRACK, // [
	SYM_RBRACK, // ]
	SYM_COLON // :
};

enum idtype {
	ID_CONSTANT, 
	ID_VARIABLE, 
	ID_PROCEDURE,
	ID_ARRAY
};

enum opcode {
	LIT, // ����������ջ��
	OPR, // һ���������߼�����ָ��
	LOD, // ������ֵ����ջ��
	STO, // ��ջ����ֵ����ĳ����
	CAL, // ���ڹ��̵��õ�ָ��
	INT, // ������ջ�з�������ռ�
	JMP, // ��תָ��
	JPC, // ������ת
	PRT, // ���
	LODA, // ��Ӷ�
	LEA, // ��ջ��ѹ������ľ��Ե�ַ
	STOA, // ���д
};

enum oprcode {
	OPR_RET, // return
	OPR_NEG, // -
	OPR_ADD, // +
	OPR_MIN, // -
	OPR_MUL, // *
	OPR_DIV, // /
	OPR_ODD, // % 2
	OPR_EQU, // =
	OPR_NEQ, // !=
	OPR_LES, // <
	OPR_LEQ, // <=
	OPR_GTR, // >
	OPR_GEQ // >=
};

typedef struct {
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

typedef struct {
	char name[MAXIDLEN + 1];
	int  kind;
	int  value;
	int  dimension[8];
} comtab;

typedef struct {
	char  name[MAXIDLEN + 1];
	int   kind;
	short level;
	short address;
	int   dimension[8];
} mask;

//////////////////////////////////////////////////////////////////////
extern char ch;         // last character read
extern int  sym;        // last symbol read
extern char id[MAXIDLEN + 1]; // last identifier read
extern int  num;        // last number read
extern int  cc;         // character count
extern int  ll;         // line length
extern int  kk;
extern int  err;
extern int  cx;         // index of current instruction to be generated.
extern int  level;
extern int  tx;

extern FILE* infile;

extern instruction code[CXMAX];

extern comtab table[TXMAX];

#endif // !PL0_H
// EOF PL0.h
