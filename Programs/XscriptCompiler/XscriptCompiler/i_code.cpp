//I-code module
//中间代码模块



#include "i_code.h"

// ---- Global Variables ----------------------------------------------------------------------

int g_iCurrJumpTargetIndex = 0;                     // The current target index



// ---- Functions -----------------------------------------------------------------------------

/// <summary>
/// Returns an I-code instruction structure based on its implicit index.
/// 获取中间代码节点
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="iInstrIndex"></param>
/// <returns></returns>
ICodeNode* GetICodeNodeByImpIndex(int iFuncIndex, int iInstrIndex)
{
	// Get the function

	FuncNode* pFunc = GetFuncByIndex(iFuncIndex);

	// If the stream is empty, return a NULL pointer

	if (!pFunc->ICodeStream.iNodeCount)
		return NULL;

	// Create a pointer to traverse the list

	LinkedListNode* pCurrNode = pFunc->ICodeStream.pHead;

	// Traverse the list until the matching index is found
	//遍历
	for (int iCurrNode = 0; iCurrNode < pFunc->ICodeStream.iNodeCount; ++iCurrNode)
	{
		// If the implicit index matches, return the instruction
		//如果绝对索引匹配
		if (iInstrIndex == iCurrNode)
			return (ICodeNode*)pCurrNode->pData;

		// Otherwise move to the next node

		pCurrNode = pCurrNode->pNext;
	}

	// The instruction was not found, so return a NULL pointer

	return NULL;
}

/// <summary>
///  Adds a line of source code annotation to the I-code stream of the specified function.
/// 增加远代码标注
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="pstrSourceLine"></param>
void AddICodeSourceLine(int iFuncIndex, char* pstrSourceLine)
{
	// Get the function to which the source line should be added
	//获取函数索引
	FuncNode* pFunc = GetFuncByIndex(iFuncIndex);

	// Create an I-code node structure to hold the line
	//创建中间节点用于保存
	ICodeNode* pSourceLineNode = (ICodeNode*)malloc(sizeof(ICodeNode));

	// Set the node type to source line
	//设置类型为源代码行
	pSourceLineNode->iType = ICODE_NODE_SOURCE_LINE;

	// Set the source line string pointer
	//设置源代码行中的字符串指针
	pSourceLineNode->pstrSourceLine = pstrSourceLine;

	// Add the instruction node to the list and get the index
	//将节点添加到链表
	AddNode(&pFunc->ICodeStream, pSourceLineNode);
}

/// <summary>
/// Adds an instruction to the local I-code stream of the specified function.
/// 添加中间代码的指令
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="iOpcode"></param>
/// <returns></returns>
int AddICodeInstr(int iFuncIndex, int iOpcode)
{
	// Get the function to which the instruction should be added
	//获取要被添加的函数
	FuncNode* pFunc = GetFuncByIndex(iFuncIndex);

	// Create an I-code node structure to hold the instruction
	//创建一个中间代码节点保存这个指令
	ICodeNode* pInstrNode = (ICodeNode*)malloc(sizeof(ICodeNode));

	// Set the node type to instruction
	//设置节点类型为指令
	pInstrNode->iType = ICODE_NODE_INSTR;

	// Set the opcode
	//设置操作码
	pInstrNode->Instr.iOpcode = iOpcode;

	// Clear the operand list
	//设置操作数列表
	pInstrNode->Instr.OpList.iNodeCount = 0;

	// Add the instruction node to the list and get the index
	//将指令添加到链表并获得索引
	int iIndex = AddNode(&pFunc->ICodeStream, pInstrNode);

	// Return the index
	//返回索引
	return iIndex;
}

