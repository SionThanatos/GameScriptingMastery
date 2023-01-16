//Code emission module
//�����ͷ�ģ��

// ---- Include Files -------------------------------------------------------------------------

#include "code_emit.h"

// ---- Globals -------------------------------------------------------------------------------

FILE* g_pOutputFile = NULL;                        // Pointer to the output file ����ļ�

// ---- Instruction Mnemonics -------------------------------------------------------------

// These mnemonics are mapped to each I-code instruction, allowing the emitter to easily translate I-code to XVM assembly
//���Ƿ�
char ppstrMnemonics[][12] =
{
	"Mov",
	"Add", "Sub", "Mul", "Div", "Mod", "Exp", "Neg", "Inc", "Dec",
	"And", "Or", "XOr", "Not", "ShL", "ShR",
	"Concat", "GetChar", "SetChar",
	"Jmp", "JE", "JNE", "JG", "JL", "JGE", "JLE",
	"Push", "Pop",
	"Call", "Ret", "CallHost",
	"Pause", "Exit"
};

// ---- Functions -----------------------------------------------------------------------------

/// <summary>
/// Emits the script's header comments.
/// �����ļ�ͷ
/// </summary>
void EmitHeader()
{
	// Get the current time
	//��ȡʱ��
	time_t CurrTimeMs;
	struct tm* pCurrTime;
	CurrTimeMs = time(NULL);
	pCurrTime = localtime(&CurrTimeMs);

	// Emit the filename
	//�����ļ���
	fprintf(g_pOutputFile, "; %s\n\n", g_pstrOutputFilename);

	// Emit the rest of the header
	//��������ͷ����Ϣ
	fprintf(g_pOutputFile, "; Source File: %s\n", g_pstrSourceFilename);
	fprintf(g_pOutputFile, "; XSC Version: %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
	fprintf(g_pOutputFile, ";   Timestamp: %s\n", asctime(pCurrTime));
}

/// <summary>
/// Emits the script's directives.
/// ���ɻ��ָ��
/// </summary>
void EmitDirectives()
{
	// If directives were emitted, this is set to TRUE so we remember to insert extra line breaks after them
	//����Ѿ�����ָ�����Ϊtrue
	int iAddNewline = FALSE;

	// If the stack size has been set, emit a SetStackSize directive
	//����Ѿ����ö�ջ��С
	if (g_ScriptHeader.iStackSize)
	{
		fprintf(g_pOutputFile, "\tSetStackSize %d\n", g_ScriptHeader.iStackSize);
		iAddNewline = TRUE;
	}

	// If the priority has been set, emit a SetPriority directive
	//����趨�����ȼ�����
	if (g_ScriptHeader.iPriorityType != PRIORITY_NONE)
	{
		fprintf(g_pOutputFile, "\tSetPriority ");
		switch (g_ScriptHeader.iPriorityType)
		{
		// Low rank

		case PRIORITY_LOW:
			fprintf(g_pOutputFile, PRIORITY_LOW_KEYWORD);
			break;

		// Medium rank

		case PRIORITY_MED:
			fprintf(g_pOutputFile, PRIORITY_MED_KEYWORD);
			break;

		// High rank

		case PRIORITY_HIGH:
			fprintf(g_pOutputFile, PRIORITY_HIGH_KEYWORD);
			break;

		// User-defined timeslice

		case PRIORITY_USER:
			fprintf(g_pOutputFile, "%d", g_ScriptHeader.iUserPriority);
			break;
		}
		fprintf(g_pOutputFile, "\n");

		iAddNewline = TRUE;
	}

	// If necessary, insert an extra line break
	//����б�Ҫ�����뻻�з�
	if (iAddNewline)
		fprintf(g_pOutputFile, "\n");
}

/// <summary>
/// Emits the symbol declarations of the specified scope
/// ���ɷ�������
/// </summary>
/// <param name="iScope"></param>
/// <param name="iType"></param>
void EmitScopeSymbols(int iScope, int iType)
{
	// If declarations were emitted, this is set to TRUE so we remember to insert extra line breaks after them
	//�Ѿ�����
	int iAddNewline = FALSE;

	// Local symbol node pointer
	//�ڵ�ָ��
	SymbolNode* pCurrSymbol;

	// Loop through each symbol in the table to find the match
	//�������ű�
	for (int iCurrSymbolIndex = 0; iCurrSymbolIndex < g_SymbolTable.iNodeCount; ++iCurrSymbolIndex)
	{
		// Get the current symbol structure
		//��ȡ���Žṹ
		pCurrSymbol = GetSymbolByIndex(iCurrSymbolIndex);

		// If the scopes and parameter flags match, emit the declaration
		//ƥ��
		if (pCurrSymbol->iScope == iScope && pCurrSymbol->iType == iType)
		{
			// Print one tab stop for global declarations, and two for locals
			//Ϊȫ��������ӡһ��tab���ֲ�����tab
			fprintf(g_pOutputFile, "\t");
			if (iScope != SCOPE_GLOBAL)
				fprintf(g_pOutputFile, "\t");

			// Is the symbol a parameter?
			//�����Ƿ��ǲ��� �����ӡparam
			if (pCurrSymbol->iType == SYMBOL_TYPE_PARAM)
				fprintf(g_pOutputFile, "Param %s", pCurrSymbol->pstrIdent);

			// Is the symbol a variable?
			//�����Ƿ��Ǳ��� �����ӡvar
			if (pCurrSymbol->iType == SYMBOL_TYPE_VAR)
			{
				fprintf(g_pOutputFile, "Var %s", pCurrSymbol->pstrIdent);

				// If the variable is an array, add the size declaration
				//���������飬��ӡ��С
				if (pCurrSymbol->iSize > 1)
					fprintf(g_pOutputFile, " [ %d ]", pCurrSymbol->iSize);
			}

			fprintf(g_pOutputFile, "\n");
			iAddNewline = TRUE;
		}
	}

	// If necessary, insert an extra line break
	//�б�Ҫ�����ϻ��з�
	if (iAddNewline)
		fprintf(g_pOutputFile, "\n");
}

/// <summary>
/// Emits a function, its local declarations, and its code.
/// ���ɺ���
/// </summary>
/// <param name="pFunc"></param>
void EmitFunc(FuncNode* pFunc)
{
	// Emit the function declaration name and opening brace
	//���ɺ���������������
	fprintf(g_pOutputFile, "\tFunc %s\n", pFunc->pstrName);
	fprintf(g_pOutputFile, "\t{\n");

	// Emit parameter declarations
	//���ɲ�������
	EmitScopeSymbols(pFunc->iIndex, SYMBOL_TYPE_PARAM);

	// Emit local variable declarations
	//���ɾֲ���������
	EmitScopeSymbols(pFunc->iIndex, SYMBOL_TYPE_VAR);

	// Does the function have an I-code block?
	//��������Ƿ����м�����
	if (pFunc->ICodeStream.iNodeCount > 0)
	{
		// Used to determine if the current line is the first
		//����ȷ���Ƿ��ǵ�һ��
		int iIsFirstSourceLine = TRUE;

		// Yes, so loop through each I-code node to emit the code
		//�ǣ�ѭ��
		for (int iCurrInstrIndex = 0; iCurrInstrIndex < pFunc->ICodeStream.iNodeCount; ++iCurrInstrIndex)
		{
			// Get the I-code instruction structure at the current node
			//��ȡ��ǰ�ڵ���м����ָ��
			ICodeNode* pCurrNode = GetICodeNodeByImpIndex(pFunc->iIndex, iCurrInstrIndex);

			// Determine the node type
			//ȷ���ڵ�����
			switch (pCurrNode->iType)
			{
			// Source code annotation
			//Դ�����ע
			case ICODE_NODE_SOURCE_LINE:
			{
				// Make a local copy of the source line
				//���н��и���
				char* pstrSourceLine = pCurrNode->pstrSourceLine;

				// If the last character of the line is a line break, clip it
				//������һ���ַ��ǻ��з���ȥ��
				int iLastCharIndex = strlen(pstrSourceLine) - 1;
				if (pstrSourceLine[iLastCharIndex] == '\n')
					pstrSourceLine[iLastCharIndex] = '\0';

				// Emit the comment, but only prepend it with a line break if it's not the first one
				//����ע�ͣ�������ǵ�һ�еĻ���Ԥ�ȼ���һ�����з�
				if (!iIsFirstSourceLine)
					fprintf(g_pOutputFile, "\n");

				fprintf(g_pOutputFile, "\t\t; %s\n\n", pstrSourceLine);

				break;
			}

			// An I-code instruction
			//�м����ָ��ڵ�
			case ICODE_NODE_INSTR:
			{
				// Emit the opcode
				//���ɲ�����
				fprintf(g_pOutputFile, "\t\t%s", ppstrMnemonics[pCurrNode->Instr.iOpcode]);

				// Determine the number of operands
				//��������������
				int iOpCount = pCurrNode->Instr.OpList.iNodeCount;

				// If there are operands to emit, follow the instruction with some space
				//����в�������Ҫ���ɣ���ָ��ĺ������һЩ�հ�
				if (iOpCount)
				{
					// All instructions get at least one tab
					//����ָ��������һ��tab
					fprintf(g_pOutputFile, "\t");

					// If it's less than a tab stop's width in characters, however, they get a second
					//����ַ���̫������Ҫ�ټ���һ��tab
					if (strlen(ppstrMnemonics[pCurrNode->Instr.iOpcode]) < TAB_STOP_WIDTH)
						fprintf(g_pOutputFile, "\t");
				}

				// Emit each operand
				//����ÿһ��������
				for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++iCurrOpIndex)
				{
					// Get a pointer to the operand structure
					//��ȡָ��Ͳ������ṹ
					Op* pOp = GetICodeOpByIndex(pCurrNode, iCurrOpIndex);

					// Emit the operand based on its type
					//���ɲ�����
					switch (pOp->iType)
					{
					// Integer literal
					case OP_TYPE_INT:
						fprintf(g_pOutputFile, "%d", pOp->iIntLiteral);
						break;

					// Float literal
					case OP_TYPE_FLOAT:
						fprintf(g_pOutputFile, "%f", pOp->fFloatLiteral);
						break;

					// String literal
					case OP_TYPE_STRING_INDEX:
						fprintf(g_pOutputFile, "\"%s\"", GetStringByIndex(&g_StringTable, pOp->iStringIndex));
						break;

					// Variable
					case OP_TYPE_VAR:
						fprintf(g_pOutputFile, "%s", GetSymbolByIndex(pOp->iSymbolIndex)->pstrIdent);
						break;

					// Array index absolute
					//ʹ�þ���ֵ����������
					case OP_TYPE_ARRAY_INDEX_ABS:
						fprintf(g_pOutputFile, "%s [ %d ]", GetSymbolByIndex(pOp->iSymbolIndex)->pstrIdent,pOp->iOffset);
						break;

					// Array index variable
					//��������������
					case OP_TYPE_ARRAY_INDEX_VAR:
						fprintf(g_pOutputFile, "%s [ %s ]", GetSymbolByIndex(pOp->iSymbolIndex)->pstrIdent,
							GetSymbolByIndex(pOp->iOffsetSymbolIndex)->pstrIdent);
						break;

					// Function
					case OP_TYPE_FUNC_INDEX:
						fprintf(g_pOutputFile, "%s", GetFuncByIndex(pOp->iSymbolIndex)->pstrName);
						break;

					// Register (just _RetVal for now)
					case OP_TYPE_REG:
						fprintf(g_pOutputFile, "_RetVal");
						break;

					// Jump target index
					case OP_TYPE_JUMP_TARGET_INDEX:
						fprintf(g_pOutputFile, "_L%d", pOp->iJumpTargetIndex);
						break;
					}

					// If the operand isn't the last one, append it with a comma and space

					if (iCurrOpIndex != iOpCount - 1)
						fprintf(g_pOutputFile, ", ");
				}

				// Finish the line

				fprintf(g_pOutputFile, "\n");

				break;
			}

			// A jump target
			//��תĿ�� ����ת������ת��Ϊһ�����
			case ICODE_NODE_JUMP_TARGET:
			{
				// Emit a label in the format _LX, where X is the jump target
				//����_LX��ʽ��XΪ��תĿ��
				fprintf(g_pOutputFile, "\t_L%d:\n", pCurrNode->iJumpTargetIndex);
			}
			}

			// Update the first line flag
			//���µ�һ���б��
			if (iIsFirstSourceLine)
				iIsFirstSourceLine = FALSE;
		}
	}
	else
	{
		// No, so emit a comment saying so
		//���ǣ���������һ��ע��˵��
		fprintf(g_pOutputFile, "\t\t; (No code)\n");
	}

	// Emit the closing brace
	//����������
	fprintf(g_pOutputFile, "\t}");
}

