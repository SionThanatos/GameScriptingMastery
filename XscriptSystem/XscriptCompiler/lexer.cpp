//Lexical analyzer module
//词法分析器模块

#include "lexer.h"

// ---- Globals -------------------------------------------------------------------------------

	// ---- Lexer -----------------------------------------------------------------------------

LexerState g_CurrLexerState;                    // The current lexer state 当前词法分析器状态
LexerState g_PrevLexerState;                    // The previous lexer state (used for rewinding the token stream) 前一个词法分析器状态,用于回退属性符流

// ---- Operators -------------------------------------------------------------------------

//! First operator characters 运算符字符串中的第一个字符
OpState g_OpChars0[MAX_OP_STATE_COUNT] =
{
	//状态 后继状态数组的索引 后继状态的个数 运算符索引
	{ '+', 0, 2, 0 },
	{ '-', 2, 2, 1 },
	{ '*', 4, 1, 2 },
	{ '/', 5, 1, 3 },
	{ '%', 6, 1, 4 },
	{ '^', 7, 1, 5 },
	{ '&', 8, 2, 6 },
	{ '|', 10, 2, 7 },
	{ '#', 12, 1, 8 },
	{ '~', 0, 0, 9 },
	{ '!', 13, 1, 10 },
	{ '=', 14, 1, 11 },
	{ '<', 15, 2, 12 },
	{ '>', 17, 2, 13 },

	{ '$', 19, 1, 35 }
};

//! Second operator characters 运算符字符串中的第二个字符
OpState g_OpChars1[MAX_OP_STATE_COUNT] =
{
											  { '=', 0, 0, 14 }, { '+', 0, 0, 15 },     // +=, ++
											  { '=', 0, 0, 16 }, { '-', 0, 0, 17 },     // -=, --
											  { '=', 0, 0, 18 },                        // *=
											  { '=', 0, 0, 19 },                        // /=
											  { '=', 0, 0, 20 },                        // %=
											  { '=', 0, 0, 21 },                        // ^=
											  { '=', 0, 0, 22 }, { '&', 0, 0, 23 },     // &=, &&
											  { '=', 0, 0, 24 }, { '|', 0, 0, 25 },     // |=, ||
											  { '=', 0, 0, 26 },                        // #=
											  { '=', 0, 0, 27 },                        // !=
											  { '=', 0, 0, 28 },                        // ==
											  { '=', 0, 0, 29 }, { '<', 0, 1, 30 },     // <=, <<
											  { '=', 0, 0, 31 }, { '>', 1, 1, 32 },     // >=, >>
											  { '=', 0, 0, 36 }                         // $=
};

//! Third operator characters 运算符字符串中的第三个字符
OpState g_OpChars2[MAX_OP_STATE_COUNT] =
{
	{ '=', 0, 0, 33 }, { '=', 0, 0, 34 } // <<=, >>=
};

//! Delimiters 分隔符集合
char cDelims[MAX_DELIM_COUNT] = { ',', '(', ')', '[', ']', '{', '}', ';' };

// ---- Function Prototypes -------------------------------------------------------------------

int GetOpStateIndex(char cChar, int iCharIndex, int iSubStateIndex, int iSubStateCount);
int IsCharOpChar(char cChar, int iCharIndex);
OpState GetOpState(int iCharIndex, int iStateIndex);
int IsCharDelim(char cChar);
int IsCharWhitespace(char cChar);
int IsCharNumeric(char cChar);
int IsCharIdent(char cChar);

char GetNextChar();

// ---- Functions -----------------------------------------------------------------------------

/// <summary>
/// Resets the lexer.
/// 重置词法分析器
/// </summary>
void ResetLexer()
{
	// Set the current line of code to the new line

	g_CurrLexerState.iCurrLineIndex = 0;
	g_CurrLexerState.pCurrLine = g_SourceCode.pHead;

	// Reset the start and end of the current lexeme to the beginning of the source

	g_CurrLexerState.iCurrLexemeStart = 0;
	g_CurrLexerState.iCurrLexemeEnd = 0;

	// Reset the current operator

	g_CurrLexerState.iCurrOp = 0;
}

