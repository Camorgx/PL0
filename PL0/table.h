#ifndef TABLE_H
#define TABLE_H

// enter object(constant, variable or procedre) into table.
extern void enter(int kind);

// locates identifier in symbol table.
extern int position(char* id);

#endif // !TABLE_H
