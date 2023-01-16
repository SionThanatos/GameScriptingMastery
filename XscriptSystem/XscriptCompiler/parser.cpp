//Parser module
//�﷨������ģ��





#include "parser.h"
#include "error.h"
#include "lexer.h"
#include "symbol_table.h"
#include "func_table.h"
#include "i_code.h"

// ---- Globals -------------------------------------------------------------------------------

// ---- Functions -------------------------------------------------------------------------

int g_iCurrScope;                               // The current scope ��ǰ��Χ Ϊ0��ʾȫ�֣���0��ʾ����

// ---- Loops -----------------------------------------------------------------------------

Stack g_LoopStack;                              // Loop handling stack ����ѭ����ջ

// ---- Functions -----------------------------------------------------------------------------

/// <summary>
/// Attempts to read a specific token and prints an error if its not found.
/// GetNextToken��װ������ָ�������Է�
/// </summary>
/// <param name="ReqToken">ָ�������Է�</param>
void ReadToken(Token ReqToken)
{
	// Determine if the next token is the required one
	//�ж���һ�����Է��Ƿ�����Ҫ ��
	if (GetNextToken() != ReqToken)
	{
		// If not, exit on a specific error
		//���ǣ�����������Ϣ
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
		//��ʾ����
		ExitOnCodeError(pstrErrorMssg);
	}
}

