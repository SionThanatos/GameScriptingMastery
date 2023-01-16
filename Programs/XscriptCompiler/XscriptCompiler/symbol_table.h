//Symbol table header
//���ű�ͷ�ļ�

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

//���ű�ڵ�
typedef struct _SymbolNode                          
{
	int iIndex;									    // Index ����
	char pstrIdent[MAX_IDENT_SIZE];              // Identifier ��ʶ��
	int iSize;                                      // Size (1 for variables, N for arrays) ��С
	int iScope;                                     // Scope (0 for globals, N for locals'function index) ������
	int iType;                                      // Symbol type (parameter or variable) �������ͣ��������߱���
}SymbolNode;

// ---- Function Prototypes -------------------------------------------------------------------

SymbolNode* GetSymbolByIndex(int iIndex);
SymbolNode* GetSymbolByIdent(char* pstrIdent, int iScope);
int GetSizeByIdent(char* pstrIdent, int iScope);
int AddSymbol(char* pstrIdent, int iSize, int iScope, int iType);

#endif