#ifndef YACC_H
#define YACC_H

#include "set.h"

extern int dx;  // data allocation index

extern void block(symset fsys);

extern void constdeclaration(void);

extern void vardeclaration(void);

extern void statement(symset fsys);

extern void condition(symset fsys);

extern void expression(symset fsys);

extern void term(symset fsys);

extern void factor(symset fsys);

// generates (assembles) an instruction.
extern void gen(int x, int y, int z);

extern void listcode(int from, int to);

#endif // !YACC_H