/// <summary>
/// Determines if the specified operator is a relational operator.
/// �Ƿ��ǹ�ϵ�����
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
/// �Ƿ����߼������
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
/// ����Դ���룬�����м�����ʾ
/// </summary>
void ParseSourceCode()
{
	// Reset the lexer
	//���ôʷ�������
	ResetLexer();

	// Initialize the loop stack
	//��ʼ��ѭ��ջ
	InitStack(&g_LoopStack);

	// Set the current scope to global
	//���õ�ǰ������Ϊȫ��
	g_iCurrScope = SCOPE_GLOBAL;

	// Parse each line of code
	//����ÿһ�д���
	while (TRUE)
	{
		// Parse the next statement and ignore an end of file marker
		//������һ����䲢�����ļ��������
		ParseStatement();

		// If we're at the end of the token stream, break the parsing loop
		//����Ѿ��������Է����Ľ�β������
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
/// �������ս�� Statement(���)
/// </summary>
void ParseStatement()
{
	// If the next token is a semicolon, the statement is empty so return
	//�����һ�����Է��Ƿֺţ�����һ�������
	if (GetLookAheadChar() == ';')
	{
		ReadToken(TOKEN_TYPE_DELIM_SEMICOLON);
		return;
	}

	// Determine the initial token of the statement
	//ȷ�����Ŀ�ʼ���Է�
	Token InitToken = GetNextToken();

	// Branch to a parse function based on the token
	//�������Է���֧����ͬ�ķ�������
	switch (InitToken)
	{
	// Unexpected end of file
	//�ļ��������
	case TOKEN_TYPE_END_OF_STREAM:
		ExitOnCodeError("Unexpected end of file");
		break;

	// Block
    //��
	case TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE:
		ParseBlock();
		break;

	// Variable/array declaration
	//���������� ����
	case TOKEN_TYPE_RSRVD_VAR:
		ParseVar();
		break;

	// Host API function import
	//host api��������
	case TOKEN_TYPE_RSRVD_HOST:
		ParseHost();
		break;

	// Function definition
	//��������
	case TOKEN_TYPE_RSRVD_FUNC:
		ParseFunc();
		break;

	// if block
	//����ǿ�
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
	//����
	case TOKEN_TYPE_RSRVD_RETURN:
		ParseReturn();
		break;

	// Assignment or Function Call

	case TOKEN_TYPE_IDENT:
	{
		// What kind of identifier is it?
		//��ʲô���͵ı�ʶ��
		if (GetSymbolByIdent(GetCurrLexeme(), g_iCurrScope))
		{
			// It's an identifier, so treat the statement as an assignment
			//����Ǳ�ʶ����������ֵ���
			ParseAssign();
		}
		else if (GetFuncByName(GetCurrLexeme()))
		{
			// It's a function
			//�Ǻ���
			// Annotate the line and parse the call
			//��ע�д��벢������������
			AddICodeSourceLine(g_iCurrScope, GetCurrSourceLine());
			ParseFuncCall();

			// Verify the presence of the semicolon
			//����Ƿ��зֺ�
			ReadToken(TOKEN_TYPE_DELIM_SEMICOLON);
		}
		else
		{
			// It's invalid
			//��Ч
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
/// ���������
/// </summary>
void ParseBlock()
{
	//��ߵ������Ѿ���ParseStatement�ж�ȡ��
	// Make sure we're not in the global scope
	//ȷ������ȫ����������
	if (g_iCurrScope == SCOPE_GLOBAL)
		ExitOnCodeError("Code blocks illegal in global scope");

	// Read each statement until the end of the block
	//����ÿ�����ֱ�������
	while (GetLookAheadChar() != '}')//�����һ�����
		ParseStatement();

	// Read the closing curly brace
	//��������ŵ��ұ�
	ReadToken(TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE);
}


/// <summary>
/// Parses the declaration of a variable or array and adds it to the symbol table.
/// var <Identifier>;
/// var <Identifier> [ <Integer> ];
/// ������������������������뵽���ű���
/// </summary>
void ParseVar()
{
	// Read an identifier token
	//��ȡ��ʶ�����Է�
	ReadToken(TOKEN_TYPE_IDENT);

	// Copy the current lexeme into a local string buffer to save the variable's identifier
	//����ǰ���ʸ��Ƶ��ֲ��ַ����������Ա���
	char pstrIdent[MAX_LEXEME_SIZE];
	CopyCurrLexeme(pstrIdent);

	// Set the size to 1 for a variable (an array will update this value)
	//�������Ĵ�С����Ϊ1
	int iSize = 1;

	// Is the look-ahead character an open brace?
	//Ԥ��������ַ���������[��
	if (GetLookAheadChar() == '[')
	{
		// Verify the open brace
		//У��������
		ReadToken(TOKEN_TYPE_DELIM_OPEN_BRACE);

		// If so, read an integer token
		//�ǣ����ȡ�������Է�
		ReadToken(TOKEN_TYPE_INT);

		// Convert the current lexeme to an integer to get the size
		//����ǰ����ת��Ϊ��������ô�С
		iSize = atoi(GetCurrLexeme());

		// Read the closing brace
		//��ȡ������
		ReadToken(TOKEN_TYPE_DELIM_CLOSE_BRACE);
	}

	// Add the identifier and size to the symbol table
	//����ʶ���ʹ�С��ӵ����ű���
	if (AddSymbol(pstrIdent, iSize, g_iCurrScope, SYMBOL_TYPE_VAR) == -1)
		ExitOnCodeError("Identifier redefinition");

	// Read the semicolon
	//��ȡ�ֺ�
	ReadToken(TOKEN_TYPE_DELIM_SEMICOLON);
}



/// <summary>
/// Parses the importing of a host API function.
/// host <Identifier> ();
/// ����host api����
/// </summary>
void ParseHost()
{
	// Read the host API function name
	//��ȡhost api��������
	ReadToken(TOKEN_TYPE_IDENT);

	// Add the function to the function table with the host API flag set
	//��������ӵ�������������host api���
	if (AddFunc(GetCurrLexeme(), TRUE) == -1)
		ExitOnCodeError("Function redefinition");

	// Make sure the function name is followed with ()
	//ȷ��������������ţ���
	ReadToken(TOKEN_TYPE_DELIM_OPEN_PAREN);
	ReadToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);

	// Read the semicolon
	//��ȡ�ֺ�
	ReadToken(TOKEN_TYPE_DELIM_SEMICOLON);
}

/// <summary>
/// Parses a function.
/// ��������
/// func <Identifier> ( <Parameter-List> ) <Statement>
/// </summary>
void ParseFunc()
{
	// Make sure we're not already in a function
	//ȷ�����ڲ���һ�������У������ڲ����ܶ��庯��
	if (g_iCurrScope != SCOPE_GLOBAL)
		ExitOnCodeError("Nested functions illegal");

	// Read the function name
	//��ȡ��������(��ʾ���Է�)
	ReadToken(TOKEN_TYPE_IDENT);

	// Add the non-host API function to the function table and get its index
	//����hostӦ��api�������뵽�������в����������
	int iFuncIndex = AddFunc(GetCurrLexeme(), FALSE);

	// Check for a function redefinition
	//��麯�����ظ�����
	if (iFuncIndex == -1)
		ExitOnCodeError("Function redefinition");

	// Set the scope to the function
	//���ú�����������
	g_iCurrScope = iFuncIndex;

	// Read the opening parenthesis
	//��ȡ������
	ReadToken(TOKEN_TYPE_DELIM_OPEN_PAREN);

	// Use the look-ahead character to determine if the function takes parameters
	//ʹ��Ԥ���鿴�ַ���ȷ�������Ƿ��в���
	if (GetLookAheadChar() != ')')
	{
		// If the function being defined is _Main (), flag an error since _Main () cannot accept paraemters
		//���������Main����һ�������ǣ���ΪMain���������ܲ���
		if (g_ScriptHeader.iIsMainFuncPresent &&
			g_ScriptHeader.iMainFuncIndex == iFuncIndex)
		{
			ExitOnCodeError("_Main () cannot accept parameters");
		}

		// Start the parameter count at zero
		//����������0��ʼ
		int iParamCount = 0;

		// Crete an array to store the parameter list locally
		//����һ�����鱣�汾�ز����б�
		char ppstrParamList[MAX_FUNC_DECLARE_PARAM_COUNT][MAX_IDENT_SIZE];

		// Read the parameters
		//��ȡ����
		while (TRUE)
		{
			// Read the identifier
			//��ȡ��ʶ��
			ReadToken(TOKEN_TYPE_IDENT);

			// Copy the current lexeme to the parameter list array
			//����ǰ�Ĵ��ظ��Ƶ������б�
			CopyCurrLexeme(ppstrParamList[iParamCount]);

			// Increment the parameter count
			//���Ӳ�������
			++iParamCount;

			// Check again for the closing parenthesis to see if the parameter list is done
			//�����������ȷ�������б��Ƿ����
			if (GetLookAheadChar() == ')')
				break;

			// Otherwise read a comma and move to the next parameter
			//�����ȡ��һ�����ţ�������һ������
			ReadToken(TOKEN_TYPE_DELIM_COMMA);
		}

		// Set the final parameter count
		//�������յĲ�������
		SetFuncParamCount(g_iCurrScope, iParamCount);

		// Write the parameters to the function's symbol table in reverse order, so they'll be emitted from right-to-left
		//����������д�뺯�����ű��У�����˳�����ҵ�������
		while (iParamCount > 0)
		{
			--iParamCount;

			// Add the parameter to the symbol table
			//������д����ű�
			AddSymbol(ppstrParamList[iParamCount], 1, g_iCurrScope, SYMBOL_TYPE_PARAM);
		}
	}

	// Read the closing parenthesis
	//����������
	ReadToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);

	//! ����������

	// Read the opening curly brace
	//��ȡ�������
	ReadToken(TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE);

	// Parse the function's body
	//����������
	ParseBlock();

	// Return to the global scope
	//���ص�ȫ�ַ�Χ
	g_iCurrScope = SCOPE_GLOBAL;
}

/// <summary>
/// Parses an expression.
/// �������ʽ
/// </summary>
void ParseExpr()
{
	int iInstrIndex;

	// The current operator type
	//��ǰ���������
	int iOpType;

	// Parse the subexpression
	//�����ӱ��ʽ
	ParseSubExpr();

	// Parse any subsequent relational or logical operators
	//����������+����-�����
	while (TRUE)
	{
		// Get the next token
		//��ȡ��һ�����Է�
		if (GetNextToken() != TOKEN_TYPE_OP ||
			(!IsOpRelational(GetCurrOp()) &&
				!IsOpLogical(GetCurrOp())))
		{
			RewindTokenStream();
			break;
		}

		// Save the operator
		//���������
		iOpType = GetCurrOp();

		// Parse the second term
		//�����ڶ���
		ParseSubExpr();

		// Pop the first operand into _T1
		//����һ������������_T1
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex);

		// Pop the second operand into _T0
		//���ڶ�������������_T0
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

		//! Perform the binary operation associated with the specified operator �����ض��������������Ӧ������

		// Determine the operator type
		//�ж����������
		if (IsOpRelational(iOpType))
		{
			// Get a pair of free jump target indices
			//��ȡ������תĿ��
			int iTrueJumpTargetIndex = GetNextJumpTargetIndex(),
				iExitJumpTargetIndex = GetNextJumpTargetIndex();

			// It's a relational operator
			//��ϵ�����
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
			//!     JCC���߱�� _T0,_T1 True
			//		PUSH 0
			//		JMP Exit
			//True:
			//		PUSH 1
			//Exit:


			// Add the jump instruction's operands (_T0 and _T1)
			//��������������T0 T1
			AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
			AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex);
			AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iTrueJumpTargetIndex);

			// Generate the outcome for falsehood
			//����Ϊ�ٵĴ���
			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
			AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);

			// Generate a jump past the true outcome
			//��������Ϊ�沿�ִ����ָ��
			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JMP);
			AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iExitJumpTargetIndex);

			// Set the jump target for the true outcome
			//����Ϊ�����תĿ��
			AddICodeJumpTarget(g_iCurrScope, iTrueJumpTargetIndex);

			// Generate the outcome for truth
			//����Ϊ�沿�ֵĴ���
			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
			AddIntICodeOp(g_iCurrScope, iInstrIndex, 1);

			// Set the jump target for exiting the operand evaluation
			//����������������ֵ����תĿ��
			AddICodeJumpTarget(g_iCurrScope, iExitJumpTargetIndex);
		}
		else
		{
			// It must be a logical operator
			//�߼������
			switch (iOpType)
			{
			// And 
			//! �߼��� ���������תΪ���Ĵ�������
			case OP_TYPE_LOGICAL_AND:
			{
				//		JE _T0, 0, False
				//		JE _T1, 0, False
				//		Push 1
				//		Jmp Exit
				// False:
				//		Push 0
				// Exit
				//ֻҪ��һ��������Ϊ0�����ΪFalse
				 
				// Get a pair of free jump target indices
				//���һ��δʹ�õ���תĿ������
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
			//�߼���
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
/// �����ӱ��ʽ
/// </summary>
void ParseSubExpr()
{
	int iInstrIndex;

	// The current operator type
	//��ǰ�����
	int iOpType;

	// Parse the first term
	//������һ��
	ParseTerm();

	// Parse any subsequent +, - or $ operators
	//���������+-��$�����
	while (TRUE)
	{
		// Get the next token
		//��ȡ��һ�����Է�
		if (GetNextToken() != TOKEN_TYPE_OP ||
			(GetCurrOp() != OP_TYPE_ADD &&
				GetCurrOp() != OP_TYPE_SUB &&
				GetCurrOp() != OP_TYPE_CONCAT))
		{
			RewindTokenStream();
			break;
		}

		// Save the operator
		//���������
		iOpType = GetCurrOp();

		// Parse the second term
		//�����ڶ���
		ParseTerm();

		// Pop the first operand into _T1
		//����һ������������_T1
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex);

		// Pop the second operand into _T0
		//���ڶ�������������_T0
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
/// ����������м�Ĳ�����
/// </summary>
void ParseTerm()
{
	int iInstrIndex;

	// The current operator type
	//��ǰ���������
	int iOpType;

	// Parse the first factor
	//������һ������
	ParseFactor();

	// Parse any subsequent *, /, %, ^, &, |, #, << and >> operators
	//����������*, /, %, ^, &, |, #, << and >> �����
	while (TRUE)
	{
		// Get the next token
		//��ȡ��һ�����Է�
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
		//���������
		iOpType = GetCurrOp();

		// Parse the second factor
		//�����ڶ�������
		ParseFactor();

		// Pop the first operand into _T1
		//����һ������������_T1
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex);

		// Pop the second operand into _T0
		//���ڶ�������������_T2
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

		// Perform the binary operation associated with the specified operator
		//�����������ɲ���
		int iOpInstr;
		switch (iOpType)
		{
		// Binary multiplication
		//�˷�
		case OP_TYPE_MUL:
			iOpInstr = INSTR_MUL;
			break;

		// Binary division
		//����
		case OP_TYPE_DIV:
			iOpInstr = INSTR_DIV;
			break;

		// Binary modulus
		//ģ
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
		//�����ջ ������_T0��
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
	//���ȼ�鵥Ŀ�����
	if (GetNextToken() == TOKEN_TYPE_OP &&
		(GetCurrOp() == OP_TYPE_ADD ||
			GetCurrOp() == OP_TYPE_SUB ||
			GetCurrOp() == OP_TYPE_BITWISE_NOT ||
			GetCurrOp() == OP_TYPE_LOGICAL_NOT))
	{
		// If it was found, save it and set the unary operator flag
		//�ҵ����沢��ǵ�Ŀ������
		iUnaryOpPending = TRUE;
		iOpType = GetCurrOp();
	}
	else
	{
		// Otherwise rewind the token stream
		//�������
		RewindTokenStream();
	}

	// Determine which type of factor we're dealing with based on the next token
	//������һ�����Է�ȷ���������ڴ�����������
	switch (GetNextToken())
	{
	// It's a true or false constant, so push either 0 and 1 onto the stack
	//��true��false��������0��1ѹջ
	case TOKEN_TYPE_RSRVD_TRUE:
	case TOKEN_TYPE_RSRVD_FALSE:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
		AddIntICodeOp(g_iCurrScope, iInstrIndex, GetCurrToken() == TOKEN_TYPE_RSRVD_TRUE ? 1 : 0);
		break;

	// It's an integer literal, so push it onto the stack
	//���ͣ�ѹջ
	case TOKEN_TYPE_INT:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
		AddIntICodeOp(g_iCurrScope, iInstrIndex, atoi(GetCurrLexeme()));
		break;

	// It's a float literal, so push it onto the stack
	//���㣬ѹջ
	case TOKEN_TYPE_FLOAT:
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
		AddFloatICodeOp(g_iCurrScope, iInstrIndex, (float)atof(GetCurrLexeme()));
		break;

	// It's a string literal, so add it to the string table and push the resulting string index onto the stack
	//�ַ��� ���뵽�ַ������Ұ��ַ�������ѹջ
	case TOKEN_TYPE_STRING:
	{
		int iStringIndex = AddString(&g_StringTable, GetCurrLexeme());
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
		AddStringICodeOp(g_iCurrScope, iInstrIndex, iStringIndex);
		break;
	}

	// It's an identifier
	//��ʶ��
	case TOKEN_TYPE_IDENT:
	{
	// First find out if the identifier is a variable or array
	//�ж������黹�Ǳ���
		SymbolNode* pSymbol = GetSymbolByIdent(GetCurrLexeme(), g_iCurrScope);
		if (pSymbol)
		{
			// Does an array index follow the identifier?
			//���������
			if (GetLookAheadChar() == '[')
			{
				// Ensure the variable is an array
				//ȷ��������
				if (pSymbol->iSize == 1)
					ExitOnCodeError("Invalid array");

				// Verify the opening brace
				//���������
				ReadToken(TOKEN_TYPE_DELIM_OPEN_BRACE);

				// Make sure an expression is present
				//ȷ�����ʽ����
				if (GetLookAheadChar() == ']')
					ExitOnCodeError("Invalid expression");

				// Parse the index as an expression recursively
				//�ݹ�����������ʽ
				ParseExpr();

				// Make sure the index is closed
				//ȷ�����������Ŵ���
				ReadToken(TOKEN_TYPE_DELIM_CLOSE_BRACE);

				// Pop the resulting value into _T0 and use it as the index variable
				//���������_T0��������Ϊ��������
				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
				AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

				// Push the original identifier onto the stack as an array, indexed with _T0
				//��ԭ���ı�ʶ��ѹջ����_T0��Ϊ����
				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
				AddArrayIndexVarICodeOp(g_iCurrScope, iInstrIndex, pSymbol->iIndex, g_iTempVar0SymbolIndex);
			}
			else
			{
				// If not, make sure the identifier is not an array, and push it onto the stack
				//������ǣ�ȷ�������ʶ������һ�����飬ѹջ
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
			//��ʶ�����Ǳ��������飬�����ǲ��Ǻ���
			if (GetFuncByName(GetCurrLexeme()))
			{
				// It is, so parse the call
				//�Ǻ�������������
				ParseFuncCall();

				// Push the return value
				//ѹ�뷵��ֵ
				iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
				AddRegICodeOp(g_iCurrScope, iInstrIndex, REG_CODE_RETVAL);
			}
		}

		break;
	}

	// It's a nested expression, so call ParseExpr () recursively and validate the presence of the closing parenthesis
	//��һ��Ƕ�׵ı��ʽ�����Եݹ����ParseExpr ()�����������
	case TOKEN_TYPE_DELIM_OPEN_PAREN:
		ParseExpr();
		ReadToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);
		break;

		// Anything else is invalid

	default:
		ExitOnCodeError("Invalid input");
	}

	// Is a unary operator pending?
	//����û�д���ĵ�Ŀ�������
	if (iUnaryOpPending)
	{
		// If so, pop the result of the factor off the top of the stack
		//���У��Ӷ�ջ�е�������
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

		// Perform the unary operation
		//ִ�е�Ŀ�������
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
			//ȡ��
			case OP_TYPE_SUB:
				iOpIndex = INSTR_NEG;
				break;

				// Bitwise not

			case OP_TYPE_BITWISE_NOT:
				iOpIndex = INSTR_NOT;
				break;
			}

			// Add the instruction's operand
			//���ָ��Ĳ�����
			iInstrIndex = AddICodeInstr(g_iCurrScope, iOpIndex);
			AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

			// Push the result onto the stack
			//�����ѹջ
			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_PUSH);
			AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
		}
	}
}




/// <summary>
/// Parses an if block.
/// if ( <Expression> ) <Statement>
/// if ( <Expression> ) <Statement> else <Statement>
/// ����if ��
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
	//�����������_T0����0�Ƚ�
	iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
	AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

	// If the result is zero, jump to the false target
	//���Ϊ0 ��ת��falseĿ��
	iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JE);
	AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
	AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);
	AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iFalseJumpTargetIndex);

	// Parse the true block
	//����true ��
	ParseStatement();

	// Look for an else clause
	//����else�Ӿ�
	if (GetNextToken() == TOKEN_TYPE_RSRVD_ELSE)
	{
		// If it's found, append the true block with an unconditional jump past the false block
		//����ҵ�����true���������������ת������false��
		int iSkipFalseJumpTargetIndex = GetNextJumpTargetIndex();
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JMP);
		AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iSkipFalseJumpTargetIndex);

		// Place the false target just before the false block
		//��falseĿ�����false��ǰ
		AddICodeJumpTarget(g_iCurrScope, iFalseJumpTargetIndex);

		// Parse the false block
		//����false��
		ParseStatement();

		// Set a jump target beyond the false block
		//����false��������תĿ��
		AddICodeJumpTarget(g_iCurrScope, iSkipFalseJumpTargetIndex);
	}
	else
	{
		// Otherwise, put the token back
		//����������Է�
		RewindTokenStream();

		// Place the false target after the true block
		//��false�����true�����ĺ���
		AddICodeJumpTarget(g_iCurrScope, iFalseJumpTargetIndex);
	}
}



