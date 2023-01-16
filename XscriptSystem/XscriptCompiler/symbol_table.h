//Symbol table header
//符号表头文件

#ifndef XSC_SYMBOL_TABLE
#define XSC_SYMBOL_TABLE



#include "xsc.h"

// ---- Constants -----------------------------------------------------------------------------

	// ---- Scope -----------------------------------------------------------------------------

#define SCOPE_GLOBAL                    0       // Global scope

// ---- Symbol Types ----------------------------------------------------------------------

#define SYMBOL_TYPE_VAR                 0       // Variable
#define SYMBOL_TYPE_PARAM               1       // Parameter

// ---- Data Structures -----------------------------------------------------------------------

//符号表节点
typedef struct _SymbolNode                          
{
	int iIndex;									    // Index 索引
	char pstrIdent[MAX_IDENT_SIZE];              // Identifier 标识符
	int iSize;                                      // Size (1 for variables, N for arrays) 大小
	int iScope;                                     // Scope (0 for globals, N for locals'function index) 作用域
	int iType;                                      // Symbol type (parameter or variable) 符号类型，参数或者变量
}SymbolNode;

// ---- Function Prototypes -------------------------------------------------------------------

SymbolNode* GetSymbolByIndex(int iIndex);
SymbolNode* GetSymbolByIdent(char* pstrIdent, int iScope);
int GetSizeByIdent(char* pstrIdent, int iScope);
int AddSymbol(char* pstrIdent, int iSize, int iScope, int iType);

#endif