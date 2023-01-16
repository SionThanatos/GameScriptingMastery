//Parser module
//语法分析器模块





#include "parser.h"
#include "error.h"
#include "lexer.h"
#include "symbol_table.h"
#include "func_table.h"
#include "i_code.h"

// ---- Globals -------------------------------------------------------------------------------

// ---- Functions -------------------------------------------------------------------------

int g_iCurrScope;                               // The current scope 当前范围 为0表示全局，非0表示索引

// ---- Loops -----------------------------------------------------------------------------

Stack g_LoopStack;                              // Loop handling stack 处理循环堆栈

// ---- Functions -----------------------------------------------------------------------------

/// <summary>
/// Attempts to read a specific token and prints an error if its not found.
/// GetNextToken封装，读入指定的属性符
/// </summary>
/// <param name="ReqToken">指定的属性符</param>
void ReadToken(Token ReqToken)
{
	// Determine if the next token is the required one
	//判断下一个属性符是否是需要 的
	if (GetNextToken() != ReqToken)
	{
		// If not, exit on a specific error
		//不是，给出错误信息
		char pstrErrorMssg[256];
		switch (ReqToken)
		{
		// Integer
		case TOKEN_TYPE_INT:
			strcpy(pstrErrorMssg, "Integer");
			break;

		// Float
		case TOKEN_TYPE_FLOAT:
			strcpy(pstrErrorMssg, "Float");
			break;

		// Identifier
		case TOKEN_TYPE_IDENT:
			strcpy(pstrErrorMssg, "Identifier");
			break;

		// var
		case TOKEN_TYPE_RSRVD_VAR:
			strcpy(pstrErrorMssg, "var");
			break;

		// true

		case TOKEN_TYPE_RSRVD_TRUE:
			strcpy(pstrErrorMssg, "true");
			break;

			// false

		case TOKEN_TYPE_RSRVD_FALSE:
			strcpy(pstrErrorMssg, "false");
			break;

			// if

		case TOKEN_TYPE_RSRVD_IF:
			strcpy(pstrErrorMssg, "if");
			break;

			// else

		case TOKEN_TYPE_RSRVD_ELSE:
			strcpy(pstrErrorMssg, "else");
			break;

			// break

		case TOKEN_TYPE_RSRVD_BREAK:
			strcpy(pstrErrorMssg, "break");
			break;

			// continue

		case TOKEN_TYPE_RSRVD_CONTINUE:
			strcpy(pstrErrorMssg, "continue");
			break;

			// for

		case TOKEN_TYPE_RSRVD_FOR:
			strcpy(pstrErrorMssg, "for");
			break;

			// while

		case TOKEN_TYPE_RSRVD_WHILE:
			strcpy(pstrErrorMssg, "while");
			break;

			// func

		case TOKEN_TYPE_RSRVD_FUNC:
			strcpy(pstrErrorMssg, "func");
			break;

			// return

		case TOKEN_TYPE_RSRVD_RETURN:
			strcpy(pstrErrorMssg, "return");
			break;

			// host

		case TOKEN_TYPE_RSRVD_HOST:
			strcpy(pstrErrorMssg, "host");
			break;

			// Operator

		case TOKEN_TYPE_OP:
			strcpy(pstrErrorMssg, "Operator");
			break;

			// Comma

		case TOKEN_TYPE_DELIM_COMMA:
			strcpy(pstrErrorMssg, ",");
			break;

			// Open parenthesis

		case TOKEN_TYPE_DELIM_OPEN_PAREN:
			strcpy(pstrErrorMssg, "(");
			break;

			// Close parenthesis

		case TOKEN_TYPE_DELIM_CLOSE_PAREN:
			strcpy(pstrErrorMssg, ")");
			break;

			// Open brace

		case TOKEN_TYPE_DELIM_OPEN_BRACE:
			strcpy(pstrErrorMssg, "[");
			break;

			// Close brace

		case TOKEN_TYPE_DELIM_CLOSE_BRACE:
			strcpy(pstrErrorMssg, "]");
			break;

			// Open curly brace

		case TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE:
			strcpy(pstrErrorMssg, "{");
			break;

			// Close curly brace

		case TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE:
			strcpy(pstrErrorMssg, "}");
			break;

			// Semicolon

		case TOKEN_TYPE_DELIM_SEMICOLON:
			strcpy(pstrErrorMssg, ";");
			break;

			// String

		case TOKEN_TYPE_STRING:
			strcpy(pstrErrorMssg, "String");
			break;
		}

		// Finish the message

		strcat(pstrErrorMssg, " expected");

		// Display the error
		//显示错误
		ExitOnCodeError(pstrErrorMssg);
	}
}

/// <summary>
/// Determines if the specified operator is a relational operator.
/// 是否是关系运算符
/// </summary>
/// <param name="iOpType"></param>
/// <returns></returns>
int IsOpRelational(int iOpType)
{
	if (iOpType != OP_TYPE_EQUAL &&
		iOpType != OP_TYPE_NOT_EQUAL &&
		iOpType != OP_TYPE_LESS &&
		iOpType != OP_TYPE_GREATER &&
		iOpType != OP_TYPE_LESS_EQUAL &&
		iOpType != OP_TYPE_GREATER_EQUAL)
		return FALSE;
	else
		return TRUE;
}

/// <summary>
/// Determines if the specified operator is a logical operator.
/// 是否是逻辑运算符
/// </summary>
/// <param name="iOpType"></param>
/// <returns></returns>
int IsOpLogical(int iOpType)
{
	if (iOpType != OP_TYPE_LOGICAL_AND &&
		iOpType != OP_TYPE_LOGICAL_OR &&
		iOpType != OP_TYPE_LOGICAL_NOT)
		return FALSE;
	else
		return TRUE;
}

/// <summary>
/// Parses the source code from start to finish, generating a complete I-code representation.
/// 分析源代码，生成中间代码表示
/// </summary>
void ParseSourceCode()
{
	// Reset the lexer
	//重置词法分析器
	ResetLexer();

	// Initialize the loop stack
	//初始化循环栈
	InitStack(&g_LoopStack);

	// Set the current scope to global
	//设置当前作用域为全局
	g_iCurrScope = SCOPE_GLOBAL;

	// Parse each line of code
	//分析每一行代码
	while (TRUE)
	{
		// Parse the next statement and ignore an end of file marker
		//分析下一个语句并忽略文件结束标记
		ParseStatement();

		// If we're at the end of the token stream, break the parsing loop
		//如果已经到了属性符流的结尾，跳出
		if (GetNextToken() == TOKEN_TYPE_END_OF_STREAM)
			break;
		else
			RewindTokenStream();
	}

	// Free the loop stack

	FreeStack(&g_LoopStack);
}