/// <summary>
/// Parses a while loop block.
/// while ( <Expression> ) <Statement>
/// ����while
/// </summary>
void ParseWhile()
{
	int iInstrIndex;

	// Make sure we're inside a function

	if (g_iCurrScope == SCOPE_GLOBAL)
		ExitOnCodeError("Statement illegal in global scope");

	// Annotate the line
	//�����
	AddICodeSourceLine(g_iCurrScope, GetCurrSourceLine());

	// Get two jump targets; for the top and bottom of the loop
	//��ȡ������תĿ�꣬�ֱ����ڶ���ѭ���͵ײ�
	int iStartTargetIndex = GetNextJumpTargetIndex(),
		iEndTargetIndex = GetNextJumpTargetIndex();

	// Set a jump target at the top of the loop
	//��ѭ�����������һ����תĿ��
	AddICodeJumpTarget(g_iCurrScope, iStartTargetIndex);

	// Read the opening parenthesis
	//��ȡ������
	ReadToken(TOKEN_TYPE_DELIM_OPEN_PAREN);

	// Parse the expression and leave the result on the stack
	//�������ʽ���������ջ
	ParseExpr();

	// Read the closing parenthesis
	//��ȡ������
	ReadToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);

	// Pop the result into _T0 and jump out of the loop if it's nonzero
	//�����������_T0 ���������0 ������ѭ��
	iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
	AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

	iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JE);
	AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
	AddIntICodeOp(g_iCurrScope, iInstrIndex, 0);
	AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iEndTargetIndex);

	// Create a new loop instance structure
	//����һ���µ�ѭ��ʵ��
	Loop* pLoop = (Loop*)malloc(sizeof(Loop));

	// Set the starting and ending jump target indices
	//���ÿ�ʼ�ͽ�����ת����Ŀ��
	pLoop->iStartTargetIndex = iStartTargetIndex;
	pLoop->iEndTargetIndex = iEndTargetIndex;

	// Push the loop structure onto the stack
	//��ѭ���ṹѹ���ջ
	Push(&g_LoopStack, pLoop);

	// Parse the loop body
	//����ѭ����
	ParseStatement();

	// Pop the loop instance off the stack
	//����ѭ����ʵ��
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
	//���ѭ����������תĿ������
	int iTargetIndex = ((Loop*)Peek(&g_LoopStack))->iEndTargetIndex;

	// Unconditionally jump to the end of the loop
	//��������ת��ѭ���Ľ���λ��
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
	//��ȡѭ����ʼλ����תĿ������
	int iTargetIndex = ((Loop*)Peek(&g_LoopStack))->iStartTargetIndex;

	// Unconditionally jump to the end of the loop
	//��������ת
	int iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_JMP);
	AddJumpTargetICodeOp(g_iCurrScope, iInstrIndex, iTargetIndex);
}




