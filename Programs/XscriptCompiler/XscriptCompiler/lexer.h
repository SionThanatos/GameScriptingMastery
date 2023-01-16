//Lexical analyzer module header
//词法分析器模块

#ifndef XSC_LEXER
#define XSC_LEXER



#include "xsc.h"

// ---- Constants -----------------------------------------------------------------------------

	// ---- Lexemes ---------------------------------------------------------------------------

#define MAX_LEXEME_SIZE                 1024    // Maximum individual lexeme size

// ---- Operators -------------------------------------------------------------------------

#define MAX_OP_STATE_COUNT              32      // Maximum number of operator
												// states

// ---- Delimiters ------------------------------------------------------------------------

#define MAX_DELIM_COUNT                 24      // Maximum number of delimiters 分隔符最大数量

//! 词法分析器 状态

#define LEX_STATE_UNKNOWN               0       // Unknown lexeme type 未知状态

#define LEX_STATE_START                 1       // Start state 开始状态

#define LEX_STATE_INT                   2       // Integer 整型值状态
#define LEX_STATE_FLOAT                 3       // Float 浮点型状态

#define LEX_STATE_IDENT                 4       // Identifier  标识符状态


#define LEX_STATE_OP                    5       // Operator      运算符状态
#define LEX_STATE_DELIM                 6       // Delimiter    分隔符状态

#define LEX_STATE_STRING                7       // String value 字符串值
#define LEX_STATE_STRING_ESCAPE         8       // Escape sequence 转义序列
#define LEX_STATE_STRING_CLOSE_QUOTE    9       // String closing quote 字符串闭合引号

//! Token Types 属性符类型

#define TOKEN_TYPE_END_OF_STREAM        0       // End of the token stream 属性符流的结束
#define TOKEN_TYPE_INVALID              1       // Invalid token 无效属性符

#define TOKEN_TYPE_INT                  2       // Integer 整型类型
#define TOKEN_TYPE_FLOAT                3       // Float 浮点类型

#define TOKEN_TYPE_IDENT                4       // Identifier 标识符

#define TOKEN_TYPE_RSRVD_VAR            5       // 保留字var/var [] 
#define TOKEN_TYPE_RSRVD_TRUE           6       // 保留字true
#define TOKEN_TYPE_RSRVD_FALSE          7       // 保留字false
#define TOKEN_TYPE_RSRVD_IF             8       // 保留字if
#define TOKEN_TYPE_RSRVD_ELSE           9       //保留字 else
#define TOKEN_TYPE_RSRVD_BREAK          10      // 保留字break
#define TOKEN_TYPE_RSRVD_CONTINUE       11      // 保留字continue
#define TOKEN_TYPE_RSRVD_FOR            12      // 保留字for
#define TOKEN_TYPE_RSRVD_WHILE          13      // 保留字while
#define TOKEN_TYPE_RSRVD_FUNC           14      // 保留字func
#define TOKEN_TYPE_RSRVD_RETURN         15      // 保留字return
#define TOKEN_TYPE_RSRVD_HOST           16      // 保留字host

#define TOKEN_TYPE_OP                   18      // Operator

#define TOKEN_TYPE_DELIM_COMMA          19      //分隔符 ,
#define TOKEN_TYPE_DELIM_OPEN_PAREN     20      //分隔符 (
#define TOKEN_TYPE_DELIM_CLOSE_PAREN    21      //分隔符)
#define TOKEN_TYPE_DELIM_OPEN_BRACE     22      //分隔符 [
#define TOKEN_TYPE_DELIM_CLOSE_BRACE    23      //分隔符 ]
#define TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE   24  //分隔符 {
#define TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE  25  //分隔符 }
#define TOKEN_TYPE_DELIM_SEMICOLON      26      //分隔符 ;

#define TOKEN_TYPE_STRING               27      // String

// ---- Operators -------------------------------------------------------------------------

	// ---- Arithmetic

#define OP_TYPE_ADD                     0       // +
#define OP_TYPE_SUB                     1       // -
#define OP_TYPE_MUL                     2       // *
#define OP_TYPE_DIV                     3       // /
#define OP_TYPE_MOD                     4       // %
#define OP_TYPE_EXP                     5       // ^
#define OP_TYPE_CONCAT                  35       // $

