//Function table header
//������ͷ�ļ�

#ifndef XSC_FUNC_TABLE
#define XSC_FUNC_TABLE

// ---- Include Files -------------------------------------------------------------------------

#include "xsc.h"

// ---- Data Structures -----------------------------------------------------------------------

//������
typedef struct _FuncNode
{
	int iIndex;									    // Index ����
	char pstrName[MAX_IDENT_SIZE];               // Name ����
	int iIsHostAPI;                                 // Is this a host API function? �Ƿ���host api
	int iParamCount;                                // The number of accepted parameters ���յĲ���
	LinkedList ICodeStream;                         // Local I-code stream �ֲ��м������
}FuncNode;

// ---- Function Prototypes -------------------------------------------------------------------

FuncNode* GetFuncByIndex(int iIndex);
FuncNode* GetFuncByName(char* pstrName);
int AddFunc(char* pstrName, int iIsHostAPI);
void SetFuncParamCount(int iIndex, int iParamCount);

#endif