/// <summary>
/// Parses a return statement.
/// ��������
/// return <expr>;
/// return;
/// </summary>
void ParseReturn()
{
	int iInstrIndex;

	// Make sure we're inside a function
	//ȷ���ں�����
	if (g_iCurrScope == SCOPE_GLOBAL)
		ExitOnCodeError("return illegal in global scope");

	// Annotate the line
	//��ע��
	AddICodeSourceLine(g_iCurrScope, GetCurrSourceLine());

	// If a semicolon doesn't appear to follow, parse the expression and place it in _RetVal
	//�������û�зֺţ��������ʽ�����������_RetVal
	if (GetLookAheadChar() != ';')
	{
		// Parse the expression to calculate the return value and leave the result on the stack.
		//�������ʽ�����㷵��ֵ������ջ��
		ParseExpr();

		// Determine which function we're returning from
		//ȷ�����ĸ���������
		if (g_ScriptHeader.iIsMainFuncPresent &&
			g_ScriptHeader.iMainFuncIndex == g_iCurrScope)
		{
			// It is _Main (), so pop the result into _T0
			//_Main���������Խ��������_T0
			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
			AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
		}
		else
		{
			// It's not _Main, so pop the result into the _RetVal register
			//����_Main����������_RetVal�Ĵ���
			iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
			AddRegICodeOp(g_iCurrScope, iInstrIndex, REG_CODE_RETVAL);
		}
	}
	else
	{
		// Clear _T0 in case we're exiting _Main ()
		//���_T0 ��������˳�_Main����
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
		//��_Main�����������˳�����_T0��Ϊ�˳�����
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_EXIT);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
	}
	else
	{
		// It's not _Main, so return from the function
		//����_Main���������ԴӺ�������
		AddICodeInstr(g_iCurrScope, INSTR_RET);
	}

	// Validate the presence of the semicolon
	//�Ƿ���ڷֺ�
	ReadToken(TOKEN_TYPE_DELIM_SEMICOLON);
}


