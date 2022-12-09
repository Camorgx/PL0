#include "error.h"
#include "table.h"
#include "PL0.h"
#include "yacc.h"

#include <string.h>

// enter object(constant, variable or procedre) into table.
void enter(int kind) {
	mask* mk;

	tx++;
	strcpy_s(table[tx].name, MAXIDLEN + 1, id);
	table[tx].kind = kind;
	switch (kind) {
		case ID_CONSTANT:
			if (num > MAXADDRESS) {
				error(25); // The number is too great.
				num = 0;
			}
			table[tx].value = num;
			break;
		case ID_VARIABLE:
			mk = (mask*)&table[tx];
			mk->level = level;
			mk->address = dx++;
			break;
		case ID_PROCEDURE:
			mk = (mask*)&table[tx];
			mk->level = level;
			break;
		case ID_ARRAY:
			mk = (mask*)&table[tx];
			mk->level = level;
			mk->address = dx;
			break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char* id) {
	int i;
	strcpy_s(table[0].name, MAXIDLEN + 1, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position
