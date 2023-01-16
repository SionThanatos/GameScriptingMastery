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


//����ڵ�
typedef struct _LinkedListNode                 
{
	void* pData;                               // Pointer to the node's data ָ��ڵ�����ݵ�ָ��

	_LinkedListNode* pNext;                    // Pointer to the next node in the list ָ����һ���ڵ��ָ��
}LinkedListNode;

//����
typedef struct _LinkedList                      
{
	LinkedListNode* pHead,                     // Pointer to head node ָ���׽ڵ�
				  * pTail;					   // Pointer to tail nail node ָ��β�ڵ�

	int iNodeCount;                             // The number of nodes in the list �ڵ�����
}LinkedList;


// ---- Function Prototypes -------------------------------------------------------------------

void InitLinkedList(LinkedList* pList);
void FreeLinkedList(LinkedList* pList);

int AddNode(LinkedList* pList, void* pData);
void DelNode(LinkedList* pList, LinkedListNode* pNode);

int AddString(LinkedList* pList, char* pstrString);
char* GetStringByIndex(LinkedList* pList, int iIndex);

#endif