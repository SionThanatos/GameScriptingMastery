//I-code module
//�м����ģ��



#include "i_code.h"

// ---- Global Variables ----------------------------------------------------------------------

int g_iCurrJumpTargetIndex = 0;                     // The current target index



// ---- Functions -----------------------------------------------------------------------------

/// <summary>
/// Returns an I-code instruction structure based on its implicit index.
/// ��ȡ�м����ڵ�
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
	//����
	for (int iCurrNode = 0; iCurrNode < pFunc->ICodeStream.iNodeCount; ++iCurrNode)
	{
		// If the implicit index matches, return the instruction
		//�����������ƥ��
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
/// ����Զ�����ע
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="pstrSourceLine"></param>
void AddICodeSourceLine(int iFuncIndex, char* pstrSourceLine)
{
	// Get the function to which the source line should be added
	//��ȡ��������
	FuncNode* pFunc = GetFuncByIndex(iFuncIndex);

	// Create an I-code node structure to hold the line
	//�����м�ڵ����ڱ���
	ICodeNode* pSourceLineNode = (ICodeNode*)malloc(sizeof(ICodeNode));

	// Set the node type to source line
	//��������ΪԴ������
	pSourceLineNode->iType = ICODE_NODE_SOURCE_LINE;

	// Set the source line string pointer
	//����Դ�������е��ַ���ָ��
	pSourceLineNode->pstrSourceLine = pstrSourceLine;

	// Add the instruction node to the list and get the index
	//���ڵ���ӵ�����
	AddNode(&pFunc->ICodeStream, pSourceLineNode);
}

/// <summary>
/// Adds an instruction to the local I-code stream of the specified function.
/// ����м�����ָ��
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="iOpcode"></param>
/// <returns></returns>
int AddICodeInstr(int iFuncIndex, int iOpcode)
{
	// Get the function to which the instruction should be added
	//��ȡҪ����ӵĺ���
	FuncNode* pFunc = GetFuncByIndex(iFuncIndex);

	// Create an I-code node structure to hold the instruction
	//����һ���м����ڵ㱣�����ָ��
	ICodeNode* pInstrNode = (ICodeNode*)malloc(sizeof(ICodeNode));

	// Set the node type to instruction
	//���ýڵ�����Ϊָ��
	pInstrNode->iType = ICODE_NODE_INSTR;

	// Set the opcode
	//���ò�����
	pInstrNode->Instr.iOpcode = iOpcode;

	// Clear the operand list
	//���ò������б�
	pInstrNode->Instr.OpList.iNodeCount = 0;

	// Add the instruction node to the list and get the index
	//��ָ����ӵ������������
	int iIndex = AddNode(&pFunc->ICodeStream, pInstrNode);

	// Return the index
	//��������
	return iIndex;
}

/// <summary>
/// Returns an I-code instruction's operand at the specified index.
/// ��ȡ�м����Ĳ�����
/// </summary>
/// <param name="pInstr"></param>
/// <param name="iOpIndex"></param>
/// <returns></returns>
Op* GetICodeOpByIndex(ICodeNode* pInstr, int iOpIndex)
{
	// If the list is empty, return a NULL pointer
	//����Ϊ�գ�����null
	if (!pInstr->Instr.OpList.iNodeCount)
		return NULL;

	// Create a pointer to traverse the list
	//����ָ�����ڱ���
	LinkedListNode* pCurrNode = pInstr->Instr.OpList.pHead;

	// Traverse the list until the matching index is found
	//����
	for (int iCurrNode = 0; iCurrNode < pInstr->Instr.OpList.iNodeCount; ++iCurrNode)
	{
		// If the index matches, return the operand
		//����ƥ��
		if (iOpIndex == iCurrNode)
			return (Op*)pCurrNode->pData;

		// Otherwise move to the next node

		pCurrNode = pCurrNode->pNext;
	}

	// The operand was not found, so return a NULL pointer
	//û�ҵ�
	return NULL;
}

/// <summary>
/// Adds an operand to the specified I-code instruction.
/// ��Ӳ�����
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="iInstrIndex"></param>
/// <param name="Value"></param>
void AddICodeOp(int iFuncIndex, int iInstrIndex, Op Value)
{
	// Get the I-code node
	//��ȡ�м�ڵ�
	ICodeNode* pInstr = GetICodeNodeByImpIndex(iFuncIndex, iInstrIndex);

	// Make a physical copy of the operand structure
	//���Ʋ������ṹ��
	Op* pValue = (Op*)malloc(sizeof(Op));
	memcpy(pValue, &Value, sizeof(Op));

	// Add the instruction
	//��Ӳ�������ָ��
	AddNode(&pInstr->Instr.OpList, pValue);
}

/// <summary>
/// Adds an integer literal operand to the specified I-code instruction.
/// ������Ͳ��������м����ָ�� ����AddICodeOp
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="iInstrIndex"></param>
/// <param name="iValue"></param>
void AddIntICodeOp(int iFuncIndex, int iInstrIndex, int iValue)
{
	// Create an operand structure to hold the new value
	//����һ���������ṹ����
	Op Value;

	// Set the operand type to integer and store the value
	//��������Ϊ����
	Value.iType = OP_TYPE_INT;
	Value.iIntLiteral = iValue;

	// Add the operand to the instruction
	//����������ӵ�ָ��
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
/// �����תĿ����������м����ָ��
/// </summary>
/// <param name="iFuncIndex"></param>
/// <param name="iInstrIndex"></param>
/// <param name="iTargetIndex"></param>
void AddJumpTargetICodeOp(int iFuncIndex, int iInstrIndex, int iTargetIndex)
{
	// Create an operand structure to hold the new value
	//�����������ṹ
	Op Value;

	// Set the operand type to register and store the code (even though we'll ignore it)
	//�������ͺ���תĿ��
	Value.iType = OP_TYPE_JUMP_TARGET_INDEX;
	Value.iJumpTargetIndex = iTargetIndex;

	// Add the operand to the instruction

	AddICodeOp(iFuncIndex, iInstrIndex, Value);
}

/// <summary>
/// Returns the next target index.
/// ��ȡ��һ��Ŀ������
/// </summary>
/// <returns></returns>
int GetNextJumpTargetIndex()
{
	// Return and increment the current target index
	//���ص�ǰ�������������ֵ
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