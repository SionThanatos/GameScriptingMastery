//Linked list implementation
//��������

#include "linked_list.h"




/// <summary>
/// Initializes a linked list.
/// ��ʼ������
/// </summary>
/// <param name="pList"></param>
void InitLinkedList(LinkedList* pList)
{
	// Set both the head and tail pointers to null
	//����β�ڵ�ָ������ΪNULL
	pList->pHead = NULL;
	pList->pTail = NULL;

	// Set the node count to zero, since the list is currently empty
	//���ýڵ����Ϊ0
	pList->iNodeCount = 0;
}

/// <summary>
/// Frees a linked list.
/// �ͷ�����
/// </summary>
/// <param name="pList"></param>
void FreeLinkedList(LinkedList* pList)
{
	// If the list is empty, exit
	//����Ϊ�գ��˳�
	if (!pList)
		return;

	// If the list is not empty, free each node
	//��Ϊ�գ��ͷ�ÿһ���ڵ�
	if (pList->iNodeCount)
	{
		// Create a pointer to hold each current node and the next node
		//����һ��ָ�뱣�ֵ�ǰ�ڵ����һ���ڵ�
		LinkedListNode* pCurrNode,
				      * pNextNode;

		// Set the current node to the head of the list
		//���õ�ǰָ��ָ���׽ڵ�
		pCurrNode = pList->pHead;

		// Traverse the list
		//��������
		while (TRUE)
		{
			// Save the pointer to the next node before freeing the current one
			//�ͷŵ�ǰ�ڵ�ǰ����ָ����һ���ڵ��ָ��
			pNextNode = pCurrNode->pNext;

			// Clear the current node's data
			//��յ�ǰ�ڵ�����
			if (pCurrNode->pData)
				free(pCurrNode->pData);

			// Clear the node itself
			//����ڵ�����
			if (pCurrNode)
				free(pCurrNode);

			// Move to the next node if it exists; otherwise, exit the loop
			//���������һ���ڵ���ƶ�����һ���ڵ㣻�������˳�ѭ��
			if (pNextNode)
				pCurrNode = pNextNode;
			else
				break;
		}
	}
}

/// <summary>
/// Adds a node to a linked list and returns its index.
/// ����ڵ�
/// </summary>
/// <param name="pList"></param>
/// <param name="pData"></param>
/// <returns></returns>
int AddNode(LinkedList* pList, void* pData)
{
	// Create a new node

	LinkedListNode* pNewNode = (LinkedListNode*)malloc(sizeof(LinkedListNode));

	// Set the node's data to the specified pointer

	pNewNode->pData = pData;

	// Set the next pointer to NULL, since nothing will lie beyond it

	pNewNode->pNext = NULL;

	// If the list is currently empty, set both the head and tail pointers to the new node

	if (!pList->iNodeCount)
	{
		// Point the head and tail of the list at the node

		pList->pHead = pNewNode;
		pList->pTail = pNewNode;
	}

	// Otherwise append it to the list and update the tail pointer

	else
	{
		// Alter the tail's next pointer to point to the new node

		pList->pTail->pNext = pNewNode;

		// Update the list's tail pointer

		pList->pTail = pNewNode;
	}

	// Increment the node count

	++pList->iNodeCount;

	// Return the new size of the linked list - 1, which is the node's index

	return pList->iNodeCount - 1;
}

/// <summary>
/// Deletes a node from a linked list.
/// ɾ���ڵ�
/// </summary>
/// <param name="pList"></param>
/// <param name="pNode"></param>
void DelNode(LinkedList* pList, LinkedListNode* pNode)
{
	// If the list is empty, return

	if (pList->iNodeCount == 0)
		return;

	// Determine if the head node is to be deleted

	if (pNode == pList->pHead)
	{
		// If so, point the list head pointer to the node just after the current head

		pList->pHead = pNode->pNext;
	}
	else
	{
		// Otherwise, traverse the list until the specified node's previous node is found

		LinkedListNode* pTravNode = pList->pHead;
		for (int iCurrNode = 0; iCurrNode < pList->iNodeCount; ++iCurrNode)
		{
			// Determine if the current node's next node is the specified one

			if (pTravNode->pNext == pNode)
			{
				// Determine if the specified node is the tail

				if (pList->pTail == pNode)
				{
					// If so, point this node's next node to NULL and set it as the new
					// tail

					pTravNode->pNext = NULL;
					pList->pTail = pTravNode;
				}
				else
				{
					// If not, patch this node to the specified one's next node

					pTravNode->pNext = pNode->pNext;
				}
				break;
			}

			// Move to the next node

			pTravNode = pTravNode->pNext;
		}
	}

	// Decrement the node count

	--pList->iNodeCount;

	// Free the data

	if (pNode->pData)
		free(pNode->pData);

	// Free the node structure

	free(pNode);
}

/// <summary>
/// Adds a string to a linked list, blocking duplicate entries
/// �����ַ����ڵ�
/// </summary>
/// <param name="pList"></param>
/// <param name="pstrString"></param>
/// <returns></returns>
int AddString(LinkedList* pList, char* pstrString)
{
	//! First check to see if the string is already in the list ���ȼ������ַ����Ƿ��Ѿ��������д���

	// Create a node to traverse the list
	//����һ��ָ��ָ��ͷ�����ڱ�������
	LinkedListNode* pNode = pList->pHead;

	// Loop through each node in the list
	//ѭ���б�
	for (int iCurrNode = 0; iCurrNode < pList->iNodeCount; ++iCurrNode)
	{
		// If the current node's string equals the specified string, return its index

		if (strcmp((char*)pNode->pData, pstrString) == 0)
			return iCurrNode;

		// Otherwise move along to the next node

		pNode = pNode->pNext;
	}

	//! Add the new string, since it wasn't added ����ַ���

	// Create space on the heap for the specified string
	//Ϊ����ַ����ڶ��������ڴ�
	char* pstrStringNode = (char*)malloc(strlen(pstrString) + 1);
	strcpy(pstrStringNode, pstrString);

	// Add the string to the list and return its index
	//������ӵ���������������
	return AddNode(pList, pstrStringNode);
}

/// <summary>
/// Returns a string from a linked list based on its index.
/// �����ַ����ڵ㡣��������
/// </summary>
/// <param name="pList"></param>
/// <param name="iIndex"></param>
/// <returns></returns>
char* GetStringByIndex(LinkedList* pList, int iIndex)
{
	// Create a node to traverse the list

	LinkedListNode* pNode = pList->pHead;

	// Loop through each node in the list

	for (int iCurrNode = 0; iCurrNode < pList->iNodeCount; ++iCurrNode)
	{
		// If the current node's string equals the specified string, return its index

		if (iIndex == iCurrNode)
			return (char*)pNode->pData;

		// Otherwise move along to the next node

		pNode = pNode->pNext;
	}

	// Return a null string if the index wasn't found

	return NULL;
}