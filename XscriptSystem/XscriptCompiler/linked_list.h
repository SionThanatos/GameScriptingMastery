/*

	Project.

		XtremeScript Compiler Framework

	Abstract.

		Linked list implementation header

	Date Created.

		9.2.2002

	Author.

		Alex Varanese

*/

#ifndef XSC_LINKED_LIST
#define XSC_LINKED_LIST



#include "globals.h"

// ---- Data Structures -----------------------------------------------------------------------


//链表节点
typedef struct _LinkedListNode                 
{
	void* pData;                               // Pointer to the node's data 指向节点的数据的指针

	_LinkedListNode* pNext;                    // Pointer to the next node in the list 指向下一个节点的指针
}LinkedListNode;

//链表
typedef struct _LinkedList                      
{
	LinkedListNode* pHead,                     // Pointer to head node 指向首节点
				  * pTail;					   // Pointer to tail nail node 指向尾节点

	int iNodeCount;                             // The number of nodes in the list 节点数量
}LinkedList;


// ---- Function Prototypes -------------------------------------------------------------------

void InitLinkedList(LinkedList* pList);
void FreeLinkedList(LinkedList* pList);

int AddNode(LinkedList* pList, void* pData);
void DelNode(LinkedList* pList, LinkedListNode* pNode);

int AddString(LinkedList* pList, char* pstrString);
char* GetStringByIndex(LinkedList* pList, int iIndex);

#endif