/// <summary>
/// Parses a statement.
/// 分析非终结符 Statement(语句)
/// </summary>
void ParseStatement()
{
	// If the next token is a semicolon, the statement is empty so return
	//如果下一个属性符是分号，就是一个空语句
	if (GetLookAheadChar() == ';')
	{
		ReadToken(TOKEN_TYPE_DELIM_SEMICOLON);
		return;
	}

	// Determine the initial token of the statement
	//确定语句的开始属性符
	Token InitToken = GetNextToken();

	// Branch to a parse function based on the token
	//根据属性符分支到不同的分析函数
	switch (InitToken)
	{
	// Unexpected end of file
	//文件意外结束
	case TOKEN_TYPE_END_OF_STREAM:
		ExitOnCodeError("Unexpected end of file");
		break;

	// Block
    //块
	case TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE:
		ParseBlock();
		break;

	// Variable/array declaration
	//变量、数组 声明
	case TOKEN_TYPE_RSRVD_VAR:
		ParseVar();
		break;

	// Host API function import
	//host api函数导入
	case TOKEN_TYPE_RSRVD_HOST:
		ParseHost();
		break;

	// Function definition
	//函数定义
	case TOKEN_TYPE_RSRVD_FUNC:
		ParseFunc();
		break;

	// if block
	//如果是块
	case TOKEN_TYPE_RSRVD_IF:
		ParseIf();
		break;

	// while loop block

	case TOKEN_TYPE_RSRVD_WHILE:
		ParseWhile();
		break;

	// for loop block

	case TOKEN_TYPE_RSRVD_FOR:
		ParseFor();
		break;

	// break

	case TOKEN_TYPE_RSRVD_BREAK:
		ParseBreak();
		break;

	// continue

	case TOKEN_TYPE_RSRVD_CONTINUE:
		ParseContinue();
		break;

	// return
	//返回
	case TOKEN_TYPE_RSRVD_RETURN:
		ParseReturn();
		break;

	// Assignment or Function Call

	case TOKEN_TYPE_IDENT:
	{
		// What kind of identifier is it?
		//是什么类型的标识符
		if (GetSymbolByIdent(GetCurrLexeme(), g_iCurrScope))
		{
			// It's an identifier, so treat the statement as an assignment
			//如果是标识符，看作赋值语句
			ParseAssign();
		}
		else if (GetFuncByName(GetCurrLexeme()))
		{
			// It's a function
			//是函数
			// Annotate the line and parse the call
			//标注行代码并分析函数调用
			AddICodeSourceLine(g_iCurrScope, GetCurrSourceLine());
			ParseFuncCall();

			// Verify the presence of the semicolon
			//检查是否有分号
			ReadToken(TOKEN_TYPE_DELIM_SEMICOLON);
		}
		else
		{
			// It's invalid
			//无效
			ExitOnCodeError("Invalid identifier");
		}

		break;
	}

	// Anything else is invalid

	default:
		ExitOnCodeError("Unexpected input");
		break;
	}
}

/// <summary>
/// Parses a code block.{ <Statement-List> }
/// 分析代码块
/// </summary>
void ParseBlock()
{
	//左边的括号已经在ParseStatement中读取了
	// Make sure we're not in the global scope
	//确保不在全局作用域中
	if (g_iCurrScope == SCOPE_GLOBAL)
		ExitOnCodeError("Code blocks illegal in global scope");

	// Read each statement until the end of the block
	//读入每个语句直到块结束
	while (GetLookAheadChar() != '}')//读到右花括号
		ParseStatement();

	// Read the closing curly brace
	//读入大括号的右边
	ReadToken(TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE);
}


/// <summary>
/// Parses the declaration of a variable or array and adds it to the symbol table.
/// var <Identifier>;
/// var <Identifier> [ <Integer> ];
/// 分析变量和数组的声明并加入到符号表中
/// </summary>
void ParseVar()
{
	// Read an identifier token
	//读取标识符属性符
	ReadToken(TOKEN_TYPE_IDENT);

	// Copy the current lexeme into a local string buffer to save the variable's identifier
	//将当前单词复制到局部字符串缓冲区以保存
	char pstrIdent[MAX_LEXEME_SIZE];
	CopyCurrLexeme(pstrIdent);

	// Set the size to 1 for a variable (an array will update this value)
	//将变量的大小设置为1
	int iSize = 1;

	// Is the look-ahead character an open brace?
	//预览后面的字符是左括号[吗？
	if (GetLookAheadChar() == '[')
	{
		// Verify the open brace
		//校验左括号
		ReadToken(TOKEN_TYPE_DELIM_OPEN_BRACE);

		// If so, read an integer token
		//是，则读取整型属性符
		ReadToken(TOKEN_TYPE_INT);

		// Convert the current lexeme to an integer to get the size
		//将当前单词转换为整型来获得大小
		iSize = atoi(GetCurrLexeme());

		// Read the closing brace
		//读取右括号
		ReadToken(TOKEN_TYPE_DELIM_CLOSE_BRACE);
	}

	// Add the identifier and size to the symbol table
	//将标识符和大小添加到符号表中
	if (AddSymbol(pstrIdent, iSize, g_iCurrScope, SYMBOL_TYPE_VAR) == -1)
		ExitOnCodeError("Identifier redefinition");

	// Read the semicolon
	//读取分号
	ReadToken(TOKEN_TYPE_DELIM_SEMICOLON);
}



/// <summary>
/// Parses the importing of a host API function.
/// host <Identifier> ();
/// 解析host api函数
/// </summary>
void ParseHost()
{
	// Read the host API function name
	//读取host api函数名称
	ReadToken(TOKEN_TYPE_IDENT);

	// Add the function to the function table with the host API flag set
	//将函数添加到函数表并且设置host api标记
	if (AddFunc(GetCurrLexeme(), TRUE) == -1)
		ExitOnCodeError("Function redefinition");

	// Make sure the function name is followed with ()
	//确保函数名后面跟着（）
	ReadToken(TOKEN_TYPE_DELIM_OPEN_PAREN);
	ReadToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);

	// Read the semicolon
	//读取分号
	ReadToken(TOKEN_TYPE_DELIM_SEMICOLON);
}

