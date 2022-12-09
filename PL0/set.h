#ifndef SET_H
#define SET_H

// 优先链表，元素从小到大排列。
// head->next 指向链表的第一个元素
typedef struct snode {
	int elem;
	struct snode* next;
} snode, * symset;

extern symset phi;

extern symset declbegsys;

extern symset statbegsys;

extern symset facbegsys;

extern symset relset;

// 基于一系列整型变量建立一个优先链表，元素从小到大排列
symset createset(int data, .../* SYM_NULL */);

// 销毁一个链表
void destroyset(symset s);

// 合并两个优先链表，元素从小到大排列
symset uniteset(symset s1, symset s2);

// 检查链表中有无给定元素
int inset(int elem, symset s);

#endif
// EOF set.h