/// <summary>
/// Returns an I-code instruction's operand at the specified index.
/// 获取中间代码的操作数
/// </summary>
/// <param name="pInstr"></param>
/// <param name="iOpIndex"></param>
/// <returns></returns>
Op* GetICodeOpByIndex(ICodeNode* pInstr, int iOpIndex)
{
	// If the list is empty, return a NULL pointer
	//链表为空，返回null
	if (!pInstr->Instr.OpList.iNodeCount)
		return NULL;

	// Create a pointer to traverse the list
	//创建指针用于遍历
	LinkedListNode* pCurrNode = pInstr->Instr.OpList.pHead;

	// Traverse the list until the matching index is found
	//遍历
	for (int iCurrNode = 0; iCurrNode < pInstr->Instr.OpList.iNodeCount; ++iCurrNode)
	{
		// If the index matches, return the operand
		//索引匹配
		if (iOpIndex == iCurrNode)
			return (Op*)pCurrNode->pData;

		// Otherwise move to the next node

		pCurrNode = pCurrNode->pNext;
	}

	// The operand was not found, so return a NULL pointer
	//没找到
	return NULL;
}

/// <summary>
/// Adds an operand to the specified I-code instruction.
/// 添加操作数
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="iInstrIndex"></param>
/// <param name="Value"></param>
void AddICodeOp(int iFuncIndex, int iInstrIndex, Op Value)
{
	// Get the I-code node
	//获取中间节点
	ICodeNode* pInstr = GetICodeNodeByImpIndex(iFuncIndex, iInstrIndex);

	// Make a physical copy of the operand structure
	//复制操作数结构体
	Op* pValue = (Op*)malloc(sizeof(Op));
	memcpy(pValue, &Value, sizeof(Op));

	// Add the instruction
	//添加操作数到指令
	AddNode(&pInstr->Instr.OpList, pValue);
}

/// <summary>
/// Adds an integer literal operand to the specified I-code instruction.
/// 添加整型操作数到中间代码指令 调用AddICodeOp
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="iInstrIndex"></param>
/// <param name="iValue"></param>
void AddIntICodeOp(int iFuncIndex, int iInstrIndex, int iValue)
{
	// Create an operand structure to hold the new value
	//创建一个操作数结构保存
	Op Value;

	// Set the operand type to integer and store the value
	//设置类型为整型
	Value.iType = OP_TYPE_INT;
	Value.iIntLiteral = iValue;

	// Add the operand to the instruction
	//将操作数添加到指令
	AddICodeOp(iFuncIndex, iInstrIndex, Value);
}

/// <summary>
/// Adds a float literal operand to the specified I-code instruction.
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="iInstrIndex"></param>
/// <param name="fValue"></param>
void AddFloatICodeOp(int iFuncIndex, int iInstrIndex, float fValue)
{
	// Create an operand structure to hold the new value

	Op Value;

	// Set the operand type to float and store the value

	Value.iType = OP_TYPE_FLOAT;
	Value.fFloatLiteral = fValue;

	// Add the operand to the instruction

	AddICodeOp(iFuncIndex, iInstrIndex, Value);
}

/// <summary>
/// Adds a string literal operand to the specified I-code instruction.
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="iInstrIndex"></param>
/// <param name="iStringIndex"></param>
void AddStringICodeOp(int iFuncIndex, int iInstrIndex, int iStringIndex)
{
	// Create an operand structure to hold the new value

	Op Value;

	// Set the operand type to string index and store the index

	Value.iType = OP_TYPE_STRING_INDEX;
	Value.iStringIndex = iStringIndex;

	// Add the operand to the instruction

	AddICodeOp(iFuncIndex, iInstrIndex, Value);
}

/// <summary>
/// Adds a variable operand to the specified I-code instruction.
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="iInstrIndex"></param>
/// <param name="iSymbolIndex"></param>
void AddVarICodeOp(int iFuncIndex, int iInstrIndex, int iSymbolIndex)
{
	// Create an operand structure to hold the new value

	Op Value;

	// Set the operand type to variable and store the symbol index

	Value.iType = OP_TYPE_VAR;
	Value.iSymbolIndex = iSymbolIndex;

	// Add the operand to the instruction

	AddICodeOp(iFuncIndex, iInstrIndex, Value);
}