/// <summary>
/// Parses a function.
/// 分析函数
/// func <Identifier> ( <Parameter-List> ) <Statement>
/// </summary>
void ParseFunc()
{
	// Make sure we're not already in a function
	//确保现在不在一个函数中，函数内部不能定义函数
	if (g_iCurrScope != SCOPE_GLOBAL)
		ExitOnCodeError("Nested functions illegal");

	// Read the function name
	//读取函数名称(表示属性符)
	ReadToken(TOKEN_TYPE_IDENT);

	// Add the non-host API function to the function table and get its index
	//将非host应用api函数加入到函数表中并获得其索引
	int iFuncIndex = AddFunc(GetCurrLexeme(), FALSE);

	// Check for a function redefinition
	//检查函数的重复定义
	if (iFuncIndex == -1)
		ExitOnCodeError("Function redefinition");

	// Set the scope to the function
	//设置函数的作用域
	g_iCurrScope = iFuncIndex;

	// Read the opening parenthesis
	//读取左括号
	ReadToken(TOKEN_TYPE_DELIM_OPEN_PAREN);

	// Use the look-ahead character to determine if the function takes parameters
	//使用预览查看字符串确定函数是否有参数
	if (GetLookAheadChar() != ')')
	{
		// If the function being defined is _Main (), flag an error since _Main () cannot accept paraemters
		//如果函数是Main，给一个错误标记，因为Main函数不接受参数
		if (g_ScriptHeader.iIsMainFuncPresent &&
			g_ScriptHeader.iMainFuncIndex == iFuncIndex)
		{
			ExitOnCodeError("_Main () cannot accept parameters");
		}

		// Start the parameter count at zero
		//参数计数从0开始
		int iParamCount = 0;

		// Crete an array to store the parameter list locally
		//创建一个数组保存本地参数列表
		char ppstrParamList[MAX_FUNC_DECLARE_PARAM_COUNT][MAX_IDENT_SIZE];

		// Read the parameters
		//读取参数
		while (TRUE)
		{
			// Read the identifier
			//读取标识符
			ReadToken(TOKEN_TYPE_IDENT);

			// Copy the current lexeme to the parameter list array
			//将当前的词素复制到参数列表
			CopyCurrLexeme(ppstrParamList[iParamCount]);

			// Increment the parameter count
			//增加参数计数
			++iParamCount;

			// Check again for the closing parenthesis to see if the parameter list is done
			//检查右括号以确定参数列表是否结束
			if (GetLookAheadChar() == ')')
				break;

			// Otherwise read a comma and move to the next parameter
			//否则读取下一个逗号，处理下一个参数
			ReadToken(TOKEN_TYPE_DELIM_COMMA);
		}

		// Set the final parameter count
		//设置最终的参数个数
		SetFuncParamCount(g_iCurrScope, iParamCount);

		// Write the parameters to the function's symbol table in reverse order, so they'll be emitted from right-to-left
		//将参数逆序写入函数符号表中，这样顺序会从右到左生成
		while (iParamCount > 0)
		{
			--iParamCount;

			// Add the parameter to the symbol table
			//将参数写入符号表
			AddSymbol(ppstrParamList[iParamCount], 1, g_iCurrScope, SYMBOL_TYPE_PARAM);
		}
	}

	// Read the closing parenthesis
	//读入右括号
	ReadToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);

	//! 分析函数体

	// Read the opening curly brace
	//读取左大括号
	ReadToken(TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE);

	// Parse the function's body
	//分析函数体
	ParseBlock();

	// Return to the global scope
	//返回到全局范围
	g_iCurrScope = SCOPE_GLOBAL;
}

