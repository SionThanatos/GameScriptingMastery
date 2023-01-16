//Stack implementation header
//ջ����ͷ�ļ�

#ifndef XSC_STACK
#define XSC_STACK



#include "globals.h"
#include "linked_list.h"

// ---- Data Structures -----------------------------------------------------------------------


//��ջ ��������Ķ�ջ��������ڴ��˷ѺͲ�ȷ������������
typedef struct _Stack                           
{
	LinkedList ElmntList;                       // An internal linked list to hold the elements ����Ԫ�ص��ڴ�����
}Stack;

// ---- Function Prototypes -------------------------------------------------------------------

void InitStack(Stack* pStack);
void FreeStack(Stack* pStack);

int IsStackEmpty(Stack* pStack);

void Push(Stack* pStack, void* pData);
void Pop(Stack* pStack);
void* Peek(Stack* pStack);

#endif