/// <summary>
/// Copies one lexer state structure into another.
/// 复制词法分析器状态结构到另一个地方
/// </summary>
/// <param name="pDestState"></param>
/// <param name="pSourceState"></param>
void CopyLexerState(LexerState& pDestState, LexerState& pSourceState)
{
	// Copy each field individually to ensure a safe copy
	//对每个field都进行复制
	pDestState.iCurrLineIndex = pSourceState.iCurrLineIndex;
	pDestState.pCurrLine = pSourceState.pCurrLine;
	pDestState.CurrToken = pSourceState.CurrToken;
	strcpy(pDestState.pstrCurrLexeme, pSourceState.pstrCurrLexeme);
	pDestState.iCurrLexemeStart = pSourceState.iCurrLexemeStart;
	pDestState.iCurrLexemeEnd = pSourceState.iCurrLexemeEnd;
	pDestState.iCurrOp = pSourceState.iCurrOp;
}

/// <summary>
/// Returns the index of the operator state associated with the specified character and character index.
/// 返回运算符索引
/// </summary>
/// <param name="cChar"></param>
/// <param name="iCharIndex"></param>
/// <param name="iSubStateIndex"></param>
/// <param name="iSubStateCount"></param>
/// <returns></returns>
int GetOpStateIndex(char cChar, int iCharIndex, int iSubStateIndex, int iSubStateCount)
{
	int iStartStateIndex;
	int iEndStateIndex;

	// Is the character index is zero?
	//字符索引是否是0
	if (iCharIndex == 0)
	{
		// Yes, so there's no substates to worry about
		//是，没有必要考虑后继状态
		iStartStateIndex = 0;
		iEndStateIndex = MAX_OP_STATE_COUNT;
	}
	else
	{
		//  No, so save the substate information
		//不是，保存后继状态的信息
		iStartStateIndex = iSubStateIndex;
		iEndStateIndex = iStartStateIndex + iSubStateCount;
	}

	// Loop through each possible substate and look for a match
	//循环查找每个可能的后继状态
	for (int iCurrOpStateIndex = iStartStateIndex; iCurrOpStateIndex < iEndStateIndex; ++iCurrOpStateIndex)
	{
		// Get the current state at the specified character index
		//获得指定字符索引位置的状态
		char cOpChar;
		switch (iCharIndex)
		{
		case 0:
			cOpChar = g_OpChars0[iCurrOpStateIndex].cChar;
			break;

		case 1:
			cOpChar = g_OpChars1[iCurrOpStateIndex].cChar;
			break;

		case 2:
			cOpChar = g_OpChars2[iCurrOpStateIndex].cChar;
			break;
		}

		// If the character is a match, return the index
		//匹配，返回索引
		if (cChar == cOpChar)
			return iCurrOpStateIndex;
	}

	// Return -1 if no match is found

	return -1;
}

/// <summary>
/// Determines if the specified character is an operator character.
/// 判断当前字符是否是运算符
/// </summary>
/// <param name="cChar"></param>
/// <param name="iCharIndex"></param>
/// <returns></returns>
int IsCharOpChar(char cChar, int iCharIndex)
{
	// Loop through each state in the specified character index and look for a match

	for (int iCurrOpStateIndex = 0; iCurrOpStateIndex < MAX_OP_STATE_COUNT; ++iCurrOpStateIndex)
	{
		// Get the current state at the specified character index

		char cOpChar;
		switch (iCharIndex)
		{
		case 0:
			cOpChar = g_OpChars0[iCurrOpStateIndex].cChar;
			break;

		case 1:
			cOpChar = g_OpChars1[iCurrOpStateIndex].cChar;
			break;

		case 2:
			cOpChar = g_OpChars2[iCurrOpStateIndex].cChar;
			break;
		}

		// If the character is a match, return TRUE

		if (cChar == cOpChar)
			return TRUE;
	}

	// Return FALSE if no match is found

	return FALSE;
}

