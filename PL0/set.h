#ifndef SET_H
#define SET_H

// ��������Ԫ�ش�С�������С�
// head->next ָ������ĵ�һ��Ԫ��
typedef struct snode {
	int elem;
	struct snode* next;
} snode, * symset;

symset phi;

symset declbegsys;

symset statbegsys;

symset facbegsys;

symset relset;

// ����һϵ�����ͱ�������һ����������Ԫ�ش�С��������
symset createset(int data, .../* SYM_NULL */);

// ����һ������
void destroyset(symset s);

// �ϲ�������������Ԫ�ش�С��������
symset uniteset(symset s1, symset s2);

// �������в���Ԫ��
int inset(int elem, symset s);

#endif
// EOF set.h
