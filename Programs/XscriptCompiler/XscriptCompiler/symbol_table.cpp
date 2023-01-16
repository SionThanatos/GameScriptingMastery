//Symbol table
//符号表


#include "symbol_table.h"

// ---- Functions -----------------------------------------------------------------------------

/// <summary>
/// 通过索引获取符号
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
/// 通过标识符获取符号结构
/// </summary>
/// <param name="pstrIdent"></param>
/// <param name="iScope"></param>
/// <returns></returns>
SymbolNode* GetSymbolByIdent(char* pstrIdent, int iScope)
{
	// Local symbol node pointer
	//局部符号节点指针
	SymbolNode* pCurrSymbol;

	// Loop through each symbol in the table to find the match
	//循环直到匹配
	for (int iCurrSymbolIndex = 0; iCurrSymbolIndex < g_SymbolTable.iNodeCount; ++iCurrSymbolIndex)
	{
		// Get the current symbol structure
		//获取当前符号结构
		pCurrSymbol = GetSymbolByIndex(iCurrSymbolIndex);

		// Return the symbol if the identifier and scope matches
		//返回
		if (pCurrSymbol && _stricmp(pCurrSymbol->pstrIdent, pstrIdent) == 0 &&
			(pCurrSymbol->iScope == iScope || pCurrSymbol->iScope == 0))
			return pCurrSymbol;
	}

	// The symbol was not found, so return a NULL pointer

	return NULL;
}

/// <summary>
/// Returns a variable's size based on its identifier.
/// 根据标识符返回相应的符号的大小
/// </summary>
/// <param name="pstrIdent"></param>
/// <param name="iScope"></param>
/// <returns></returns>
int GetSizeByIdent(char* pstrIdent, int iScope)
{
	// Get the symbol's information
	//获取符号信息
	SymbolNode* pSymbol = GetSymbolByIdent(pstrIdent, iScope);

	// Return its size

	return pSymbol->iSize;
}

/// <summary>
/// Adds a symbol to the symbol table.
/// 在符号表中添加符号
/// </summary>
/// <param name="pstrIdent"></param>
/// <param name="iSize"></param>
/// <param name="iScope"></param>
/// <param name="iType"></param>
/// <returns></returns>
int AddSymbol(char* pstrIdent, int iSize, int iScope, int iType)
{
	// If a label already exists
	//如果已经存在
	if (GetSymbolByIdent(pstrIdent, iScope))
		return -1;

	// Create a new symbol node
	//创建新的符号节点
	SymbolNode* pNewSymbol = (SymbolNode*)malloc(sizeof(SymbolNode));

	// Initialize the new label
	//初始化标签
	strcpy(pNewSymbol->pstrIdent, pstrIdent);
	pNewSymbol->iSize = iSize;
	pNewSymbol->iScope = iScope;
	pNewSymbol->iType = iType;

	// Add the symbol to the list and get its index
	//添加到链表中并获得索引
	int iIndex = AddNode(&g_SymbolTable, pNewSymbol);

	// Set the symbol node's index
	//设置符号节点的索引
	pNewSymbol->iIndex = iIndex;

	// Return the new symbol's index
	//返回符号的索引
	return iIndex;
}