/// <summary>
/// Parses an expression.
/// 解析表达式
/// </summary>
void ParseExpr()
{
	int iInstrIndex;

	// The current operator type
	//当前运算符类型
	int iOpType;

	// Parse the subexpression
	//分析子表达式
	ParseSubExpr();

	// Parse any subsequent relational or logical operators
	//分析后续的+或者-运算符
	while (TRUE)
	{
		// Get the next token
		//获取下一个属性符
		if (GetNextToken() != TOKEN_TYPE_OP ||
			(!IsOpRelational(GetCurrOp()) &&
				!IsOpLogical(GetCurrOp())))
		{
			RewindTokenStream();
			break;
		}

		// Save the operator
		//保存运算符
		iOpType = GetCurrOp();

		// Parse the second term
		//解析第二项
		ParseSubExpr();

		// Pop the first operand into _T1
		//将第一个操作数弹到_T1
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex);

		// Pop the second operand into _T0
		//将第二个操作数弹到_T0
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

		//! Perform the binary operation associated with the specified operator 根据特定的运算符进行相应的运算

		// Determine the operator type
		//判断运算符类型
		if (IsOpRelational(iOpType))
		{
			// Get a pair of free jump target indices
			//获取两个跳转目标
			int iTrueJumpTargetIndex = GetNextJumpTargetIndex(),
				iExitJumpTargetIndex = GetNextJumpTargetIndex();

			// It's a relational operator
			//关系运算符
			switch (iOpType)
			{
			// Equal

			case OP_TYPE_EQUAL:
			{
				// Generate a JE instruction

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JE);
				break;
			}

			// Not Equal

			case OP_TYPE_NOT_EQUAL:
			{
				// Generate a JNE instruction

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JNE);
				break;
			}

			// Greater

			case OP_TYPE_GREATER:
			{
				// Generate a JG instruction

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JG);
				break;
			}

			// Less

			case OP_TYPE_LESS:
			{
				// Generate a JL instruction

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JL);
				break;
			}

			// Greater or Equal

			case OP_TYPE_GREATER_EQUAL:
			{
				// Generate a JGE instruction

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JGE);
				break;
			}

			// Less Than or Equal

			case OP_TYPE_LESS_EQUAL:
			{
				// Generate a JLE instruction

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JLE);
				break;
			}
			}

			//		PUSH X
			//		PUSH Y
			//		POP _T1
			//		POP _T0
			//!     JCC或者别的 _T0,_T1 True
			//		PUSH 0
			//		JMP Exit
			//True:
			//		PUSH 1
			//Exit:


			// Add the jump instruction's operands (_T0 and _T1)
			//将操作数弹出到T0 T1
			AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
			AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex);
			AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iTrueJumpTargetIndex);

			// Generate the outcome for falsehood
			//生成为假的代码
			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
			AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);

			// Generate a jump past the true outcome
			//生成跳过为真部分代码的指令
			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JMP);
			AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iExitJumpTargetIndex);

			// Set the jump target for the true outcome
			//设置为真的跳转目标
			AddICodeJumpTarget(g_iCurrScope, iTrueJumpTargetIndex);

			// Generate the outcome for truth
			//生成为真部分的代码
			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
			AddIntICodeOp(g_iCurrScope, iInstrIndex, 1);

			// Set the jump target for exiting the operand evaluation
			//设置跳出操作数求值的跳转目标
			AddICodeJumpTarget(g_iCurrScope, iExitJumpTargetIndex);
		}
		else
		{
			// It must be a logical operator
			//逻辑运算符
			switch (iOpType)
			{
			// And 
			//! 逻辑与 将与运算符转为汇编的代码如下
			case OP_TYPE_LOGICAL_AND:
			{
				//		JE _T0, 0, False
				//		JE _T1, 0, False
				//		Push 1
				//		Jmp Exit
				// False:
				//		Push 0
				// Exit
				//只要有一个操作数为0，结果为False
				 
				// Get a pair of free jump target indices
				//获得一对未使用的跳转目标索引
				int iFalseJumpTargetIndex = GetNextJumpTargetIndex(),
					iExitJumpTargetIndex = GetNextJumpTargetIndex();

				// JE _T0, 0, False

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JE);//JE
				AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);//T0
				AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);//0
				AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iFalseJumpTargetIndex);//False

				// JE _T1, 0, False

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JE);
				AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex);
				AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);
				AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iFalseJumpTargetIndex);

				// Push 1

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
				AddIntICodeOp(g_iCurrScope, iInstrIndex, 1);

				// Jmp Exit

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JMP);
				AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iExitJumpTargetIndex);

				// L0: (False)

				AddICodeJumpTarget(g_iCurrScope, iFalseJumpTargetIndex);

				// Push 0

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
				AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);

				// L1: (Exit)

				AddICodeJumpTarget(g_iCurrScope, iExitJumpTargetIndex);

				break;
			}

			// Or
			//逻辑或
			case OP_TYPE_LOGICAL_OR:
			{
				// Get a pair of free jump target indices

				int iTrueJumpTargetIndex = GetNextJumpTargetIndex(),
					iExitJumpTargetIndex = GetNextJumpTargetIndex();

				// JNE _T0, 0, True

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JNE);
				AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
				AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);
				AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iTrueJumpTargetIndex);

				// JNE _T1, 0, True

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JNE);
				AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex);
				AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);
				AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iTrueJumpTargetIndex);

				// Push 0

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
				AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);

				// Jmp Exit

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JMP);
				AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iExitJumpTargetIndex);

				// L0: (True)

				AddICodeJumpTarget(g_iCurrScope, iTrueJumpTargetIndex);

				// Push 1

				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
				AddIntICodeOp(g_iCurrScope, iInstrIndex, 1);

				// L1: (Exit)

				AddICodeJumpTarget(g_iCurrScope, iExitJumpTargetIndex);

				break;
			}
			}
		}
	}
}

/// <summary>
/// Parses a sub expression.
/// 解析子表达式
/// </summary>
void ParseSubExpr()
{
	int iInstrIndex;

	// The current operator type
	//当前运算符
	int iOpType;

	// Parse the first term
	//分析第一项
	ParseTerm();

	// Parse any subsequent +, - or $ operators
	//分析后面的+-或$运算符
	while (TRUE)
	{
		// Get the next token
		//获取下一个属性符
		if (GetNextToken() != TOKEN_TYPE_OP ||
			(GetCurrOp() != OP_TYPE_ADD &&
				GetCurrOp() != OP_TYPE_SUB &&
				GetCurrOp() != OP_TYPE_CONCAT))
		{
			RewindTokenStream();
			break;
		}

		// Save the operator
		//保存运算符
		iOpType = GetCurrOp();

		// Parse the second term
		//解析第二项
		ParseTerm();

		// Pop the first operand into _T1
		//将第一个操作数弹到_T1
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex);

		// Pop the second operand into _T0
		//将第二个操作数弹到_T0
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

		// Perform the binary operation associated with the specified operator

		int iOpInstr;
		switch (iOpType)
		{
			// Binary addition

		case OP_TYPE_ADD:
			iOpInstr = INSTR_ADD;
			break;

			// Binary subtraction

		case OP_TYPE_SUB:
			iOpInstr = INSTR_SUB;
			break;

			// Binary string concatenation

		case OP_TYPE_CONCAT:
			iOpInstr = INSTR_CONCAT;
			break;
		}
		iInstrIndex = AddICodeInstr(g_iCurrScope, iOpInstr);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex);

		// Push the result (stored in _T0)

		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
	}
}