/// <summary>
/// Returns the operator state associated with the specified index and state.
/// 返回运算符状态结构
/// </summary>
/// <param name="iCharIndex"></param>
/// <param name="iStateIndex"></param>
/// <returns></returns>
OpState GetOpState(int iCharIndex, int iStateIndex)
{
	OpState State;

	// Save the specified state at the specified character index
	//保存指定字符索引位置的特定状态
	switch (iCharIndex)
	{
	case 0:
		State = g_OpChars0[iStateIndex];
		break;

	case 1:
		State = g_OpChars1[iStateIndex];
		break;

	case 2:
		State = g_OpChars2[iStateIndex];
		break;
	}

	return State;
}

/// <summary>
///  Determines whether a character is a delimiter.
/// 判断字符是否是分隔符
/// </summary>
/// <param name="cChar"></param>
/// <returns></returns>
int IsCharDelim(char cChar)
{
	// Loop through each delimiter in the array and compare it to the specified character
	//循环字符与数组里的每个分隔符进行比较
	for (int iCurrDelimIndex = 0; iCurrDelimIndex < MAX_DELIM_COUNT; ++iCurrDelimIndex)
	{
		// Return TRUE if a match was found
		//匹配到
		if (cChar == cDelims[iCurrDelimIndex])
			return TRUE;
	}

	// The character is not a delimiter, so return FALSE
	//不是分隔符
	return FALSE;
}

/// <summary>
/// Returns a nonzero if the given character is whitespace, or zero otherwise.
/// 如果字符是空格或者tab返回非0值
/// </summary>
/// <param name="cChar"></param>
/// <returns></returns>
int IsCharWhitespace(char cChar)
{
	// Return true if the character is a space or tab.
	//如果当前字符是空格或者tab的话就返回true
	if (cChar == ' ' || cChar == '\t' || cChar == '\n')
		return TRUE;
	else
		return FALSE;
}

/// <summary>
/// Returns a nonzero if the given character is numeric, or zero otherwise.
/// 如果是整形则返回非0值
/// </summary>
/// <param name="cChar"></param>
/// <returns></returns>
int IsCharNumeric(char cChar)
{
	// Return true if the character is between 0 and 9 inclusive.
	//0-9中间的数字返回true
	if (cChar >= '0' && cChar <= '9')
		return TRUE;
	else
		return FALSE;
}

/// <summary>
/// Returns a nonzero if the given character is part of a valid identifier, meaning it's an alphanumeric or underscore. Zero is returned otherwise.
/// 如果字符是部分有效的标识符，返回非0
/// </summary>
/// <param name="cChar"></param>
/// <returns></returns>
int IsCharIdent(char cChar)
{
	// Return true if the character is between 0 or 9 inclusive or is an uppercase or lowercase letter or underscore
	//字符0-9之间的数字或者大写、小写字幕一级下划线 返回true
	if ((cChar >= '0' && cChar <= '9') ||
		(cChar >= 'A' && cChar <= 'Z') ||
		(cChar >= 'a' && cChar <= 'z') ||
		cChar == '_')
		return TRUE;
	else
		return FALSE;
}

