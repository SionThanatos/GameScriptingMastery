//Symbol table
//���ű�


#include "symbol_table.h"

// ---- Functions -----------------------------------------------------------------------------

/// <summary>
/// ͨ��������ȡ����
/// </summary>
/// <param name="iIndex"></param>
/// <returns></returns>
SymbolNode* GetSymbolByIndex(int iIndex)
{
	// If the table is empty, return a NULL pointer

	if (!g_SymbolTable.iNodeCount)
		return NULL;

	// Create a pointer to traverse the list

	LinkedListNode* pCurrNode = g_SymbolTable.pHead;

	// Traverse the list until the matching structure is found

	for (int iCurrNode = 0; iCurrNode < g_SymbolTable.iNodeCount; ++iCurrNode)
	{
		// Create a pointer to the current symbol structure

		SymbolNode* pCurrSymbol = (SymbolNode*)pCurrNode->pData;

		// If the indices match, return the symbol

		if (iIndex == pCurrSymbol->iIndex)
			return pCurrSymbol;

		// Otherwise move to the next node

		pCurrNode = pCurrNode->pNext;
	}

	// The symbol was not found, so return a NULL pointer

	return NULL;
}

/// <summary>
/// Returns a pointer to the symbol structure corresponding to the identifier and scope.
/// ͨ����ʶ����ȡ���Žṹ
/// </summary>
/// <param name="pstrIdent"></param>
/// <param name="iScope"></param>
/// <returns></returns>
SymbolNode* GetSymbolByIdent(char* pstrIdent, int iScope)
{
	// Local symbol node pointer
	//�ֲ����Žڵ�ָ��
	SymbolNode* pCurrSymbol;

	// Loop through each symbol in the table to find the match
	//ѭ��ֱ��ƥ��
	for (int iCurrSymbolIndex = 0; iCurrSymbolIndex < g_SymbolTable.iNodeCount; ++iCurrSymbolIndex)
	{
		// Get the current symbol structure
		//��ȡ��ǰ���Žṹ
		pCurrSymbol = GetSymbolByIndex(iCurrSymbolIndex);

		// Return the symbol if the identifier and scope matches
		//����
		if (pCurrSymbol && _stricmp(pCurrSymbol->pstrIdent, pstrIdent) == 0 &&
			(pCurrSymbol->iScope == iScope || pCurrSymbol->iScope == 0))
			return pCurrSymbol;
	}

	// The symbol was not found, so return a NULL pointer

	return NULL;
}

/// <summary>
/// Returns a variable's size based on its identifier.
/// ���ݱ�ʶ��������Ӧ�ķ��ŵĴ�С
/// </summary>
/// <param name="pstrIdent"></param>
/// <param name="iScope"></param>
/// <returns></returns>
int GetSizeByIdent(char* pstrIdent, int iScope)
{
	// Get the symbol's information
	//��ȡ������Ϣ
	SymbolNode* pSymbol = GetSymbolByIdent(pstrIdent, iScope);

	// Return its size

	return pSymbol->iSize;
}

/// <summary>
/// Adds a symbol to the symbol table.
/// �ڷ��ű�����ӷ���
/// </summary>
/// <param name="pstrIdent"></param>
/// <param name="iSize"></param>
/// <param name="iScope"></param>
/// <param name="iType"></param>
/// <returns></returns>
int AddSymbol(char* pstrIdent, int iSize, int iScope, int iType)
{
	// If a label already exists
	//����Ѿ�����
	if (GetSymbolByIdent(pstrIdent, iScope))
		return -1;

	// Create a new symbol node
	//�����µķ��Žڵ�
	SymbolNode* pNewSymbol = (SymbolNode*)malloc(sizeof(SymbolNode));

	// Initialize the new label
	//��ʼ����ǩ
	strcpy(pNewSymbol->pstrIdent, pstrIdent);
	pNewSymbol->iSize = iSize;
	pNewSymbol->iScope = iScope;
	pNewSymbol->iType = iType;

	// Add the symbol to the list and get its index
	//��ӵ������в��������
	int iIndex = AddNode(&g_SymbolTable, pNewSymbol);

	// Set the symbol node's index
	//���÷��Žڵ������
	pNewSymbol->iIndex = iIndex;

	// Return the new symbol's index
	//���ط��ŵ�����
	return iIndex;
}