/// <summary>
/// Parses an assignment statement.
/// <Ident> <Assign-Op> <Expr>;
/// ������ֵ���
/// </summary>
void ParseAssign()
{
	// Make sure we're inside a function
	//ȷ����һ��������
	if (g_iCurrScope == SCOPE_GLOBAL)
		ExitOnCodeError("Assignment illegal in global scope");

	int iInstrIndex;

	// Assignment operator
	//���������
	int iAssignOp;

	// Annotate the line
	//��ע��
	AddICodeSourceLine(g_iCurrScope, GetCurrSourceLine());

	//! Parse the variable or array ��������������
	SymbolNode* pSymbol = GetSymbolByIdent(GetCurrLexeme(), g_iCurrScope);

	// Does an array index follow the identifier?
	//��ʶ������������������
	int iIsArray = FALSE;
	if (GetLookAheadChar() == '[')
	{
		// Ensure the variable is an array
		//ȷ������������
		if (pSymbol->iSize == 1)
			ExitOnCodeError("Invalid array");

		// Verify the opening brace
		//У��������
		ReadToken(TOKEN_TYPE_DELIM_OPEN_BRACE);

		// Make sure an expression is present
		//ȷ�����ʽ����
		if (GetLookAheadChar() == ']')
			ExitOnCodeError("Invalid expression");

		// Parse the index as an expression
		//�������ʽ�õ�����
		ParseExpr();

		// Make sure the index is closed
		//ȷ�������պ�
		ReadToken(TOKEN_TYPE_DELIM_CLOSE_BRACE);

		// Set the array flag
		//���������־
		iIsArray = TRUE;
	}
	else
	{
		// Make sure the variable isn't an array
		//ȷ����������
		if (pSymbol->iSize > 1)
			ExitOnCodeError("Arrays must be indexed");
	}

	//! Parse the assignment operator ������ֵ�����
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

	//! Parse the value expression ������ֵ���ʽ

	ParseExpr();

	// Validate the presence of the semicolon
	//��֤�ֺ��Ƿ����
	ReadToken(TOKEN_TYPE_DELIM_SEMICOLON);

	// Pop the value into _T0
	//����_T0
	iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
	AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);

	// If the variable was an array, pop the top of the stack into _T1 for use as the index
	//���������������飬ջ��������_T1��Ϊ����
	if (iIsArray)
	{
		iInstrIndex = AddICodeInstr(g_iCurrScope, INSTR_POP);
		AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex);
	}

	//! Generate the I-code for the assignment instruction Ϊ�м�������ɸ�ֵָ��
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
	//����Ŀ�������
	if (iIsArray)
		AddArrayIndexVarICodeOp(g_iCurrScope, iInstrIndex, pSymbol->iIndex, g_iTempVar1SymbolIndex);
	else
		AddVarICodeOp(g_iCurrScope, iInstrIndex, pSymbol->iIndex);

	// Generate the source
	//����Դ������
	AddVarICodeOp(g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex);
}


