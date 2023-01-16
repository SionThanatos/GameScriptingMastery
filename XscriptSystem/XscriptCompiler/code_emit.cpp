//Code emission module
//代码释放模块

// ---- Include Files -------------------------------------------------------------------------

#include "code_emit.h"

// ---- Globals -------------------------------------------------------------------------------

FILE* g_pOutputFile = NULL;                        // Pointer to the output file 输出文件

// ---- Instruction Mnemonics -------------------------------------------------------------

// These mnemonics are mapped to each I-code instruction, allowing the emitter to easily translate I-code to XVM assembly
//助记符
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
/// 生成文件头
/// </summary>
void EmitHeader()
{
	// Get the current time
	//获取时间
	time_t CurrTimeMs;
	struct tm* pCurrTime;
	CurrTimeMs = time(NULL);
	pCurrTime = localtime(&CurrTimeMs);

	// Emit the filename
	//生成文件名
	fprintf(g_pOutputFile, "; %s\n\n", g_pstrOutputFilename);

	// Emit the rest of the header
	//生成其他头部信息
	fprintf(g_pOutputFile, "; Source File: %s\n", g_pstrSourceFilename);
	fprintf(g_pOutputFile, "; XSC Version: %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
	fprintf(g_pOutputFile, ";   Timestamp: %s\n", asctime(pCurrTime));
}

/// <summary>
/// Emits the script's directives.
/// 生成汇编指令
/// </summary>
void EmitDirectives()
{
	// If directives were emitted, this is set to TRUE so we remember to insert extra line breaks after them
	//如果已经生成指令，设置为true
	int iAddNewline = FALSE;

	// If the stack size has been set, emit a SetStackSize directive
	//如果已经设置堆栈大小
	if (g_ScriptHeader.iStackSize)
	{
		fprintf(g_pOutputFile, "\tSetStackSize %d\n", g_ScriptHeader.iStackSize);
		iAddNewline = TRUE;
	}

	// If the priority has been set, emit a SetPriority directive
	//如果设定了优先级生成
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
	//如果有必要，插入换行符
	if (iAddNewline)
		fprintf(g_pOutputFile, "\n");
}

/// <summary>
/// Emits the symbol declarations of the specified scope
/// 生成符号声明
/// </summary>
/// <param name="iScope"></param>
/// <param name="iType"></param>
void EmitScopeSymbols(int iScope, int iType)
{
	// If declarations were emitted, this is set to TRUE so we remember to insert extra line breaks after them
	//已经生成
	int iAddNewline = FALSE;

	// Local symbol node pointer
	//节点指针
	SymbolNode* pCurrSymbol;

	// Loop through each symbol in the table to find the match
	//遍历符号表
	for (int iCurrSymbolIndex = 0; iCurrSymbolIndex < g_SymbolTable.iNodeCount; ++iCurrSymbolIndex)
	{
		// Get the current symbol structure
		//获取符号结构
		pCurrSymbol = GetSymbolByIndex(iCurrSymbolIndex);

		// If the scopes and parameter flags match, emit the declaration
		//匹配
		if (pCurrSymbol->iScope == iScope && pCurrSymbol->iType == iType)
		{
			// Print one tab stop for global declarations, and two for locals
			//为全局声明打印一个tab，局部两个tab
			fprintf(g_pOutputFile, "\t");
			if (iScope != SCOPE_GLOBAL)
				fprintf(g_pOutputFile, "\t");

			// Is the symbol a parameter?
			//符号是否是参数 是则打印param
			if (pCurrSymbol->iType == SYMBOL_TYPE_PARAM)
				fprintf(g_pOutputFile, "Param %s", pCurrSymbol->pstrIdent);

			// Is the symbol a variable?
			//符号是否是变量 是则打印var
			if (pCurrSymbol->iType == SYMBOL_TYPE_VAR)
			{
				fprintf(g_pOutputFile, "Var %s", pCurrSymbol->pstrIdent);

				// If the variable is an array, add the size declaration
				//变量是数组，打印大小
				if (pCurrSymbol->iSize > 1)
					fprintf(g_pOutputFile, " [ %d ]", pCurrSymbol->iSize);
			}

			fprintf(g_pOutputFile, "\n");
			iAddNewline = TRUE;
		}
	}

	// If necessary, insert an extra line break
	//有必要，加上换行符
	if (iAddNewline)
		fprintf(g_pOutputFile, "\n");
}

/// <summary>
/// Emits a function, its local declarations, and its code.
/// 生成函数
/// </summary>
/// <param name="pFunc"></param>
void EmitFunc(FuncNode* pFunc)
{
	// Emit the function declaration name and opening brace
	//生成函数声明和左括号
	fprintf(g_pOutputFile, "\tFunc %s\n", pFunc->pstrName);
	fprintf(g_pOutputFile, "\t{\n");

	// Emit parameter declarations
	//生成参数声明
	EmitScopeSymbols(pFunc->iIndex, SYMBOL_TYPE_PARAM);

	// Emit local variable declarations
	//生成局部变量声明
	EmitScopeSymbols(pFunc->iIndex, SYMBOL_TYPE_VAR);

	// Does the function have an I-code block?
	//这个函数是否有中间代码块
	if (pFunc->ICodeStream.iNodeCount > 0)
	{
		// Used to determine if the current line is the first
		//用来确定是否是第一行
		int iIsFirstSourceLine = TRUE;

		// Yes, so loop through each I-code node to emit the code
		//是，循环
		for (int iCurrInstrIndex = 0; iCurrInstrIndex < pFunc->ICodeStream.iNodeCount; ++iCurrInstrIndex)
		{
			// Get the I-code instruction structure at the current node
			//获取当前节点的中间代码指令
			ICodeNode* pCurrNode = GetICodeNodeByImpIndex(pFunc->iIndex, iCurrInstrIndex);

			// Determine the node type
			//确定节点类型
			switch (pCurrNode->iType)
			{
			// Source code annotation
			//源代码标注
			case ICODE_NODE_SOURCE_LINE:
			{
				// Make a local copy of the source line
				//对行进行复制
				char* pstrSourceLine = pCurrNode->pstrSourceLine;

				// If the last character of the line is a line break, clip it
				//如果最后一个字符是换行符，去掉
				int iLastCharIndex = strlen(pstrSourceLine) - 1;
				if (pstrSourceLine[iLastCharIndex] == '\n')
					pstrSourceLine[iLastCharIndex] = '\0';

				// Emit the comment, but only prepend it with a line break if it's not the first one
				//生成注释，如果不是第一行的话就预先加入一个换行符
				if (!iIsFirstSourceLine)
					fprintf(g_pOutputFile, "\n");

				fprintf(g_pOutputFile, "\t\t; %s\n\n", pstrSourceLine);

				break;
			}

			// An I-code instruction
			//中间代码指令节点
			case ICODE_NODE_INSTR:
			{
				// Emit the opcode
				//生成操作码
				fprintf(g_pOutputFile, "\t\t%s", ppstrMnemonics[pCurrNode->Instr.iOpcode]);

				// Determine the number of operands
				//决定操作数类型
				int iOpCount = pCurrNode->Instr.OpList.iNodeCount;

				// If there are operands to emit, follow the instruction with some space
				//如果有操作数需要生成，在指令的后面加上一些空白
				if (iOpCount)
				{
					// All instructions get at least one tab
					//所有指令至少有一个tab
					fprintf(g_pOutputFile, "\t");

					// If it's less than a tab stop's width in characters, however, they get a second
					//如果字符串太长，就要再加上一个tab
					if (strlen(ppstrMnemonics[pCurrNode->Instr.iOpcode]) < TAB_STOP_WIDTH)
						fprintf(g_pOutputFile, "\t");
				}

				// Emit each operand
				//生成每一个操作数
				for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++iCurrOpIndex)
				{
					// Get a pointer to the operand structure
					//获取指针和操作数结构
					Op* pOp = GetICodeOpByIndex(pCurrNode, iCurrOpIndex);

					// Emit the operand based on its type
					//生成操作数
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
					//使用绝对值索引的数组
					case OP_TYPE_ARRAY_INDEX_ABS:
						fprintf(g_pOutputFile, "%s [ %d ]", GetSymbolByIndex(pOp->iSymbolIndex)->pstrIdent,pOp->iOffset);
						break;

					// Array index variable
					//变量索引的数组
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
			//跳转目标 将跳转索引表转换为一个标号
			case ICODE_NODE_JUMP_TARGET:
			{
				// Emit a label in the format _LX, where X is the jump target
				//生成_LX格式，X为跳转目标
				fprintf(g_pOutputFile, "\t_L%d:\n", pCurrNode->iJumpTargetIndex);
			}
			}

			// Update the first line flag
			//更新第一个行标记
			if (iIsFirstSourceLine)
				iIsFirstSourceLine = FALSE;
		}
	}
	else
	{
		// No, so emit a comment saying so
		//不是，所以生成一个注释说明
		fprintf(g_pOutputFile, "\t\t; (No code)\n");
	}

	// Emit the closing brace
	//生成右括号
	fprintf(g_pOutputFile, "\t}");
}

/// <summary>
/// Translates the I-code representation of the script to an ASCII-foramtted XVM assembly
/// 将中间代码表示的脚本生成为XVM汇编
/// </summary>
void EmitCode()
{
	// ---- Open the output file

	if (!(g_pOutputFile = fopen(g_pstrOutputFilename, "wb")))
		ExitOnError("Could not open output file for output");

	//! Emit the header 生成文件头
	EmitHeader();

	//! Emit directives 生成命令
	fprintf(g_pOutputFile, "; ---- Directives -----------------------------------------------------------------------------\n\n");
	EmitDirectives();

	//! Emit global variable declarations 生成局部变量

	fprintf(g_pOutputFile, "; ---- Global Variables -----------------------------------------------------------------------\n\n");

	// Emit the globals by printing all non-parameter symbols in the global scope
	//生成符号
	EmitScopeSymbols(SCOPE_GLOBAL, SYMBOL_TYPE_VAR);

	//! Emit functions 生成函数

	fprintf(g_pOutputFile, "; ---- Functions ------------------------------------------------------------------------------\n\n");

	// Local node for traversing lists
	//遍历用节点
	LinkedListNode* pNode = g_FuncTable.pHead;

	// Local function node pointer
	//局部函数节点指针
	FuncNode* pCurrFunc;

	// Pointer to hold the _Main () function, if it's found
	//保存Main函数的指针
	FuncNode* pMainFunc = NULL;

	// Loop through each function and emit its declaration and code, if functions exist
	//循环遍历
	if (g_FuncTable.iNodeCount > 0)
	{
		while (TRUE)
		{
			// Get a pointer to the node
			//获取节点指针
			pCurrFunc = (FuncNode*)pNode->pData;

			// Don't emit host API function nodes
			//不能生成host  api函数节点
			if (!pCurrFunc->iIsHostAPI)
			{
				// Is the current function _Main ()?
				//是否是Main
				if (_stricmp(pCurrFunc->pstrName, MAIN_FUNC_NAME) == 0)
				{
					// Yes, so save the pointer for later (and don't emit it yet)
					//是，保存这个指针以备使用
					pMainFunc = pCurrFunc;
				}
				else
				{
					// No, so emit it
					//不是，生成
					EmitFunc(pCurrFunc);
					fprintf(g_pOutputFile, "\n\n");
				}
			}

			// Move to the next node
			//移动到下一个节点
			pNode = pNode->pNext;
			if (!pNode)
				break;
		}
	}

	//! Emit _Main () 生成Main函数

	fprintf(g_pOutputFile, "; ---- Main -----------------------------------------------------------------------------------");

	// If the last pass over the functions found a _Main () function. emit it
	//发现Main函数，生成
	if (pMainFunc)
	{
		fprintf(g_pOutputFile, "\n\n");
		EmitFunc(pMainFunc);
	}

	// ---- Close output file

	fclose(g_pOutputFile);
}