/// <summary>
/// Parses a term.
/// 处理运算符中间的操作数
/// </summary>
void ParseTerm()
{
	int iInstrIndex;

	// The current operator type
	//当前运算符类型
	int iOpType;

	// Parse the first factor
	//分析第一个因子
	ParseFactor();

	// Parse any subsequent *, /, %, ^, &, |, #, << and >> operators
	//分析后续的*, /, %, ^, &, |, #, << and >> 运算符
	while (TRUE)
	{
		// Get the next token
		//获取下一个属性符
		if (GetNextToken() != TOKEN_TYPE_OP ||
			(GetCurrOp() != OP_TYPE_MUL &&
				GetCurrOp() != OP_TYPE_DIV &&
				GetCurrOp() != OP_TYPE_MOD &&
				GetCurrOp() != OP_TYPE_EXP &&
				GetCurrOp() != OP_TYPE_BITWISE_AND &&
				GetCurrOp() != OP_TYPE_BITWISE_OR &&
				GetCurrOp() != OP_TYPE_BITWISE_XOR &&
				GetCurrOp() != OP_TYPE_BITWISE_SHIFT_LEFT &&
				GetCurrOp() != OP_TYPE_BITWISE_SHIFT_RIGHT))
		{
			RewindTokenStream();
			break;
		}

		// Save the operator
		//保存运算符
		iOpType = GetCurrOp();

		// Parse the second factor
		//分析第二个因子
		ParseFactor();

		// Pop the first operand into _T1
		//将第一个操作数弹到_T1
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex);

		// Pop the second operand into _T0
		//将第二个操作数弹到_T2
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

		// Perform the binary operation associated with the specified operator
		//根据运算符完成操作
		int iOpInstr;
		switch (iOpType)
		{
		// Binary multiplication
		//乘法
		case OP_TYPE_MUL:
			iOpInstr = INSTR_MUL;
			break;

		// Binary division
		//除法
		case OP_TYPE_DIV:
			iOpInstr = INSTR_DIV;
			break;

		// Binary modulus
		//模
		case OP_TYPE_MOD:
			iOpInstr = INSTR_MOD;
			break;

		// Binary exponentiation

		case OP_TYPE_EXP:
			iOpInstr = INSTR_EXP;
			break;

			// Binary bitwise AND

		case OP_TYPE_BITWISE_AND:
			iOpInstr = INSTR_AND;
			break;

			// Binary bitwise OR

		case OP_TYPE_BITWISE_OR:
			iOpInstr = INSTR_OR;
			break;

			// Binary bitwise XOR

		case OP_TYPE_BITWISE_XOR:
			iOpInstr = INSTR_XOR;
			break;

			// Binary bitwise shift left

		case OP_TYPE_BITWISE_SHIFT_LEFT:
			iOpInstr = INSTR_SHL;
			break;

			// Binary bitwise shift left

		case OP_TYPE_BITWISE_SHIFT_RIGHT:
			iOpInstr = INSTR_SHR;
			break;
		}
		iInstrIndex = AddICodeInstr(g_iCurrScope, iOpInstr);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex);

		// Push the result (stored in _T0)
		//结果入栈 保存在_T0中
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
	}
}

/// <summary>
/// Parses a factor.
/// </summary>
void ParseFactor()
{
	int iInstrIndex;
	int iUnaryOpPending = FALSE;
	int iOpType;

	// First check for a unary operator
	//首先检查单目运算符
	if (GetNextToken() == TOKEN_TYPE_OP &&
		(GetCurrOp() == OP_TYPE_ADD ||
			GetCurrOp() == OP_TYPE_SUB ||
			GetCurrOp() == OP_TYPE_BITWISE_NOT ||
			GetCurrOp() == OP_TYPE_LOGICAL_NOT))
	{
		// If it was found, save it and set the unary operator flag
		//找到保存并标记单目运算标记
		iUnaryOpPending = TRUE;
		iOpType = GetCurrOp();
	}
	else
	{
		// Otherwise rewind the token stream
		//否则回退
		RewindTokenStream();
	}

	// Determine which type of factor we're dealing with based on the next token
	//根据下一个属性符确定我们正在处理哪种因子
	switch (GetNextToken())
	{
	// It's a true or false constant, so push either 0 and 1 onto the stack
	//是true或false常量，把0或1压栈
	case TOKEN_TYPE_RSRVD_TRUE:
	case TOKEN_TYPE_RSRVD_FALSE:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
		AddIntICodeOp(g_iCurrScope, iInstrIndex, GetCurrToken() == TOKEN_TYPE_RSRVD_TRUE ? 1 : 0);
		break;

	// It's an integer literal, so push it onto the stack
	//整型，压栈
	case TOKEN_TYPE_INT:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
		AddIntICodeOp(g_iCurrScope, iInstrIndex, atoi(GetCurrLexeme()));
		break;

	// It's a float literal, so push it onto the stack
	//浮点，压栈
	case TOKEN_TYPE_FLOAT:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
		AddFloatICodeOp(g_iCurrScope, iInstrIndex, (float)atof(GetCurrLexeme()));
		break;

	// It's a string literal, so add it to the string table and push the resulting string index onto the stack
	//字符串 加入到字符串表并且把字符串索引压栈
	case TOKEN_TYPE_STRING:
	{
		int iStringIndex = AddString(&g_StringTable, GetCurrLexeme());
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
		AddStringICodeOp(g_iCurrScope, iInstrIndex, iStringIndex);
		break;
	}

	// It's an identifier
	//标识符
	case TOKEN_TYPE_IDENT:
	{
	// First find out if the identifier is a variable or array
	//判断是数组还是变量
		SymbolNode* pSymbol = GetSymbolByIdent(GetCurrLexeme(), g_iCurrScope);
		if (pSymbol)
		{
			// Does an array index follow the identifier?
			//检查左括号
			if (GetLookAheadChar() == '[')
			{
				// Ensure the variable is an array
				//确保是数组
				if (pSymbol->iSize == 1)
					ExitOnCodeError("Invalid array");

				// Verify the opening brace
				//检查左括号
				ReadToken(TOKEN_TYPE_DELIM_OPEN_BRACE);

				// Make sure an expression is present
				//确保表达式存在
				if (GetLookAheadChar() == ']')
					ExitOnCodeError("Invalid expression");

				// Parse the index as an expression recursively
				//递归分析索引表达式
				ParseExpr();

				// Make sure the index is closed
				//确保索引右括号存在
				ReadToken(TOKEN_TYPE_DELIM_CLOSE_BRACE);

				// Pop the resulting value into _T0 and use it as the index variable
				//将结果弹到_T0并将其作为索引变量
				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
				AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

				// Push the original identifier onto the stack as an array, indexed with _T0
				//将原来的标识符压栈，以_T0作为索引
				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
				AddArrayIndexVarICodeOp(g_iCurrScope, iInstrIndex, pSymbol->iIndex, g_iTempVar0SymbolIndex);
			}
			else
			{
				// If not, make sure the identifier is not an array, and push it onto the stack
				//如果不是，确保这个标识符不是一个数组，压栈
				if (pSymbol->iSize == 1)
				{
					iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
					AddVarICodeOp(g_iCurrScope, iInstrIndex, pSymbol->iIndex);
				}
				else
				{
					ExitOnCodeError("Arrays must be indexed");
				}
			}
		}
		else
		{
			// The identifier wasn't a variable or array, so find out if it's a function
			//标识符不是变量或数组，看看是不是函数
			if (GetFuncByName(GetCurrLexeme()))
			{
				// It is, so parse the call
				//是函数，解析函数
				ParseFuncCall();

				// Push the return value
				//压入返回值
				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
				AddRegICodeOp(g_iCurrScope, iInstrIndex, REG_CODE_RETVAL);
			}
		}

		break;
	}

	// It's a nested expression, so call ParseExpr () recursively and validate the presence of the closing parenthesis
	//是一个嵌套的表达式，所以递归调用ParseExpr ()并检查右括号
	case TOKEN_TYPE_DELIM_OPEN_PAREN:
		ParseExpr();
		ReadToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);
		break;

		// Anything else is invalid

	default:
		ExitOnCodeError("Invalid input");
	}

	// Is a unary operator pending?
	//还有没有处理的单目运算符？
	if (iUnaryOpPending)
	{
		// If so, pop the result of the factor off the top of the stack
		//如有，从堆栈中弹出因子
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

		// Perform the unary operation
		//执行单目运算操作
		if (iOpType == OP_TYPE_LOGICAL_NOT)
		{
			// Get a pair of free jump target indices
			//
			int iTrueJumpTargetIndex = GetNextJumpTargetIndex(),
				iExitJumpTargetIndex = GetNextJumpTargetIndex();

			// JE _T0, 0, True

			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JE);
			AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
			AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);
			AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iTrueJumpTargetIndex);

			// Push 0

			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
			AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);

			// Jmp L1

			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JMP);
			AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iExitJumpTargetIndex);

			// L0: (True)

			AddICodeJumpTarget(g_iCurrScope, iTrueJumpTargetIndex);

			// Push 1

			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
			AddIntICodeOp(g_iCurrScope, iInstrIndex, 1);

			// L1: (Exit)

			AddICodeJumpTarget(g_iCurrScope, iExitJumpTargetIndex);
		}
		else
		{
			int iOpIndex;
			switch (iOpType)
			{
			// Negation
			//取反
			case OP_TYPE_SUB:
				iOpIndex = INSTR_NEG;
				break;

				// Bitwise not

			case OP_TYPE_BITWISE_NOT:
				iOpIndex = INSTR_NOT;
				break;
			}

			// Add the instruction's operand
			//添加指令的操作数
			iInstrIndex = AddICodeInstr(g_iCurrScope, iOpIndex);
			AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

			// Push the result onto the stack
			//将结果压栈
			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
			AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
		}
	}
}