/// <summary>
/// Returns the next character in the source buffer.
/// 获取下一个字符
/// </summary>
/// <returns></returns>
char GetNextChar()
{
	// Make a local copy of the string pointer, unless we're at the end of the source code
	//创建一个字符串的指针的拷贝
	char* pstrCurrLine;
	if (g_CurrLexerState.pCurrLine)
		pstrCurrLine = (char*)g_CurrLexerState.pCurrLine->pData;
	else
		return '\0';

	// If the current lexeme end index is beyond the length of the string, we're past the end of the line
	//如果当前单词的结束索引已经超出了当前行的长度，就已经到达了行尾
	if (g_CurrLexerState.iCurrLexemeEnd >= (int)strlen(pstrCurrLine))
	{
		// Move to the next node in the source code list
		//移动源代码到下一个节点
		g_CurrLexerState.pCurrLine = g_CurrLexerState.pCurrLine->pNext;

		// Is the line valid?
		//这个行是否合法
		if (g_CurrLexerState.pCurrLine)
		{
			// Yes, so move to the next line of code and reset the lexeme pointers
			//是，索引移动到下一行并设置词素指针
			pstrCurrLine = (char*)g_CurrLexerState.pCurrLine->pData;

			++g_CurrLexerState.iCurrLineIndex;
			g_CurrLexerState.iCurrLexemeStart = 0;
			g_CurrLexerState.iCurrLexemeEnd = 0;
		}
		else
		{
			// No, so return a null terminator to alert the lexer that the end of the source code has been reached
			//不合法，返回null告知词法分析到达末尾
			return '\0';
		}

	}

	// Return the character and increment the pointer
	//返回字符并且增加指针位置
	return pstrCurrLine[g_CurrLexerState.iCurrLexemeEnd++];
}

