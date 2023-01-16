//Linked list implementation
//链表依赖

#include "linked_list.h"




/// <summary>
/// Initializes a linked list.
/// 初始化链表
/// </summary>
/// <param name="pList"></param>
void InitLinkedList(LinkedList* pList)
{
	// Set both the head and tail pointers to null
	//将首尾节点指针设置为NULL
	pList->pHead = NULL;
	pList->pTail = NULL;

	// Set the node count to zero, since the list is currently empty
	//设置节点计数为0
	pList->iNodeCount = 0;
}

/// <summary>
/// Frees a linked list.
/// 释放链表
/// </summary>
/// <param name="pList"></param>
void FreeLinkedList(LinkedList* pList)
{
	// If the list is empty, exit
	//链表为空，退出
	if (!pList)
		return;

	// If the list is not empty, free each node
	//不为空，释放每一个节点
	if (pList->iNodeCount)
	{
		// Create a pointer to hold each current node and the next node
		//创建一个指针保持当前节点和下一个节点
		LinkedListNode* pCurrNode,
				      * pNextNode;

		// Set the current node to the head of the list
		//设置当前指针指向首节点
		pCurrNode = pList->pHead;

		// Traverse the list
		//遍历链表
		while (TRUE)
		{
			// Save the pointer to the next node before freeing the current one
			//释放当前节点前保持指向下一个节点的指针
			pNextNode = pCurrNode->pNext;

			// Clear the current node's data
			//清空当前节点数据
			if (pCurrNode->pData)
				free(pCurrNode->pData);

			// Clear the node itself
			//清除节点自身
			if (pCurrNode)
				free(pCurrNode);

			// Move to the next node if it exists; otherwise, exit the loop
			//如果存在下一个节点就移动到下一个节点；否则在退出循环
			if (pNextNode)
				pCurrNode = pNextNode;
			else
				break;
		}
	}
}

/// <summary>
/// Adds a node to a linked list and returns its index.
/// 插入节点
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
/// 删除节点
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
/// 插入字符串节点
/// </summary>
/// <param name="pList"></param>
/// <param name="pstrString"></param>
/// <returns></returns>
int AddString(LinkedList* pList, char* pstrString)
{
	//! First check to see if the string is already in the list 首先检查这个字符串是否已经在链表中存在

	// Create a node to traverse the list
	//创建一个指针指向头部用于遍历链表
	LinkedListNode* pNode = pList->pHead;

	// Loop through each node in the list
	//循环列表
	for (int iCurrNode = 0; iCurrNode < pList->iNodeCount; ++iCurrNode)
	{
		// If the current node's string equals the specified string, return its index

		if (strcmp((char*)pNode->pData, pstrString) == 0)
			return iCurrNode;

		// Otherwise move along to the next node

		pNode = pNode->pNext;
	}

	//! Add the new string, since it wasn't added 添加字符串

	// Create space on the heap for the specified string
	//为这个字符串在堆中申请内存
	char* pstrStringNode = (char*)malloc(strlen(pstrString) + 1);
	strcpy(pstrStringNode, pstrString);

	// Add the string to the list and return its index
	//将其添加到链表，并返回索引
	return AddNode(pList, pstrStringNode);
}

/// <summary>
/// Returns a string from a linked list based on its index.
/// 查找字符串节点。返回索引
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