/// <summary>
/// Parses an if block.
/// if ( <Expression> ) <Statement>
/// if ( <Expression> ) <Statement> else <Statement>
/// 分析if 块
/// </summary>
void ParseIf()
{
	int iInstrIndex;

	// Make sure we're inside a function

	if (g_iCurrScope == SCOPE_GLOBAL)
		ExitOnCodeError("if illegal in global scope");

	// Annotate the line

	AddICodeSourceLine(g_iCurrScope, GetCurrSourceLine());

	// Create a jump target to mark the beginning of the false block

	int iFalseJumpTargetIndex = GetNextJumpTargetIndex();

	// Read the opening parenthesis

	ReadToken(TOKEN_TYPE_DELIM_OPEN_PAREN);

	// Parse the expression and leave the result on the stack

	ParseExpr();

	// Read the closing parenthesis

	ReadToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);

	// Pop the result into _T0 and compare it to zero
	//将结果弹出到_T0并与0比较
	iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
	AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

	// If the result is zero, jump to the false target
	//结果为0 跳转到false目标
	iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JE);
	AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
	AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);
	AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iFalseJumpTargetIndex);

	// Parse the true block
	//解析true 块
	ParseStatement();

	// Look for an else clause
	//查找else子句
	if (GetNextToken() == TOKEN_TYPE_RSRVD_ELSE)
	{
		// If it's found, append the true block with an unconditional jump past the false block
		//如果找到就在true后面加上无条件跳转以跳过false块
		int iSkipFalseJumpTargetIndex = GetNextJumpTargetIndex();
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JMP);
		AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iSkipFalseJumpTargetIndex);

		// Place the false target just before the false block
		//将false目标放在false块前
		AddICodeJumpTarget(g_iCurrScope, iFalseJumpTargetIndex);

		// Parse the false block
		//解析false块
		ParseStatement();

		// Set a jump target beyond the false block
		//设置false块后面的跳转目标
		AddICodeJumpTarget(g_iCurrScope, iSkipFalseJumpTargetIndex);
	}
	else
	{
		// Otherwise, put the token back
		//否则回退属性符
		RewindTokenStream();

		// Place the false target after the true block
		//将false块放在true块代码的后面
		AddICodeJumpTarget(g_iCurrScope, iFalseJumpTargetIndex);
	}
}



/// <summary>
/// Parses a while loop block.
/// while ( <Expression> ) <Statement>
/// 分析while
/// </summary>
void ParseWhile()
{
	int iInstrIndex;

	// Make sure we're inside a function

	if (g_iCurrScope == SCOPE_GLOBAL)
		ExitOnCodeError("Statement illegal in global scope");

	// Annotate the line
	//标记行
	AddICodeSourceLine(g_iCurrScope, GetCurrSourceLine());

	// Get two jump targets; for the top and bottom of the loop
	//获取两个跳转目标，分别用于顶部循环和底部
	int iStartTargetIndex = GetNextJumpTargetIndex(),
		iEndTargetIndex = GetNextJumpTargetIndex();

	// Set a jump target at the top of the loop
	//在循环的最后设置一个跳转目标
	AddICodeJumpTarget(g_iCurrScope, iStartTargetIndex);

	// Read the opening parenthesis
	//读取右括号
	ReadToken(TOKEN_TYPE_DELIM_OPEN_PAREN);

	// Parse the expression and leave the result on the stack
	//分析表达式并将结果入栈
	ParseExpr();

	// Read the closing parenthesis
	//读取右括号
	ReadToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);

	// Pop the result into _T0 and jump out of the loop if it's nonzero
	//将结果弹出到_T0 并且如果非0 就跳出循环
	iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
	AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

	iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JE);
	AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
	AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);
	AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iEndTargetIndex);

	// Create a new loop instance structure
	//创建一个新的循环实例
	Loop* pLoop = (Loop*)malloc(sizeof(Loop));

	// Set the starting and ending jump target indices
	//设置开始和结束跳转索引目标
	pLoop->iStartTargetIndex = iStartTargetIndex;
	pLoop->iEndTargetIndex = iEndTargetIndex;

	// Push the loop structure onto the stack
	//将循环结构压入堆栈
	Push(&g_LoopStack, pLoop);

	// Parse the loop body
	//分析循环体
	ParseStatement();

	// Pop the loop instance off the stack
	//弹出循环体实例
	Pop(&g_LoopStack);

	// Unconditionally jump back to the start of the loop

	iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JMP);
	AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iStartTargetIndex);

	// Set a jump target for the end of the loop

	AddICodeJumpTarget(g_iCurrScope, iEndTargetIndex);
}