/// <summary>
/// Translates the I-code representation of the script to an ASCII-foramtted XVM assembly
/// ���м�����ʾ�Ľű�����ΪXVM���
/// </summary>
void EmitCode()
{
	// ---- Open the output file

	if (!(g_pOutputFile = fopen(g_pstrOutputFilename, "wb")))
		ExitOnError("Could not open output file for output");

	//! Emit the header �����ļ�ͷ
	EmitHeader();

	//! Emit directives ��������
	fprintf(g_pOutputFile, "; ---- Directives -----------------------------------------------------------------------------\n\n");
	EmitDirectives();

	//! Emit global variable declarations ���ɾֲ�����

	fprintf(g_pOutputFile, "; ---- Global Variables -----------------------------------------------------------------------\n\n");

	// Emit the globals by printing all non-parameter symbols in the global scope
	//���ɷ���
	EmitScopeSymbols(SCOPE_GLOBAL, SYMBOL_TYPE_VAR);

	//! Emit functions ���ɺ���

	fprintf(g_pOutputFile, "; ---- Functions ------------------------------------------------------------------------------\n\n");

	// Local node for traversing lists
	//�����ýڵ�
	LinkedListNode* pNode = g_FuncTable.pHead;

	// Local function node pointer
	//�ֲ������ڵ�ָ��
	FuncNode* pCurrFunc;

	// Pointer to hold the _Main () function, if it's found
	//����Main������ָ��
	FuncNode* pMainFunc = NULL;

	// Loop through each function and emit its declaration and code, if functions exist
	//ѭ������
	if (g_FuncTable.iNodeCount > 0)
	{
		while (TRUE)
		{
			// Get a pointer to the node
			//��ȡ�ڵ�ָ��
			pCurrFunc = (FuncNode*)pNode->pData;

			// Don't emit host API function nodes
			//��������host  api�����ڵ�
			if (!pCurrFunc->iIsHostAPI)
			{
				// Is the current function _Main ()?
				//�Ƿ���Main
				if (_stricmp(pCurrFunc->pstrName, MAIN_FUNC_NAME) == 0)
				{
					// Yes, so save the pointer for later (and don't emit it yet)
					//�ǣ��������ָ���Ա�ʹ��
					pMainFunc = pCurrFunc;
				}
				else
				{
					// No, so emit it
					//���ǣ�����
					EmitFunc(pCurrFunc);
					fprintf(g_pOutputFile, "\n\n");
				}
			}

			// Move to the next node
			//�ƶ�����һ���ڵ�
			pNode = pNode->pNext;
			if (!pNode)
				break;
		}
	}

	//! Emit _Main () ����Main����

	fprintf(g_pOutputFile, "; ---- Main -----------------------------------------------------------------------------------");

	// If the last pass over the functions found a _Main () function. emit it
	//����Main����������
	if (pMainFunc)
	{
		fprintf(g_pOutputFile, "\n\n");
		EmitFunc(pMainFunc);
	}

	// ---- Close output file

	fclose(g_pOutputFile);
}
