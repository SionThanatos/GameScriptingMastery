//Stack implementation header
//栈依赖头文件

#ifndef XSC_STACK
#define XSC_STACK



#include "globals.h"
#include "linked_list.h"

// ---- Data Structures -----------------------------------------------------------------------


//堆栈 基于链表的堆栈不会造成内存浪费和不确定的增长收缩
typedef struct _Stack                           
{
	LinkedList ElmntList;                       // An internal linked list to hold the elements 保存元素的内存链表
}Stack;

// ---- Function Prototypes -------------------------------------------------------------------

void InitStack(Stack* pStack);
void FreeStack(Stack* pStack);

int IsStackEmpty(Stack* pStack);

void Push(Stack* pStack, void* pData);
void Pop(Stack* pStack);
void* Peek(Stack* pStack);

#endif