/// <summary>
/// Parses a for loop block.
/// for ( <Initializer>; <Condition>; <Perpetuator> ) <Statement>
/// </summary>
void ParseFor()
{
	if (g_iCurrScope == SCOPE_GLOBAL)
		ExitOnCodeError("for illegal in global scope");

	// Annotate the line

	AddICodeSourceLine(g_iCurrScope, GetCurrSourceLine());

	/*
		A for loop parser implementation could go here
	*/
}

/// <summary>
/// Parses a break statement.
/// </summary>
void ParseBreak()
{
	// Make sure we're in a loop

	if (IsStackEmpty(&g_LoopStack))
		ExitOnCodeError("break illegal outside loops");

	// Annotate the line

	AddICodeSourceLine(g_iCurrScope, GetCurrSourceLine());

	// Attempt to read the semicolon

	ReadToken(TOKEN_TYPE_DELIM_SEMICOLON);

	// Get the jump target index for the end of the loop
	//获得循环结束的跳转目标索引
	int iTargetIndex = ((Loop*)Peek(&g_LoopStack))->iEndTargetIndex;

	// Unconditionally jump to the end of the loop
	//无条件跳转到循环的结束位置
	int iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JMP);
	AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iTargetIndex);
}

/// <summary>
/// Parses a continue statement.
/// </summary>
void ParseContinue()
{
	// Make sure we're inside a function

	if (IsStackEmpty(&g_LoopStack))
		ExitOnCodeError("continue illegal outside loops");

	// Annotate the line

	AddICodeSourceLine(g_iCurrScope, GetCurrSourceLine());

	// Attempt to read the semicolon

	ReadToken(TOKEN_TYPE_DELIM_SEMICOLON);

	// Get the jump target index for the start of the loop
	//获取循环开始位置跳转目标索引
	int iTargetIndex = ((Loop*)Peek(&g_LoopStack))->iStartTargetIndex;

	// Unconditionally jump to the end of the loop
	//无条件跳转
	int iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JMP);
	AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iTargetIndex);
}




/// <summary>
/// Parses a return statement.
/// 解析返回
/// return <expr>;
/// return;
/// </summary>
void ParseReturn()
{
	int iInstrIndex;

	// Make sure we're inside a function
	//确保在函数中
	if (g_iCurrScope == SCOPE_GLOBAL)
		ExitOnCodeError("return illegal in global scope");

	// Annotate the line
	//标注行
	AddICodeSourceLine(g_iCurrScope, GetCurrSourceLine());

	// If a semicolon doesn't appear to follow, parse the expression and place it in _RetVal
	//如果后面没有分号，分析表达式并将结果放入_RetVal
	if (GetLookAheadChar() != ';')
	{
		// Parse the expression to calculate the return value and leave the result on the stack.
		//分析表达式并计算返回值并放入栈中
		ParseExpr();

		// Determine which function we're returning from
		//确定从哪个函数返回
		if (g_ScriptHeader.iIsMainFuncPresent &&
			g_ScriptHeader.iMainFuncIndex == g_iCurrScope)
		{
			// It is _Main (), so pop the result into _T0
			//_Main函数，所以将结果弹到_T0
			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
			AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
		}
		else
		{
			// It's not _Main, so pop the result into the _RetVal register
			//不是_Main函数，弹到_RetVal寄存器
			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
			AddRegICodeOp(g_iCurrScope, iInstrIndex, REG_CODE_RETVAL);
		}
	}
	else
	{
		// Clear _T0 in case we're exiting _Main ()
		//清空_T0 如果我们退出_Main函数
		if (g_ScriptHeader.iIsMainFuncPresent &&
			g_ScriptHeader.iMainFuncIndex == g_iCurrScope)
		{

			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_MOV);
			AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
			AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);
		}
	}

	if (g_ScriptHeader.iIsMainFuncPresent &&
		g_ScriptHeader.iMainFuncIndex == g_iCurrScope)
	{
		// It's _Main, so exit the script with _T0 as the exit code
		//是_Main函数，所以退出并以_T0作为退出代码
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_EXIT);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
	}
	else
	{
		// It's not _Main, so return from the function
		//不是_Main函数，所以从函数返回
		AddICodeInstr(g_iCurrScope, INSTR_RET);
	}

	// Validate the presence of the semicolon
	//是否存在分号
	ReadToken(TOKEN_TYPE_DELIM_SEMICOLON);
}


