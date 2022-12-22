#ifndef SET_H
#define SET_H

// ��������Ԫ�ش�С�������С�
// head->next ָ������ĵ�һ��Ԫ��
typedef struct snode {
	int elem;
	struct snode* next;
} snode, * symset;

extern symset phi;

extern symset declbegsys;

extern symset statbegsys;

extern symset facbegsys;

extern symset relset;

// ����һϵ�����ͱ�������һ����������Ԫ�ش�С��������
symset create_set(int data, .../* SYM_NULL */);

// ����һ������
void destroy_set(symset s);

// �ϲ�������������Ԫ�ش�С��������
symset unite_set(symset s1, symset s2);

// ������������޸���Ԫ��
int in_set(int elem, symset s);

#endif
// EOF set.h
