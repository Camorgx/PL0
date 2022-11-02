#ifndef ERROR_H
#define ERROR_H

#include "set.h"

// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
extern void test(symset s1, symset s2, int n);

// print error message.
extern void error(int n);

#endif // ERROR_H