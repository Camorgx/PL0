#ifndef EXECUTE_H
#define EXECUTE_H

extern int base(int stack[], int currentLevel, int levelDiff);

// interprets and executes codes.
extern void interpret(void);

extern void listcode(int from, int to);

#endif // !EXECUTE_H