#define OP_TYPE_INC                     15      // ++
#define OP_TYPE_DEC                     17      // --

#define OP_TYPE_ASSIGN_ADD              14      // +=
#define OP_TYPE_ASSIGN_SUB              16      // -=
#define OP_TYPE_ASSIGN_MUL              18      // *=
#define OP_TYPE_ASSIGN_DIV              19      // /=
#define OP_TYPE_ASSIGN_MOD              20      // %=
#define OP_TYPE_ASSIGN_EXP              21      // ^=
#define OP_TYPE_ASSIGN_CONCAT           36      // $=

// ---- Bitwise

#define OP_TYPE_BITWISE_AND             6       // &
#define OP_TYPE_BITWISE_OR              7       // |
#define OP_TYPE_BITWISE_XOR             8       // #
#define OP_TYPE_BITWISE_NOT             9       // ~
#define OP_TYPE_BITWISE_SHIFT_LEFT      30      // <<
#define OP_TYPE_BITWISE_SHIFT_RIGHT     32      // >>

#define OP_TYPE_ASSIGN_AND              22      // &=
#define OP_TYPE_ASSIGN_OR               24      // |=
#define OP_TYPE_ASSIGN_XOR              26      // #=
#define OP_TYPE_ASSIGN_SHIFT_LEFT       33      // <<=
#define OP_TYPE_ASSIGN_SHIFT_RIGHT      34      // >>=

// ---- Logical

#define OP_TYPE_LOGICAL_AND             23      // &&
#define OP_TYPE_LOGICAL_OR              25      // ||
#define OP_TYPE_LOGICAL_NOT             10      // !

// ---- Relational

#define OP_TYPE_EQUAL                   28      // ==
#define OP_TYPE_NOT_EQUAL               27      // !=
#define OP_TYPE_LESS                    12      // <
#define OP_TYPE_GREATER                 13      // >
#define OP_TYPE_LESS_EQUAL              29      // <=
#define OP_TYPE_GREATER_EQUAL           31      // >=

// ---- Assignment

#define OP_TYPE_ASSIGN                  11      // =

//! 数据结构体定义

typedef int Token;                                  // Token type 属性符（单词）类型 尽管属性符就是就是整形，我们还是重新定义

// The lexer's state 词法分析器状态
typedef struct _LexerState
{
	int iCurrLineIndex;                             // Current line index 当前行索引
	LinkedListNode* pCurrLine;                     // Current line node pointer 当前行节点指针
	Token CurrToken;                                // Current token 当前属性符
	char pstrCurrLexeme[MAX_LEXEME_SIZE];        // Current lexeme 当前词素
	int iCurrLexemeStart;                           // Current lexeme's starting index 当前词素的开始索引
	int iCurrLexemeEnd;                             // Current lexeme's ending index 当前词素的结束索引
	int iCurrOp;                                    // Current operator
}LexerState;

// Operator state 运算符状态
typedef struct _OpState                             
{
	char cChar;                                     // State character 状态字符
	int iSubStateIndex;                             // Index into sub state array where sub 后继状态数组的索引
	int iSubStateCount;                             // Number of substates 后继状态的个数
	int iIndex;                                     // Operator index 运算符索引
}OpState;

//! 函数声明

void ResetLexer();
void CopyLexerState(LexerState& pDestState, LexerState& pSourceState);

int GetOpStateIndex(char cChar, int iCharIndex, int iSubStateIndex, int iSubStateCount);
int IsCharOpChar(char cChar, int iCharIndex);
OpState GetOpState(int iCharIndex, int iStateIndex);
int IsCharDelim(char cChar);
int IsCharWhitespace(char cChar);
int IsCharNumeric(char cChar);
int IsCharIdent(char cChar);

char GetNextChar();
Token GetNextToken();
void RewindTokenStream();
Token GetCurrToken();
char* GetCurrLexeme();
void CopyCurrLexeme(char* pstrBuffer);
int GetCurrOp();
char GetLookAheadChar();

char* GetCurrSourceLine();
int GetCurrSourceLineIndex();
int GetLexemeStartIndex();

#endif