//Function table header
//函数表头文件

#ifndef XSC_FUNC_TABLE
#define XSC_FUNC_TABLE

// ---- Include Files -------------------------------------------------------------------------

#include "xsc.h"

// ---- Data Structures -----------------------------------------------------------------------

//函数表
typedef struct _FuncNode
{
	int iIndex;									    // Index 索引
	char pstrName[MAX_IDENT_SIZE];               // Name 名称
	int iIsHostAPI;                                 // Is this a host API function? 是否是host api
	int iParamCount;                                // The number of accepted parameters 接收的参数
	LinkedList ICodeStream;                         // Local I-code stream 局部中间代码流
}FuncNode;

// ---- Function Prototypes -------------------------------------------------------------------

FuncNode* GetFuncByIndex(int iIndex);
FuncNode* GetFuncByName(char* pstrName);
int AddFunc(char* pstrName, int iIsHostAPI);
void SetFuncParamCount(int iIndex, int iParamCount);

#endif