/// <summary>
/// Returns the next token in the source buffer.
/// 获得下一个属性符
/// </summary>
/// <returns></returns>
Token GetNextToken()
{
	// Save the current lexer state for future rewinding

	CopyLexerState(g_PrevLexerState, g_CurrLexerState);

	// Start the new lexeme at the end of the last one
	//从上一个结束的位置开始一个新的词素
	g_CurrLexerState.iCurrLexemeStart = g_CurrLexerState.iCurrLexemeEnd;

	// Set the initial state to the start state
	//设置初始状态为开始状态
	int iCurrLexState = LEX_STATE_START;

	// Set the current operator state

	int iCurrOpCharIndex = 0;
	int iCurrOpStateIndex = 0;
	OpState CurrOpState;

	// Flag to determine when the lexeme is done

	int iLexemeDone = FALSE;

	//Loop until a token is completed
	//

	// Current character
	//当前字符
	char cCurrChar;

	// Current position in the lexeme string buffer
	//词素字符缓冲区的当前位置
	int iNextLexemeCharIndex = 0;

	// Should the current character be included in the lexeme?
	//当前字符是否应当被包含在词素中
	int iAddCurrChar;

	//! 状态机设计
	// Begin the loop
	//开始循环
	while (TRUE)
	{
		// Read the next character, and exit if the end of the source has been reached
		//读取下一个字符，到达代码结尾就退出
		cCurrChar = GetNextChar();
		if (cCurrChar == '\0') //结尾 
			break;

		// Assume the character will be added to the lexeme
		//假设这个字符会被加入到词素中
		iAddCurrChar = TRUE;

		// Depending on the current state of the lexer, handle the incoming character
		//根据当前词法分析器状态处理 输入的字符
		switch (iCurrLexState)
		{
			// If an unknown state occurs, the token is invalid, so exit
			//
		case LEX_STATE_UNKNOWN:

			iLexemeDone = TRUE;

			break;

			// The start state
			//开始状态
		case LEX_STATE_START:

			// Just loop past whitespace, and don't add it to the lexeme
			//循环跳过空格，并且不要将它们加入词素
			if (IsCharWhitespace(cCurrChar))
			{
				++g_CurrLexerState.iCurrLexemeStart;//索引增加
				iAddCurrChar = FALSE;//不加入
			}

			// An integer is starting
			//是否是整形值
			else if (IsCharNumeric(cCurrChar))
			{
				iCurrLexState = LEX_STATE_INT;
			}

			// A float is starting
			//是否是浮点值
			else if (cCurrChar == '.')
			{
				iCurrLexState = LEX_STATE_FLOAT;
			}

			// An identifier is starting
			//是否是标识符 从最开始判断这个是不行的。因为标识符可能包含0-9
			else if (IsCharIdent(cCurrChar))
			{
				iCurrLexState = LEX_STATE_IDENT;
			}

			// A delimiter has been read
			//是否是分隔符
			else if (IsCharDelim(cCurrChar))
			{
				iCurrLexState = LEX_STATE_DELIM;
			}

			// An operator is starting
			//是否是运算符
			else if (IsCharOpChar(cCurrChar, 0))
			{
				// Get the index of the initial operand state
				//获得最初运算符状态的索引
				iCurrOpStateIndex = GetOpStateIndex(cCurrChar, 0, 0, 0);
				if (iCurrOpStateIndex == -1)
					return TOKEN_TYPE_INVALID;

				// Get the full state structure
				//获取整个状态的结构
				CurrOpState = GetOpState(0, iCurrOpStateIndex);

				// Move to the next character in the operator (1)
				//移动到运算符的下一个字符
				iCurrOpCharIndex = 1;

				// Set the current operator
				//设置当前运算符
				g_CurrLexerState.iCurrOp = CurrOpState.iIndex;

				iCurrLexState = LEX_STATE_OP;
			}

			// A string is starting, but don't add the opening quote to the lexeme
			//左引号，一个新的字符串开始，不要添加到词素里
			else if (cCurrChar == '"')
			{
				iAddCurrChar = FALSE;
				iCurrLexState = LEX_STATE_STRING;
			}

			// It's invalid
			//非法字符
			else
				iCurrLexState = LEX_STATE_UNKNOWN;

			break;

			// Integer
			//整形状态
		case LEX_STATE_INT:

			// If a numeric is read, keep the state as-is
			//如果读取到整形，保持这个状态
			if (IsCharNumeric(cCurrChar))
			{
				iCurrLexState = LEX_STATE_INT;
			}

			// If a radix point is read, the numeric is really a float
			//如果读到了小数点，就是浮点值
			else if (cCurrChar == '.')
			{
				iCurrLexState = LEX_STATE_FLOAT;
			}

			// If whitespace or a delimiter is read, the lexeme is done
			//如果读到空格代表词素已经识别完成
			else if (IsCharWhitespace(cCurrChar) || IsCharDelim(cCurrChar))
			{
				iAddCurrChar = FALSE;
				iLexemeDone = TRUE;
			}

			// Anything else is invalid
			//其它都是非法的
			else
				iCurrLexState = LEX_STATE_UNKNOWN;

			break;

			// Floating-point
			//浮点状态
		case LEX_STATE_FLOAT:

			// If a numeric is read, keep the state as-is
			//读到数字保持当前状态
			if (IsCharNumeric(cCurrChar))
			{
				iCurrLexState = LEX_STATE_FLOAT;
			}

			// If whitespace or a delimiter is read, the lexeme is done
			//读到空格，词素识别完成
			else if (IsCharWhitespace(cCurrChar) || IsCharDelim(cCurrChar))
			{
				iLexemeDone = TRUE;
				iAddCurrChar = FALSE;
			}

			// Anything else is invalid
			//其他都是非法的
			else
				iCurrLexState = LEX_STATE_UNKNOWN;

			break;

			// Identifier
			//标识符 状态
		case LEX_STATE_IDENT:

			// If an identifier character is read, keep the state as-is
			//如果读到的是一个标识符，保持状态
			if (IsCharIdent(cCurrChar))
			{
				iCurrLexState = LEX_STATE_IDENT;
			}

			// If whitespace or a delimiter is read, the lexeme is done
			//读到空格，词素结束
			else if (IsCharWhitespace(cCurrChar) || IsCharDelim(cCurrChar))
			{
				iAddCurrChar = FALSE;
				iLexemeDone = TRUE;
			}

			// Anything else is invalid
			//其他东西都是非法的
			else
				iCurrLexState = LEX_STATE_UNKNOWN;

			break;

			// Operator
			//运算符状态
		case LEX_STATE_OP:

			// If the current character within the operator has no substates, we're done
			//如果运算符中的当前字符没有后继状态，工作完成
			if (CurrOpState.iSubStateCount == 0)
			{
				iAddCurrChar = FALSE;
				iLexemeDone = TRUE;
				break;
			}

			// Otherwise, find out if the new character is a possible substate
			//否则确定新字符是否可能是后继状态
			if (IsCharOpChar(cCurrChar, iCurrOpCharIndex))
			{
				// Get the index of the next substate
				//获取后继状态的索引
				iCurrOpStateIndex = GetOpStateIndex(cCurrChar, iCurrOpCharIndex, CurrOpState.iSubStateIndex, CurrOpState.iSubStateCount);
				if (iCurrOpStateIndex == -1)
				{
					iCurrLexState = LEX_STATE_UNKNOWN;
				}
				else
				{
					// Get the next operator structure
					//获取下一个运算符的结构
					CurrOpState = GetOpState(iCurrOpCharIndex, iCurrOpStateIndex);

					// Move to the next character in the operator
					//开始处理运算符中下一个字符
					++iCurrOpCharIndex;

					// Set the current operator
					//设置当前运算符
					g_CurrLexerState.iCurrOp = CurrOpState.iIndex;
				}
			}

			// If not, the lexeme is done
			//如果新字符不符合，词素分析完成
			else
			{
				iAddCurrChar = FALSE;
				iLexemeDone = TRUE;
			}

			break;

			// Delimiter
			//分隔符状态
		case LEX_STATE_DELIM:

			// Don't add whatever comes after the delimiter to the lexeme, because it's done
			//不管分隔符后面是什么都不加入到词素中，因为它已经结束了
			iAddCurrChar = FALSE;
			iLexemeDone = TRUE;

			break;

			// String
			//字符串状态
		case LEX_STATE_STRING:

			// If the current character is a closing quote, finish the lexeme
			//如果当前字符串是闭引号，完成分析
			if (cCurrChar == '"')
			{
				iAddCurrChar = FALSE;
				iCurrLexState = LEX_STATE_STRING_CLOSE_QUOTE;
			}

			// If it's a newline, the string token is invalid

			else if (cCurrChar == '\n')
			{
				iAddCurrChar = FALSE;
				iCurrLexState = LEX_STATE_UNKNOWN;
			}

			// If it's an escape sequence, switch to the escape state and don't add the backslash to the lexeme
			//如果是转义符就迁移到转义状态并且不要将反斜线加入到词素中
			else if (cCurrChar == '\\')
			{
				iAddCurrChar = FALSE;
				iCurrLexState = LEX_STATE_STRING_ESCAPE;
			}

			// Anything else gets added to the string
			//将其他任何东西都添加到字符串中
			break;

			// Escape sequence
			//转义状态
		case LEX_STATE_STRING_ESCAPE:

			// Immediately switch back to the string state, now that the character's been added
			//添加字符后马上回到字符串状态
			iCurrLexState = LEX_STATE_STRING;

			break;

			// String closing quote
			//字符串闭合引号状态
		case LEX_STATE_STRING_CLOSE_QUOTE:

			// Finish the string lexeme
			//完成分析
			iAddCurrChar = FALSE;
			iLexemeDone = TRUE;

			break;
		}

		//! 分析完一个字符，准备下一个
		// Add the next character to the lexeme and increment the index
		//将下一个字符加入到词素并增加索引
		if (iAddCurrChar)
		{
			g_CurrLexerState.pstrCurrLexeme[iNextLexemeCharIndex] = cCurrChar;
			++iNextLexemeCharIndex;
		}

		// If the lexeme is complete, exit the loop
		//单词分析完，跳出循环
		if (iLexemeDone)
			break;
	}

	// Complete the lexeme string
	//完成词素字符串
	g_CurrLexerState.pstrCurrLexeme[iNextLexemeCharIndex] = '\0';

	// Retract the lexeme end index by one
	//词素结尾索引回退一位
	--g_CurrLexerState.iCurrLexemeEnd;

	// Determine the token type
	//! 决定属性符类型
	Token TokenType;
	switch (iCurrLexState)
	{
		// Unknown
		//未知
	case LEX_STATE_UNKNOWN:
		TokenType = TOKEN_TYPE_INVALID;
		break;

		// Integer
		//整型
	case LEX_STATE_INT:
		TokenType = TOKEN_TYPE_INT;
		break;

		// Float
		//浮点型
	case LEX_STATE_FLOAT:
		TokenType = TOKEN_TYPE_FLOAT;
		break;

		// Identifier/Reserved Word
		//标识符或者保留字
	case LEX_STATE_IDENT:

		// Set the token type to identifier in case none of the reserved words match
		//
		TokenType = TOKEN_TYPE_IDENT;

		//! 决定标识符是否是保留字
		//! 进行字符串比较

		// var/var []
		if (_stricmp(g_CurrLexerState.pstrCurrLexeme, "var") == 0)
			TokenType = TOKEN_TYPE_RSRVD_VAR;

		// true
		if (_stricmp(g_CurrLexerState.pstrCurrLexeme, "true") == 0)
			TokenType = TOKEN_TYPE_RSRVD_TRUE;

		// false
		if (_stricmp(g_CurrLexerState.pstrCurrLexeme, "false") == 0)
			TokenType = TOKEN_TYPE_RSRVD_FALSE;

		// if
		if (_stricmp(g_CurrLexerState.pstrCurrLexeme, "if") == 0)
			TokenType = TOKEN_TYPE_RSRVD_IF;

		// else
		if (_stricmp(g_CurrLexerState.pstrCurrLexeme, "else") == 0)
			TokenType = TOKEN_TYPE_RSRVD_ELSE;

		// break
		if (_stricmp(g_CurrLexerState.pstrCurrLexeme, "break") == 0)
			TokenType = TOKEN_TYPE_RSRVD_BREAK;

		// continue
		if (_stricmp(g_CurrLexerState.pstrCurrLexeme, "continue") == 0)
			TokenType = TOKEN_TYPE_RSRVD_CONTINUE;

		// for
		if (_stricmp(g_CurrLexerState.pstrCurrLexeme, "for") == 0)
			TokenType = TOKEN_TYPE_RSRVD_FOR;

		// while
		if (_stricmp(g_CurrLexerState.pstrCurrLexeme, "while") == 0)
			TokenType = TOKEN_TYPE_RSRVD_WHILE;

		// func
		if (_stricmp(g_CurrLexerState.pstrCurrLexeme, "func") == 0)
			TokenType = TOKEN_TYPE_RSRVD_FUNC;

		// return
		if (_stricmp(g_CurrLexerState.pstrCurrLexeme, "return") == 0)
			TokenType = TOKEN_TYPE_RSRVD_RETURN;

		// host
		if (_stricmp(g_CurrLexerState.pstrCurrLexeme, "host") == 0)
			TokenType = TOKEN_TYPE_RSRVD_HOST;

		break;

		// Delimiter
		//分隔符
	case LEX_STATE_DELIM:

		// Determine which delimiter was found
		//判断是发现了哪一个分隔符
		switch (g_CurrLexerState.pstrCurrLexeme[0])
		{
		case ',':
			TokenType = TOKEN_TYPE_DELIM_COMMA;
			break;

		case '(':
			TokenType = TOKEN_TYPE_DELIM_OPEN_PAREN;
			break;

		case ')':
			TokenType = TOKEN_TYPE_DELIM_CLOSE_PAREN;
			break;

		case '[':
			TokenType = TOKEN_TYPE_DELIM_OPEN_BRACE;
			break;

		case ']':
			TokenType = TOKEN_TYPE_DELIM_CLOSE_BRACE;
			break;

		case '{':
			TokenType = TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE;
			break;

		case '}':
			TokenType = TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE;
			break;

		case ';':
			TokenType = TOKEN_TYPE_DELIM_SEMICOLON;
			break;
		}

		break;

		// Operators

	case LEX_STATE_OP:
		TokenType = TOKEN_TYPE_OP;
		break;

		// Strings

	case LEX_STATE_STRING_CLOSE_QUOTE:
		TokenType = TOKEN_TYPE_STRING;
		break;

		// All that's left is whitespace, which means the end of the stream
		//其他所有都是空格，意味流的结束
	default:
		TokenType = TOKEN_TYPE_END_OF_STREAM;
	}

	// Return the token type and set the global copy
	//返回属性符类型，并且设置一份全局拷贝
	g_CurrLexerState.CurrToken = TokenType;
	return TokenType;
}