/// <summary>
/// Adds an array indexed with a literal integer value operand to the specified I-code instruction.
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="iInstrIndex"></param>
/// <param name="iArraySymbolIndex"></param>
/// <param name="iOffset"></param>
void AddArrayIndexAbsICodeOp(int iFuncIndex, int iInstrIndex, int iArraySymbolIndex, int iOffset)
{
	// Create an operand structure to hold the new value

	Op Value;

	// Set the operand type to array index absolute and store the indices

	Value.iType = OP_TYPE_ARRAY_INDEX_ABS;
	Value.iSymbolIndex = iArraySymbolIndex;
	Value.iOffset = iOffset;

	// Add the operand to the instruction

	AddICodeOp(iFuncIndex, iInstrIndex, Value);
}

/******************************************************************************************
*
*   AddArrayIndexVarICodeOp ()
*
*   Adds an array indexed with a variable operand to the specified I-code
*   instruction.
*/

void AddArrayIndexVarICodeOp(int iFuncIndex, int iInstrIndex, int iArraySymbolIndex, int iOffsetSymbolIndex)
{
	// Create an operand structure to hold the new value

	Op Value;

	// Set the operand type to array index variable and store the indices

	Value.iType = OP_TYPE_ARRAY_INDEX_VAR;
	Value.iSymbolIndex = iArraySymbolIndex;
	Value.iOffsetSymbolIndex = iOffsetSymbolIndex;

	// Add the operand to the instruction

	AddICodeOp(iFuncIndex, iInstrIndex, Value);
}

/******************************************************************************************
*
*   AddFuncICodeOp ()
*
*   Adds a function operand to the specified I-code instruction.
*/

void AddFuncICodeOp(int iFuncIndex, int iInstrIndex, int iOpFuncIndex)
{
	// Create an operand structure to hold the new value

	Op Value;

	// Set the operand type to function index and store the index

	Value.iType = OP_TYPE_FUNC_INDEX;
	Value.iFuncIndex = iOpFuncIndex;

	// Add the operand to the instruction

	AddICodeOp(iFuncIndex, iInstrIndex, Value);
}

/******************************************************************************************
*
*   AddRegICodeOp ()
*
*   Adds a register operand to the specified I-code instruction.
*/

void AddRegICodeOp(int iFuncIndex, int iInstrIndex, int iRegCode)
{
	// Create an operand structure to hold the new value

	Op Value;

	// Set the operand type to register and store the code (even though we'll ignore it)

	Value.iType = OP_TYPE_REG;
	Value.iRegCode = iRegCode;

	// Add the operand to the instruction

	AddICodeOp(iFuncIndex, iInstrIndex, Value);
}


/// <summary>
/// Adds a jump target operand to the specified I-code instruction.
/// 添加跳转目标操作数到中间代码指令
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="iInstrIndex"></param>
/// <param name="iTargetIndex"></param>
void AddJumpTargetICodeOp(int iFuncIndex, int iInstrIndex, int iTargetIndex)
{
	// Create an operand structure to hold the new value
	//创建操作数结构
	Op Value;

	// Set the operand type to register and store the code (even though we'll ignore it)
	//设置类型和跳转目标
	Value.iType = OP_TYPE_JUMP_TARGET_INDEX;
	Value.iJumpTargetIndex = iTargetIndex;

	// Add the operand to the instruction

	AddICodeOp(iFuncIndex, iInstrIndex, Value);
}

/// <summary>
/// Returns the next target index.
/// 获取下一个目标索引
/// </summary>
/// <returns></returns>
int GetNextJumpTargetIndex()
{
	// Return and increment the current target index
	//返回当前索引并增加这个值
	return g_iCurrJumpTargetIndex++;
}

/******************************************************************************************
*
*   AddICodeJumpTarget ()
*
*   Adds a jump target to the I-code stream.
*/

void AddICodeJumpTarget(int iFuncIndex, int iTargetIndex)
{
	// Get the function to which the source line should be added

	FuncNode* pFunc = GetFuncByIndex(iFuncIndex);

	// Create an I-code node structure to hold the line

	ICodeNode* pSourceLineNode = (ICodeNode*)malloc(sizeof(ICodeNode));

	// Set the node type to jump target

	pSourceLineNode->iType = ICODE_NODE_JUMP_TARGET;

	// Set the jump target

	pSourceLineNode->iJumpTargetIndex = iTargetIndex;

	// Add the instruction node to the list and get the index

	AddNode(&pFunc->ICodeStream, pSourceLineNode);
}