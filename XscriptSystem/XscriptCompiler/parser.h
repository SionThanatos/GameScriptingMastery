//Parser module header

#ifndef XSC_PARSER
#define XSC_PARSER

// ---- Include Files -------------------------------------------------------------------------

#include "xsc.h"
#include "lexer.h"

// ---- Constants -----------------------------------------------------------------------------

#define MAX_FUNC_DECLARE_PARAM_COUNT        32      // The maximum number of parameters
													// that can appear in a function
													// declaration.

// ---- Data Structures -----------------------------------------------------------------------

// Expression instance ���ʽʵ��
typedef struct _Expr                                
{
	int iStackOffset;                               // The current offset of the stack
}Expr;


//Loop instance ѭ��ʵ��
typedef struct Loop                                 
{
	int iStartTargetIndex;                          // The starting jump target
	int iEndTargetIndex;                            // The ending jump target
}Loop;

// ---- Function Prototypes -------------------------------------------------------------------

void ReadToken(Token ReqToken);

int IsOpRelational(int iOpType);
int IsOpLogical(int iOpType);

void ParseSourceCode();

void ParseStatement();
void ParseBlock();

void ParseVar();
void ParseHost();
void ParseFunc();

void ParseExpr();
void ParseSubExpr();
void ParseTerm();
void ParseFactor();

void ParseIf();
void ParseWhile();
void ParseFor();
void ParseBreak();
void ParseContinue();
void ParseReturn();

void ParseAssign();
void ParseFuncCall();

#endif