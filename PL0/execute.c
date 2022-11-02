#include "execute.h"
#include "PL0.h"

//////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff) {
	int b = currentLevel;

	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret(void) {
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do {
		i = code[pc++];
		switch (i.f) {
			case LIT:
				stack[++top] = i.a;
				break;
			case OPR:
				switch (i.a) // operator
				{
					case OPR_RET:
						top = b - 1;
						pc = stack[top + 3];
						b = stack[top + 2];
						break;
					case OPR_NEG:
						stack[top] = -stack[top];
						break;
					case OPR_ADD:
						top--;
						stack[top] += stack[top + 1];
						break;
					case OPR_MIN:
						top--;
						stack[top] -= stack[top + 1];
						break;
					case OPR_MUL:
						top--;
						stack[top] *= stack[top + 1];
						break;
					case OPR_DIV:
						top--;
						if (stack[top + 1] == 0) {
							fprintf(stderr, "Runtime Error: Divided by zero.\n");
							fprintf(stderr, "Program terminated.\n");
							continue;
						}
						stack[top] /= stack[top + 1];
						break;
					case OPR_ODD:
						stack[top] %= 2;
						break;
					case OPR_EQU:
						top--;
						stack[top] = stack[top] == stack[top + 1];
						break;
					case OPR_NEQ:
						top--;
						stack[top] = stack[top] != stack[top + 1];
						break;
					case OPR_LES:
						top--;
						stack[top] = stack[top] < stack[top + 1];
						break;
					case OPR_GEQ:
						top--;
						stack[top] = stack[top] >= stack[top + 1];
						break;
					case OPR_GTR:
						top--;
						stack[top] = stack[top] > stack[top + 1];
						break;
					case OPR_LEQ:
						top--;
						stack[top] = stack[top] <= stack[top + 1];
						break;
				} // switch
				break;
			case LOD:
				stack[++top] = stack[base(stack, b, i.l) + i.a];
				break;
			case STO:
				stack[base(stack, b, i.l) + i.a] = stack[top];
				printf("%d\n", stack[top]);
				top--;
				break;
			case CAL:
				stack[top + 1] = base(stack, b, i.l);
				// generate new block mark
				stack[top + 2] = b;
				stack[top + 3] = pc;
				b = top + 1;
				pc = i.a;
				break;
			case INT:
				top += i.a;
				break;
			case JMP:
				pc = i.a;
				break;
			case JPC:
				if (stack[top] == 0)
					pc = i.a;
				top--;
				break;
		} // switch
	} while (pc);

	printf("End executing PL/0 program.\n");
} // interpret
