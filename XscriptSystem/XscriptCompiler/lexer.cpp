//Lexical analyzer module
//�ʷ�������ģ��

#include "lexer.h"

// ---- Globals -------------------------------------------------------------------------------

	// ---- Lexer -----------------------------------------------------------------------------

LexerState g_CurrLexerState;                    // The current lexer state ��ǰ�ʷ�������״̬
LexerState g_PrevLexerState;                    // The previous lexer state (used for rewinding the token stream) ǰһ���ʷ�������״̬,���ڻ������Է���

// ---- Operators -------------------------------------------------------------------------

//! First operator characters ������ַ����еĵ�һ���ַ�
OpState g_OpChars0[MAX_OP_STATE_COUNT] =
{
	//״̬ ���״̬��������� ���״̬�ĸ��� ���������
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

//! Second operator characters ������ַ����еĵڶ����ַ�
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

//! Third operator characters ������ַ����еĵ������ַ�
OpState g_OpChars2[MAX_OP_STATE_COUNT] =
{
	{ '=', 0, 0, 33 }, { '=', 0, 0, 34 } // <<=, >>=
};

//! Delimiters �ָ�������
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
/// ���ôʷ�������
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
/// ���ƴʷ�������״̬�ṹ����һ���ط�
/// </summary>
/// <param name="pDestState"></param>
/// <param name="pSourceState"></param>
void CopyLexerState(LexerState& pDestState, LexerState& pSourceState)
{
	// Copy each field individually to ensure a safe copy
	//��ÿ��field�����и���
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
/// �������������
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
	//�ַ������Ƿ���0
	if (iCharIndex == 0)
	{
		// Yes, so there's no substates to worry about
		//�ǣ�û�б�Ҫ���Ǻ��״̬
		iStartStateIndex = 0;
		iEndStateIndex = MAX_OP_STATE_COUNT;
	}
	else
	{
		//  No, so save the substate information
		//���ǣ�������״̬����Ϣ
		iStartStateIndex = iSubStateIndex;
		iEndStateIndex = iStartStateIndex + iSubStateCount;
	}

	// Loop through each possible substate and look for a match
	//ѭ������ÿ�����ܵĺ��״̬
	for (int iCurrOpStateIndex = iStartStateIndex; iCurrOpStateIndex < iEndStateIndex; ++iCurrOpStateIndex)
	{
		// Get the current state at the specified character index
		//���ָ���ַ�����λ�õ�״̬
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
		//ƥ�䣬��������
		if (cChar == cOpChar)
			return iCurrOpStateIndex;
	}

	// Return -1 if no match is found

	return -1;
}

/// <summary>
/// Determines if the specified character is an operator character.
/// �жϵ�ǰ�ַ��Ƿ��������
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
/// ���������״̬�ṹ
/// </summary>
/// <param name="iCharIndex"></param>
/// <param name="iStateIndex"></param>
/// <returns></returns>
OpState GetOpState(int iCharIndex, int iStateIndex)
{
	OpState State;

	// Save the specified state at the specified character index
	//����ָ���ַ�����λ�õ��ض�״̬
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
/// �ж��ַ��Ƿ��Ƿָ���
/// </summary>
/// <param name="cChar"></param>
/// <returns></returns>
int IsCharDelim(char cChar)
{
	// Loop through each delimiter in the array and compare it to the specified character
	//ѭ���ַ����������ÿ���ָ������бȽ�
	for (int iCurrDelimIndex = 0; iCurrDelimIndex < MAX_DELIM_COUNT; ++iCurrDelimIndex)
	{
		// Return TRUE if a match was found
		//ƥ�䵽
		if (cChar == cDelims[iCurrDelimIndex])
			return TRUE;
	}

	// The character is not a delimiter, so return FALSE
	//���Ƿָ���
	return FALSE;
}

/// <summary>
/// Returns a nonzero if the given character is whitespace, or zero otherwise.
/// ����ַ��ǿո����tab���ط�0ֵ
/// </summary>
/// <param name="cChar"></param>
/// <returns></returns>
int IsCharWhitespace(char cChar)
{
	// Return true if the character is a space or tab.
	//�����ǰ�ַ��ǿո����tab�Ļ��ͷ���true
	if (cChar == ' ' || cChar == '\t' || cChar == '\n')
		return TRUE;
	else
		return FALSE;
}

/// <summary>
/// Returns a nonzero if the given character is numeric, or zero otherwise.
/// ����������򷵻ط�0ֵ
/// </summary>
/// <param name="cChar"></param>
/// <returns></returns>
int IsCharNumeric(char cChar)
{
	// Return true if the character is between 0 and 9 inclusive.
	//0-9�м�����ַ���true
	if (cChar >= '0' && cChar <= '9')
		return TRUE;
	else
		return FALSE;
}

/// <summary>
/// Returns a nonzero if the given character is part of a valid identifier, meaning it's an alphanumeric or underscore. Zero is returned otherwise.
/// ����ַ��ǲ�����Ч�ı�ʶ�������ط�0
/// </summary>
/// <param name="cChar"></param>
/// <returns></returns>
int IsCharIdent(char cChar)
{
	// Return true if the character is between 0 or 9 inclusive or is an uppercase or lowercase letter or underscore
	//�ַ�0-9֮������ֻ��ߴ�д��Сд��Ļһ���»��� ����true
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
/// ��ȡ��һ���ַ�
/// </summary>
/// <returns></returns>
char GetNextChar()
{
	// Make a local copy of the string pointer, unless we're at the end of the source code
	//����һ���ַ�����ָ��Ŀ���
	char* pstrCurrLine;
	if (g_CurrLexerState.pCurrLine)
		pstrCurrLine = (char*)g_CurrLexerState.pCurrLine->pData;
	else
		return '\0';

	// If the current lexeme end index is beyond the length of the string, we're past the end of the line
	//�����ǰ���ʵĽ��������Ѿ������˵�ǰ�еĳ��ȣ����Ѿ���������β
	if (g_CurrLexerState.iCurrLexemeEnd >= (int)strlen(pstrCurrLine))
	{
		// Move to the next node in the source code list
		//�ƶ�Դ���뵽��һ���ڵ�
		g_CurrLexerState.pCurrLine = g_CurrLexerState.pCurrLine->pNext;

		// Is the line valid?
		//������Ƿ�Ϸ�
		if (g_CurrLexerState.pCurrLine)
		{
			// Yes, so move to the next line of code and reset the lexeme pointers
			//�ǣ������ƶ�����һ�в����ô���ָ��
			pstrCurrLine = (char*)g_CurrLexerState.pCurrLine->pData;

			++g_CurrLexerState.iCurrLineIndex;
			g_CurrLexerState.iCurrLexemeStart = 0;
			g_CurrLexerState.iCurrLexemeEnd = 0;
		}
		else
		{
			// No, so return a null terminator to alert the lexer that the end of the source code has been reached
			//���Ϸ�������null��֪�ʷ���������ĩβ
			return '\0';
		}

	}

	// Return the character and increment the pointer
	//�����ַ���������ָ��λ��
	return pstrCurrLine[g_CurrLexerState.iCurrLexemeEnd++];
}

/// <summary>
/// Returns the next token in the source buffer.
/// �����һ�����Է�
/// </summary>
/// <returns></returns>
Token GetNextToken()
{
	// Save the current lexer state for future rewinding

	CopyLexerState(g_PrevLexerState, g_CurrLexerState);

	// Start the new lexeme at the end of the last one
	//����һ��������λ�ÿ�ʼһ���µĴ���
	g_CurrLexerState.iCurrLexemeStart = g_CurrLexerState.iCurrLexemeEnd;

	// Set the initial state to the start state
	//���ó�ʼ״̬Ϊ��ʼ״̬
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
	//��ǰ�ַ�
	char cCurrChar;

	// Current position in the lexeme string buffer
	//�����ַ��������ĵ�ǰλ��
	int iNextLexemeCharIndex = 0;

	// Should the current character be included in the lexeme?
	//��ǰ�ַ��Ƿ�Ӧ���������ڴ�����
	int iAddCurrChar;

	//! ״̬�����
	// Begin the loop
	//��ʼѭ��
	while (TRUE)
	{
		// Read the next character, and exit if the end of the source has been reached
		//��ȡ��һ���ַ�����������β���˳�
		cCurrChar = GetNextChar();
		if (cCurrChar == '\0') //��β 
			break;

		// Assume the character will be added to the lexeme
		//��������ַ��ᱻ���뵽������
		iAddCurrChar = TRUE;

		// Depending on the current state of the lexer, handle the incoming character
		//���ݵ�ǰ�ʷ�������״̬���� ������ַ�
		switch (iCurrLexState)
		{
			// If an unknown state occurs, the token is invalid, so exit
			//
		case LEX_STATE_UNKNOWN:

			iLexemeDone = TRUE;

			break;

			// The start state
			//��ʼ״̬
		case LEX_STATE_START:

			// Just loop past whitespace, and don't add it to the lexeme
			//ѭ�������ո񣬲��Ҳ�Ҫ�����Ǽ������
			if (IsCharWhitespace(cCurrChar))
			{
				++g_CurrLexerState.iCurrLexemeStart;//��������
				iAddCurrChar = FALSE;//������
			}

			// An integer is starting
			//�Ƿ�������ֵ
			else if (IsCharNumeric(cCurrChar))
			{
				iCurrLexState = LEX_STATE_INT;
			}

			// A float is starting
			//�Ƿ��Ǹ���ֵ
			else if (cCurrChar == '.')
			{
				iCurrLexState = LEX_STATE_FLOAT;
			}

			// An identifier is starting
			//�Ƿ��Ǳ�ʶ�� ���ʼ�ж�����ǲ��еġ���Ϊ��ʶ�����ܰ���0-9
			else if (IsCharIdent(cCurrChar))
			{
				iCurrLexState = LEX_STATE_IDENT;
			}

			// A delimiter has been read
			//�Ƿ��Ƿָ���
			else if (IsCharDelim(cCurrChar))
			{
				iCurrLexState = LEX_STATE_DELIM;
			}

			// An operator is starting
			//�Ƿ��������
			else if (IsCharOpChar(cCurrChar, 0))
			{
				// Get the index of the initial operand state
				//�����������״̬������
				iCurrOpStateIndex = GetOpStateIndex(cCurrChar, 0, 0, 0);
				if (iCurrOpStateIndex == -1)
					return TOKEN_TYPE_INVALID;

				// Get the full state structure
				//��ȡ����״̬�Ľṹ
				CurrOpState = GetOpState(0, iCurrOpStateIndex);

				// Move to the next character in the operator (1)
				//�ƶ������������һ���ַ�
				iCurrOpCharIndex = 1;

				// Set the current operator
				//���õ�ǰ�����
				g_CurrLexerState.iCurrOp = CurrOpState.iIndex;

				iCurrLexState = LEX_STATE_OP;
			}

			// A string is starting, but don't add the opening quote to the lexeme
			//�����ţ�һ���µ��ַ�����ʼ����Ҫ��ӵ�������
			else if (cCurrChar == '"')
			{
				iAddCurrChar = FALSE;
				iCurrLexState = LEX_STATE_STRING;
			}

			// It's invalid
			//�Ƿ��ַ�
			else
				iCurrLexState = LEX_STATE_UNKNOWN;

			break;

			// Integer
			//����״̬
		case LEX_STATE_INT:

			// If a numeric is read, keep the state as-is
			//�����ȡ�����Σ��������״̬
			if (IsCharNumeric(cCurrChar))
			{
				iCurrLexState = LEX_STATE_INT;
			}

			// If a radix point is read, the numeric is really a float
			//���������С���㣬���Ǹ���ֵ
			else if (cCurrChar == '.')
			{
				iCurrLexState = LEX_STATE_FLOAT;
			}

			// If whitespace or a delimiter is read, the lexeme is done
			//��������ո��������Ѿ�ʶ�����
			else if (IsCharWhitespace(cCurrChar) || IsCharDelim(cCurrChar))
			{
				iAddCurrChar = FALSE;
				iLexemeDone = TRUE;
			}

			// Anything else is invalid
			//�������ǷǷ���
			else
				iCurrLexState = LEX_STATE_UNKNOWN;

			break;

			// Floating-point
			//����״̬
		case LEX_STATE_FLOAT:

			// If a numeric is read, keep the state as-is
			//�������ֱ��ֵ�ǰ״̬
			if (IsCharNumeric(cCurrChar))
			{
				iCurrLexState = LEX_STATE_FLOAT;
			}

			// If whitespace or a delimiter is read, the lexeme is done
			//�����ո񣬴���ʶ�����
			else if (IsCharWhitespace(cCurrChar) || IsCharDelim(cCurrChar))
			{
				iLexemeDone = TRUE;
				iAddCurrChar = FALSE;
			}

			// Anything else is invalid
			//�������ǷǷ���
			else
				iCurrLexState = LEX_STATE_UNKNOWN;

			break;

			// Identifier
			//��ʶ�� ״̬
		case LEX_STATE_IDENT:

			// If an identifier character is read, keep the state as-is
			//�����������һ����ʶ��������״̬
			if (IsCharIdent(cCurrChar))
			{
				iCurrLexState = LEX_STATE_IDENT;
			}

			// If whitespace or a delimiter is read, the lexeme is done
			//�����ո񣬴��ؽ���
			else if (IsCharWhitespace(cCurrChar) || IsCharDelim(cCurrChar))
			{
				iAddCurrChar = FALSE;
				iLexemeDone = TRUE;
			}

			// Anything else is invalid
			//�����������ǷǷ���
			else
				iCurrLexState = LEX_STATE_UNKNOWN;

			break;

			// Operator
			//�����״̬
		case LEX_STATE_OP:

			// If the current character within the operator has no substates, we're done
			//���������еĵ�ǰ�ַ�û�к��״̬���������
			if (CurrOpState.iSubStateCount == 0)
			{
				iAddCurrChar = FALSE;
				iLexemeDone = TRUE;
				break;
			}

			// Otherwise, find out if the new character is a possible substate
			//����ȷ�����ַ��Ƿ�����Ǻ��״̬
			if (IsCharOpChar(cCurrChar, iCurrOpCharIndex))
			{
				// Get the index of the next substate
				//��ȡ���״̬������
				iCurrOpStateIndex = GetOpStateIndex(cCurrChar, iCurrOpCharIndex, CurrOpState.iSubStateIndex, CurrOpState.iSubStateCount);
				if (iCurrOpStateIndex == -1)
				{
					iCurrLexState = LEX_STATE_UNKNOWN;
				}
				else
				{
					// Get the next operator structure
					//��ȡ��һ��������Ľṹ
					CurrOpState = GetOpState(iCurrOpCharIndex, iCurrOpStateIndex);

					// Move to the next character in the operator
					//��ʼ�������������һ���ַ�
					++iCurrOpCharIndex;

					// Set the current operator
					//���õ�ǰ�����
					g_CurrLexerState.iCurrOp = CurrOpState.iIndex;
				}
			}

			// If not, the lexeme is done
			//������ַ������ϣ����ط������
			else
			{
				iAddCurrChar = FALSE;
				iLexemeDone = TRUE;
			}

			break;

			// Delimiter
			//�ָ���״̬
		case LEX_STATE_DELIM:

			// Don't add whatever comes after the delimiter to the lexeme, because it's done
			//���ָܷ���������ʲô�������뵽�����У���Ϊ���Ѿ�������
			iAddCurrChar = FALSE;
			iLexemeDone = TRUE;

			break;

			// String
			//�ַ���״̬
		case LEX_STATE_STRING:

			// If the current character is a closing quote, finish the lexeme
			//�����ǰ�ַ����Ǳ����ţ���ɷ���
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
			//�����ת�����Ǩ�Ƶ�ת��״̬���Ҳ�Ҫ����б�߼��뵽������
			else if (cCurrChar == '\\')
			{
				iAddCurrChar = FALSE;
				iCurrLexState = LEX_STATE_STRING_ESCAPE;
			}

			// Anything else gets added to the string
			//�������κζ�������ӵ��ַ�����
			break;

			// Escape sequence
			//ת��״̬
		case LEX_STATE_STRING_ESCAPE:

			// Immediately switch back to the string state, now that the character's been added
			//����ַ������ϻص��ַ���״̬
			iCurrLexState = LEX_STATE_STRING;

			break;

			// String closing quote
			//�ַ����պ�����״̬
		case LEX_STATE_STRING_CLOSE_QUOTE:

			// Finish the string lexeme
			//��ɷ���
			iAddCurrChar = FALSE;
			iLexemeDone = TRUE;

			break;
		}

		//! ������һ���ַ���׼����һ��
		// Add the next character to the lexeme and increment the index
		//����һ���ַ����뵽���ز���������
		if (iAddCurrChar)
		{
			g_CurrLexerState.pstrCurrLexeme[iNextLexemeCharIndex] = cCurrChar;
			++iNextLexemeCharIndex;
		}

		// If the lexeme is complete, exit the loop
		//���ʷ����꣬����ѭ��
		if (iLexemeDone)
			break;
	}

	// Complete the lexeme string
	//��ɴ����ַ���
	g_CurrLexerState.pstrCurrLexeme[iNextLexemeCharIndex] = '\0';

	// Retract the lexeme end index by one
	//���ؽ�β��������һλ
	--g_CurrLexerState.iCurrLexemeEnd;

	// Determine the token type
	//! �������Է�����
	Token TokenType;
	switch (iCurrLexState)
	{
		// Unknown
		//δ֪
	case LEX_STATE_UNKNOWN:
		TokenType = TOKEN_TYPE_INVALID;
		break;

		// Integer
		//����
	case LEX_STATE_INT:
		TokenType = TOKEN_TYPE_INT;
		break;

		// Float
		//������
	case LEX_STATE_FLOAT:
		TokenType = TOKEN_TYPE_FLOAT;
		break;

		// Identifier/Reserved Word
		//��ʶ�����߱�����
	case LEX_STATE_IDENT:

		// Set the token type to identifier in case none of the reserved words match
		//
		TokenType = TOKEN_TYPE_IDENT;

		//! ������ʶ���Ƿ��Ǳ�����
		//! �����ַ����Ƚ�

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
		//�ָ���
	case LEX_STATE_DELIM:

		// Determine which delimiter was found
		//�ж��Ƿ�������һ���ָ���
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
		//�������ж��ǿո���ζ���Ľ���
	default:
		TokenType = TOKEN_TYPE_END_OF_STREAM;
	}

	// Return the token type and set the global copy
	//�������Է����ͣ���������һ��ȫ�ֿ���
	g_CurrLexerState.CurrToken = TokenType;
	return TokenType;
}

/// <summary>
/// Moves the lexer back one token.
/// �ôʷ�����������һ�����Է�
/// </summary>
void RewindTokenStream()
{
	CopyLexerState(g_CurrLexerState, g_PrevLexerState);
}

/// <summary>
/// Returns the current token.
/// ��ȡ��ǰ���Է�
/// </summary>
/// <returns></returns>
Token GetCurrToken()
{
	return g_CurrLexerState.CurrToken;
}

/// <summary>
/// Returns a pointer to the current lexeme.
/// ����ָ��ǰ���ص�ָ��
/// </summary>
/// <returns></returns>
char* GetCurrLexeme()
{
	//����ȫ�ֱ���
	return g_CurrLexerState.pstrCurrLexeme;
}

/// <summary>
///  Makes a physical copy of the current lexeme into the specified string buffer.
/// ���Ƶ�ǰ����
/// </summary>
/// <param name="pstrBuffer"></param>
void CopyCurrLexeme(char* pstrBuffer)
{
	strcpy(pstrBuffer, g_CurrLexerState.pstrCurrLexeme);
}

/// <summary>
/// Returns the current operator.
/// ��ȡ��ǰ�����
/// </summary>
/// <returns></returns>
int GetCurrOp()
{
	return g_CurrLexerState.iCurrOp;
}

/// <summary>
/// Returns the first character of the next token.
/// Ԥ��ȡ���鿴������һ�����Է��ĵ�һ���ַ�
/// </summary>
/// <returns></returns>
char GetLookAheadChar()
{
	// Save the current lexer state
	//���浱ǰ�ʷ�������״̬
	LexerState PrevLexerState;
	CopyLexerState(PrevLexerState, g_CurrLexerState);

	// Skip any whitespace that may exist and return the first non-whitespace character
	//���Կո񣬷��ص�һ���ǿո��ַ�
	char cCurrChar;
	while (TRUE)
	{
		cCurrChar = GetNextChar();
		if (!IsCharWhitespace(cCurrChar))
			break;
	}

	// Restore the lexer state
	//����״̬
	CopyLexerState(g_CurrLexerState, PrevLexerState);

	// Return the look-ahead character
	//����
	return cCurrChar;
}

/// <summary>
/// Returns a pointer to the current source line string.
/// ���ص�ǰ��ָ��
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
/// ���ص�ǰ������
/// </summary>
/// <returns></returns>
int GetCurrSourceLineIndex()
{
	return g_CurrLexerState.iCurrLineIndex;
}

/// <summary>
/// Returns the pointer to the start of the current lexeme
/// ���ص�ǰ���ؿ�ʼ��ָ��
/// </summary>
/// <returns></returns>
int GetLexemeStartIndex()
{
	return g_CurrLexerState.iCurrLexemeStart;
}
