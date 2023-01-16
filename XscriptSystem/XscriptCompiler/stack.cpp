//Stack implementation



#include "stack.h"


/// <summary>
/// ��ʼ����ջ
/// </summary>
/// <param name="pStack"></param>
void InitStack(Stack* pStack)
{
	// Initialize the stack's internal list
	//��ʼ����ջ�ڲ�����
	InitLinkedList(&pStack->ElmntList);
}

/// <summary>
/// Frees a stack.
/// �ͷŶ�ջ
/// </summary>
/// <param name="pStack"></param>
void FreeStack(Stack* pStack)
{
	// Free the stack's internal list
	//�ͷŶ�ջ�ڲ�����
	FreeLinkedList(&pStack->ElmntList);
}

/// <summary>
/// Returns TRUE if the specified stack is empty, FALSE otherwise.
/// ȷ����ջ�ͷ�Ϊ��
/// </summary>
/// <param name="pStack"></param>
/// <returns></returns>
int IsStackEmpty(Stack* pStack)
{
	//�������ڵ���
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
	//���ڵ��������β��
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
/// ��ȡջ��Ԫ��
/// </summary>
/// <param name="pStack"></param>
/// <returns></returns>
void* Peek(Stack* pStack)
{
	// Return the data at the tail node of the list

	return pStack->ElmntList.pTail->pData;
}
