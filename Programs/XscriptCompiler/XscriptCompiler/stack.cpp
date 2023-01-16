//Stack implementation



#include "stack.h"


/// <summary>
/// 初始化堆栈
/// </summary>
/// <param name="pStack"></param>
void InitStack(Stack* pStack)
{
	// Initialize the stack's internal list
	//初始化堆栈内部链表
	InitLinkedList(&pStack->ElmntList);
}

/// <summary>
/// Frees a stack.
/// 释放堆栈
/// </summary>
/// <param name="pStack"></param>
void FreeStack(Stack* pStack)
{
	// Free the stack's internal list
	//释放堆栈内部链表
	FreeLinkedList(&pStack->ElmntList);
}

/// <summary>
/// Returns TRUE if the specified stack is empty, FALSE otherwise.
/// 确定堆栈释放为空
/// </summary>
/// <param name="pStack"></param>
/// <returns></returns>
int IsStackEmpty(Stack* pStack)
{
	//检查链表节点数
	if (pStack->ElmntList.iNodeCount > 0)
		return FALSE;
	else
		return TRUE;
}

/// <summary>
/// Pushes an element onto a stack.
/// </summary>
/// <param name="pStack"></param>
/// <param name="pData"></param>
void Push(Stack* pStack, void* pData)
{
	// Add a node to the end of the stack's internal list
	//将节点放入链表尾部
	AddNode(&pStack->ElmntList, pData);
}

/// <summary>
/// Pops an element off the top of a stack.
/// </summary>
/// <param name="pStack"></param>
void Pop(Stack* pStack)
{
	// Free the tail node of the list and it's data

	DelNode(&pStack->ElmntList, pStack->ElmntList.pTail);
}

/// <summary>
/// Returns the element at the top of a stack.
/// 获取栈顶元素
/// </summary>
/// <param name="pStack"></param>
/// <returns></returns>
void* Peek(Stack* pStack)
{
	// Return the data at the tail node of the list

	return pStack->ElmntList.pTail->pData;
}