/// <summary>
/// Moves the lexer back one token.
/// 让词法分析器回退一个属性符
/// </summary>
void RewindTokenStream()
{
	CopyLexerState(g_CurrLexerState, g_PrevLexerState);
}

/// <summary>
/// Returns the current token.
/// 获取当前属性符
/// </summary>
/// <returns></returns>
Token GetCurrToken()
{
	return g_CurrLexerState.CurrToken;
}

/// <summary>
/// Returns a pointer to the current lexeme.
/// 返回指向当前词素的指针
/// </summary>
/// <returns></returns>
char* GetCurrLexeme()
{
	//返回全局变量
	return g_CurrLexerState.pstrCurrLexeme;
}

/// <summary>
///  Makes a physical copy of the current lexeme into the specified string buffer.
/// 复制当前词素
/// </summary>
/// <param name="pstrBuffer"></param>
void CopyCurrLexeme(char* pstrBuffer)
{
	strcpy(pstrBuffer, g_CurrLexerState.pstrCurrLexeme);
}

/// <summary>
/// Returns the current operator.
/// 获取当前运算符
/// </summary>
/// <returns></returns>
int GetCurrOp()
{
	return g_CurrLexerState.iCurrOp;
}

/// <summary>
/// Returns the first character of the next token.
/// 预读取，查看流中下一个属性符的第一个字符
/// </summary>
/// <returns></returns>
char GetLookAheadChar()
{
	// Save the current lexer state
	//保存当前词法分析器状态
	LexerState PrevLexerState;
	CopyLexerState(PrevLexerState, g_CurrLexerState);

	// Skip any whitespace that may exist and return the first non-whitespace character
	//忽略空格，返回第一个非空格字符
	char cCurrChar;
	while (TRUE)
	{
		cCurrChar = GetNextChar();
		if (!IsCharWhitespace(cCurrChar))
			break;
	}

	// Restore the lexer state
	//保存状态
	CopyLexerState(g_CurrLexerState, PrevLexerState);

	// Return the look-ahead character
	//返回
	return cCurrChar;
}

/// <summary>
/// Returns a pointer to the current source line string.
/// 返回当前行指针
/// </summary>
/// <returns></returns>
char* GetCurrSourceLine()
{
	if (g_CurrLexerState.pCurrLine)
		return (char*)g_CurrLexerState.pCurrLine->pData;
	else
		return NULL;
}

/// <summary>
/// Returns the current source line number.
/// 返回当前行索引
/// </summary>
/// <returns></returns>
int GetCurrSourceLineIndex()
{
	return g_CurrLexerState.iCurrLineIndex;
}

/// <summary>
/// Returns the pointer to the start of the current lexeme
/// 返回当前词素开始的指针
/// </summary>
/// <returns></returns>
int GetLexemeStartIndex()
{
	return g_CurrLexerState.iCurrLexemeStart;
}