/// <summary>
///  Parses a function call
/// <Ident> ( <Expr>, <Expr> );
/// ������������
/// </summary>
void ParseFuncCall()
{
	// Get the function by it's identifier
	//���ݱ�ʶ����ú���
	FuncNode* pFunc = GetFuncByName(GetCurrLexeme());

	// It is, so start the parameter count at zero
	//����Ǻ�����������������0
	int iParamCount = 0;

	// Attempt to read the opening parenthesis
	//���Զ�ȡ������
	ReadToken(TOKEN_TYPE_DELIM_OPEN_PAREN);

	// Parse each parameter and push it onto the stack
	//����ÿ��������ѹջ
	while (TRUE)
	{
		// Find out if there's another parameter to push
		//�鿴�Ƿ��в���
		if (GetLookAheadChar() != ')')
		{
			// There is, so parse it as an expression
			//���в���������һ�����ʽ����
			ParseExpr();

			// Increment the parameter count and make sure it's not greater than the amount accepted by the function (unless it's a host API function
			//���Ӳ�������
			++iParamCount;
			if (!pFunc->iIsHostAPI && iParamCount > pFunc->iParamCount)
				ExitOnCodeError("Too many parameters");

			// Unless this is the final parameter, attempt to read a comma
			//����������һ����������ȡ����
			if (GetLookAheadChar() != ')')
				ReadToken(TOKEN_TYPE_DELIM_COMMA);
		}
		else
		{
			// There isn't, so break the loop and complete the call
			//û�ж��ţ��˳�ѭ����ɷ���
			break;
		}
	}

	// Attempt to read the closing parenthesis
	//���������
	ReadToken(TOKEN_TYPE_DELIM_CLOSE_PAREN);

	// Make sure the parameter wasn't passed too few parameters (unless it's a host API function)
	//�������Ƿ�̫��
	if (!pFunc->iIsHostAPI && iParamCount < pFunc->iParamCount)
		ExitOnCodeError("Too few parameters");

	// Call the function, but make sure the right call instruction is used
	//���ú�����ȷ����ȷ��ʹ�ú���ָ��
	int iCallInstr = INSTR_CALL;
	if (pFunc->iIsHostAPI)
		iCallInstr = INSTR_CALLHOST;

	int iInstrIndex = AddICodeInstr(g_iCurrScope, iCallInstr);
	AddFuncICodeOp(g_iCurrScope, iInstrIndex, pFunc->iIndex);
}