/// <summary>
/// Parses an assignment statement.
/// <Ident> <Assign-Op> <Expr>;
/// 分析赋值语句
/// </summary>
void ParseAssign()
{
	// Make sure we're inside a function
	//确保在一个函数中
	if (g_iCurrScope == SCOPE_GLOBAL)
		ExitOnCodeError("Assignment illegal in global scope");

	int iInstrIndex;

	// Assignment operator
	//复制运算符
	int iAssignOp;

	// Annotate the line
	//标注行
	AddICodeSourceLine(g_iCurrScope, GetCurrSourceLine());

	//! Parse the variable or array 分析变量或数组
	SymbolNode* pSymbol = GetSymbolByIdent(GetCurrLexeme(), g_iCurrScope);

	// Does an array index follow the identifier?
	//标识符后面有数组索引？
	int iIsArray = FALSE;
	if (GetLookAheadChar() == '[')
	{
		// Ensure the variable is an array
		//确保变量是数组
		if (pSymbol->iSize == 1)
			ExitOnCodeError("Invalid array");

		// Verify the opening brace
		//校验左括号
		ReadToken(TOKEN_TYPE_DELIM_OPEN_BRACE);

		// Make sure an expression is present
		//确保表达式存在
		if (GetLookAheadChar() == ']')
			ExitOnCodeError("Invalid expression");

		// Parse the index as an expression
		//分析表达式得到索引
		ParseExpr();

		// Make sure the index is closed
		//确保索引闭合
		ReadToken(TOKEN_TYPE_DELIM_CLOSE_BRACE);

		// Set the array flag
		//设置数组标志
		iIsArray = TRUE;
	}
	else
	{
		// Make sure the variable isn't an array
		//确保不是数组
		if (pSymbol->iSize > 1)
			ExitOnCodeError("Arrays must be indexed");
	}

	//! Parse the assignment operator 解析赋值运算符
	if (GetNextToken() != TOKEN_TYPE_OP &&
		(GetCurrOp() != OP_TYPE_ASSIGN &&
			GetCurrOp() != OP_TYPE_ASSIGN_ADD &&
			GetCurrOp() != OP_TYPE_ASSIGN_SUB &&
			GetCurrOp() != OP_TYPE_ASSIGN_MUL &&
			GetCurrOp() != OP_TYPE_ASSIGN_DIV &&
			GetCurrOp() != OP_TYPE_ASSIGN_MOD &&
			GetCurrOp() != OP_TYPE_ASSIGN_EXP &&
			GetCurrOp() != OP_TYPE_ASSIGN_CONCAT &&
			GetCurrOp() != OP_TYPE_ASSIGN_AND &&
			GetCurrOp() != OP_TYPE_ASSIGN_OR &&
			GetCurrOp() != OP_TYPE_ASSIGN_XOR &&
			GetCurrOp() != OP_TYPE_ASSIGN_SHIFT_LEFT &&
			GetCurrOp() != OP_TYPE_ASSIGN_SHIFT_RIGHT))
		ExitOnCodeError("Illegal assignment operator");
	else
		iAssignOp = GetCurrOp();

	//! Parse the value expression 解析赋值表达式

	ParseExpr();

	// Validate the presence of the semicolon
	//验证分号是否存在
	ReadToken(TOKEN_TYPE_DELIM_SEMICOLON);

	// Pop the value into _T0
	//弹入_T0
	iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
	AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

	// If the variable was an array, pop the top of the stack into _T1 for use as the index
	//如果这个变量是数组，栈顶弹出到_T1作为索引
	if (iIsArray)
	{
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex);
	}

	//! Generate the I-code for the assignment instruction 为中间代码生成赋值指令
	switch (iAssignOp)
	{
	// =
	case OP_TYPE_ASSIGN:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_MOV);
		break;

	// +=
	case OP_TYPE_ASSIGN_ADD:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_ADD);
		break;

	// -=
	case OP_TYPE_ASSIGN_SUB:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_SUB);
		break;

	// *=
	case OP_TYPE_ASSIGN_MUL:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_MUL);
		break;

	// /=
	case OP_TYPE_ASSIGN_DIV:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_DIV);
		break;

	// %=
	case OP_TYPE_ASSIGN_MOD:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_MOD);
		break;

	// ^=
	case OP_TYPE_ASSIGN_EXP:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_EXP);
		break;

	// $=
	case OP_TYPE_ASSIGN_CONCAT:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_CONCAT);
		break;

	// &=
	case OP_TYPE_ASSIGN_AND:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_AND);
		break;

	// |=
	case OP_TYPE_ASSIGN_OR:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_OR);
		break;

	// #=
	case OP_TYPE_ASSIGN_XOR:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_XOR);
		break;

	// <<=
	case OP_TYPE_ASSIGN_SHIFT_LEFT:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_SHL);
		break;

	// >>=
	case OP_TYPE_ASSIGN_SHIFT_RIGHT:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_SHR);
		break;
	}

	// Generate the destination operand
	//生成目标操作数
	if (iIsArray)
		AddArrayIndexVarICodeOp(g_iCurrScope, iInstrIndex, pSymbol->iIndex, g_iTempVar1SymbolIndex);
	else
		AddVarICodeOp(g_iCurrScope, iInstrIndex, pSymbol->iIndex);

	// Generate the source
	//生成源操作数
	AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
}


/// <summary>
///  Parses a function call
/// <Ident> ( <Expr>, <Expr> );
/// 分析函数调用
/// </summary>
void ParseFuncCall()
{
	// Get the function by it's identifier
	//根据标识符获得函数
	FuncNode* pFunc = GetFuncByName(GetCurrLexeme());

	// It is, so start the parameter count at zero
	//如果是函数，参数计数设置0
	int iParamCount = 0;

	// Attempt to read the opening parenthesis
	//尝试读取左括号
	ReadToken(TOKEN_TYPE_DELIM_OPEN_PAREN);

	// Parse each parameter and push it onto the stack
	//分析每个参数并压栈
	while (TRUE)
	{
		// Find out if there's another parameter to push
		//查看是否还有参数
		if (GetLookAheadChar() != ')')
		{
			// There is, so parse it as an expression
			//还有参数，当作一个表达式解析
			ParseExpr();

			// Increment the parameter count and make sure it's not greater than the amount accepted by the function (unless it's a host API function
			//增加参数计数
			++iParamCount;
			if (!pFunc->iIsHostAPI && iParamCount > pFunc->iParamCount)
				ExitOnCodeError("Too many parameters");

			// Unless this is the final parameter, attempt to read a comma
			//如果不是最后一个参数。读取逗号
			if (GetLookAheadChar() != ')')
				ReadToken(TOKEN_TYPE_DELIM_COMMA);
		}
		else
		{
			// There isn't, so break the loop and complete the call
			//没有逗号，退出循环完成分析
			break;
		}
	}

	// Attempt to read the closing parenthesis
	//检查右括号
	ReadToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);

	// Make sure the parameter wasn't passed too few parameters (unless it's a host API function)
	//检查参数是否太少
	if (!pFunc->iIsHostAPI && iParamCount < pFunc->iParamCount)
		ExitOnCodeError("Too few parameters");

	// Call the function, but make sure the right call instruction is used
	//调用函数，确保正确的使用函数指令
	int iCallInstr = INSTR_CALL;
	if (pFunc->iIsHostAPI)
		iCallInstr = INSTR_CALLHOST;

	int iInstrIndex = AddICodeInstr(g_iCurrScope, iCallInstr);
	AddFuncICodeOp(g_iCurrScope, iInstrIndex, pFunc->iIndex);
}