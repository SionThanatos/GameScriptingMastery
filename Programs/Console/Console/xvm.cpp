#include "xvm.h"



#define EXEC_FILE_EXT			    ".XSE"	    // Executable file extension

#define XSE_ID_STRING               "XSE0"      // Used to validate an .XSE executable

#define MAX_THREAD_COUNT		    1024        // The maximum number of scripts that
														// can be loaded at once. Change this
														// to support more or less.
														// to support more or less.
														// to support more or less.
														// to support more or less.
														// to support more or less.
														// to support more or less.
														// to support more or less.
														// to support more or less.
														// to support more or less.

	// ---- Operand/Value Types ---------------------------------------------------------------

#define OP_TYPE_NULL                -1          // Uninitialized/Null data
#define OP_TYPE_INT                 0           // Integer literal value
#define OP_TYPE_FLOAT               1           // Floating-point literal value ����
#define OP_TYPE_STRING		        2           // String literal value �ַ���
#define OP_TYPE_ABS_STACK_INDEX     3           // Absolute array index ������������
#define OP_TYPE_REL_STACK_INDEX     4           // Relative array index �����������
#define OP_TYPE_Instruction_INDEX         5           // Instruction index ָ������
#define OP_TYPE_FUNC_INDEX          6           // Function index ��������
#define OP_TYPE_HOST_API_CALL_INDEX 7           // Host API call index host api ��������
#define OP_TYPE_REG                 8           // Register �Ĵ���

#define OP_TYPE_STACK_BASE_MARKER   9           // Marks a stack base ��ջ��ַ���


//! ָ���opcode
#define Instruction_MOV                   0

#define Instruction_ADD                   1
#define Instruction_SUB                   2
#define Instruction_MUL                   3
#define Instruction_DIV                   4
#define Instruction_MOD                   5
#define Instruction_EXP                   6
#define Instruction_NEG                   7
#define Instruction_INC                   8
#define Instruction_DEC                   9

#define Instruction_AND                   10
#define Instruction_OR                    11
#define Instruction_XOR                   12
#define Instruction_NOT                   13
#define Instruction_SHL                   14
#define Instruction_SHR                   15

#define Instruction_CONCAT                16
#define Instruction_GETCHAR               17
#define Instruction_SETCHAR               18

#define Instruction_JMP                   19
#define Instruction_JE                    20
#define Instruction_JNE                   21
#define Instruction_JG                    22
#define Instruction_JL                    23
#define Instruction_JGE                   24
#define Instruction_JLE                   25

#define Instruction_PUSH                  26
#define Instruction_POP                   27

#define Instruction_CALL                  28
#define Instruction_RET                   29
#define Instruction_CALLHOST              30

#define Instruction_PAUSE                 31
#define Instruction_EXIT                  32

// ---- Stack -----------------------------------------------------------------------------

#define DEF_STACK_SIZE			    1024	    // The default stack size

// ---- Coercion --------------------------------------------------------------------------

#define MAX_COERCION_STRING_SIZE    64          // The maximum allocated space for a
												// string coercion

//! ���߳�

#define THREAD_MODE_MULTI           0           // Multithreaded execution ���߳�ִ��
#define THREAD_MODE_SINGLE          1           // Single-threaded execution ���߳�ִ��

#define THREAD_PRIORITY_DUR_LOW     20          // Low-priority thread timeslice �����ȼ��߳�ʱ��Ƭ
#define THREAD_PRIORITY_DUR_MED     40          // Medium-priority thread timeslice �����ȼ��߳�ʱ��Ƭ
#define THREAD_PRIORITY_DUR_HIGH    80          // High-priority thread timeslice �����ȼ��߳�ʱ��Ƭ

// ---- The Host API ----------------------------------------------------------------------

#define MAX_HOST_API_SIZE           1024        // Maximum number of functions in the
												// host API

// ---- Functions -------------------------------------------------------------------------

#define MAX_FUNC_NAME_SIZE          256         // Maximum size of a function's name

// ---- Data Structures -----------------------------------------------------------------------


//runtime value ����ʱ ֵ �ṹ
typedef struct _Value							
{
	int iType;                                  // Type
	union                                       // The value
	{
		int iIntLiteral;                        // Integer literal
		float fFloatLiteral;                    // Float literal
		char* pstrStringLiteral;				// String literal
		int iStackIndex;                        // Stack Index ջ����
		int iInstructionIndex;                  // Instruction index ָ������
		int iFuncIndex;                         // Function index ��������
		int iHostAPICallIndex;                  // Host API Call index Host API Call����
		int iReg;                               // Register code �Ĵ���
	};
	int iOffsetIndex;                           // Index of the offset
}Value;


//����ʱ��ջ
typedef struct _RuntimeStack					
{
	Value* pElmnts;							    // The stack elements ��ջԪ��
	int iSize;									// The number of elements in the stack ��ջ�е�Ԫ�ظ���
	int iTopIndex;								// The top index ջ������
	int iFrameIndex;                            // Index of the top of the Current ��ǰջ֡��������

}RuntimeStack;


//������
typedef struct _Func							
{
	int iEntryPoint;							// The entry point ��ڵ�
	int iParamCount;							// The parameter count ��������
	int iLocalDataSize;							// Total size of all local data �������ݴ�С
	int iStackFrameSize;						// Total size of the stack frame ջ֡��С
	char pstrName[MAX_FUNC_NAME_SIZE + 1];      // The function's name ��������
}Func;


//ָ��ṹ��
typedef struct _Instruction                           
{
	int iOpcode;                                // The opcode ������
	int iOpCount;                               // The number of operands �����������
	Value* pOpList;                            // The operand list
}Instruction;

//ָ�����ṹ
typedef struct _InstructionStream                     // An Instruction stream
{
	Instruction* pInstructions;							    // The Instructionuctions themselves ָ��ָ������ָ���ָ��
	int iSize;                                  // The number of Instructionuctions in the stream ����ָ��ĸ���
	int iCurrentInstruction;                             // The Instruction pointer ��ǰָ��ָ��
}InstructionStream;

//������
typedef struct _FuncTable                       // A function table
{
	Func* pFuncs;                              // Pointer to the function array
	int iSize;                                  // The number of functions in the array
}
FuncTable;


//host ����api���ñ�
typedef struct _HostAPICallTable			
{
	char** ppstrCalls;							// Pointer to the call array ָ��������ֵ�ָ��
	int iSize;									// The number of calls in the array �����еĵ��ø���
}HostAPICallTable;



//�ű��ṹ ��װһ�������Ľű�
typedef struct _Script							
{
	int iIsActive;								// Is this script structure in use? ����ű��ṹ���Ƿ񼤻�

	// Header data

	int iGlobalDataSize;						// The size of the script's global data �ű�ȫ�����ݴ�С
	int iIsMainFuncPresent;                     // Is _Main () present?  _Main ()�����Ƿ����
	int iMainFuncIndex;							// _Main ()'s function index _Main ()������

	// Runtime tracking

	int iIsRunning;								// Is the script running? �ű��Ƿ�����
	int iIsPaused;								// Is the script Currently paused? �ű��Ƿ���ͣ
	int iPauseEndTime;			                // If so, when should it resume? �����ͣ����ʱ�ָ�

	// Threading

	int iTimesliceDur;                          // The thread's timeslice duration �߳�ʱ��Ƭ

	// Register file

	Value _RetVal;								// The _RetVal register _RetVal�Ĵ���

	// Script data �ű�����

	InstructionStream InstructionStream;        // The Instruction stream ָ����
	RuntimeStack Stack;                         // The runtime stack ����ʱ��ջ
	FuncTable FuncTable;                        // The function table ������
	HostAPICallTable HostAPICallTable;			// The host API call table hostӦ��api���ñ�
}Script;

//Host API function
typedef struct _HostAPIFunc                  
{
	int iIsActive;                              // Is this slot in use? �ú����Ƿ�ʹ��

	int iThreadIndex;                           // The thread to which this function �����ĸ��߳���������ɼ�
	// is visible
	char* pstrName;                            // The function name ��������
	HostAPIFuncPntr fnFunc;                     // Pointer to the function definition ָ���������ָ��
}HostAPIFunc;

//! ȫ��


//ȫ�ֽű�ʵ�� �������ж���ű���װ��
Script g_Scripts[MAX_THREAD_COUNT];		    // The script array ȫ�ֽű�ʵ������

//! �߳�

int g_iCurrentThreadMode;                          // The Current threading mode �߳�ģʽ
int g_iCurrentThread;								// The Currently running thread ��ǰ���е��߳�
int g_iCurrentThreadActiveTime;					// The time at which the Current thread was activated

//! Host API
HostAPIFunc g_HostAPI[MAX_HOST_API_SIZE];    

// ---- Macros --------------------------------------------------------------------------------

//Resolves a stack index by translating negative indices relative to the top of the stack, to positive ones relative to the bottom.
#define ResolveStackIndex( iIndex )	\
										\
		( iIndex < 0 ? iIndex += g_Scripts [ g_iCurrentThread ].Stack.iFrameIndex : iIndex )



//Returns TRUE if the specified thread index is within the bounds of the array, FALSE otherwise.
//ȷ���߳������ں��ʵķ�Χ��
#define IsValidThreadIndex( iIndex )    \
                                            \
        ( iIndex < 0 || iIndex > MAX_THREAD_COUNT ? FALSE : TRUE )



//Returns TRUE if the specified thread is both a valid index and active, FALSE otherwise.
//ȷ���߳�����ָ����ǻ�Ծ���߳�
#define IsThreadActive( iIndex )    \
                                        \
        ( IsValidThreadIndex ( iIndex ) && g_Scripts [ iIndex ].iIsActive ? TRUE : FALSE )

	// ---- Function Prototypes -------------------------------------------------------------------

		// ---- Operand Interface -----------------------------------------------------------------

int CoerceValueToInt(Value Val);
float CoerceValueToFloat(Value Val);
char* CoerceValueToString(Value Val);

void CopyValue(Value* pDest, Value Source);

int GetOpType(int iOpIndex);
int ResolveOpStackIndex(int iOpIndex);
Value ResolveOpValue(int iOpIndex);
int ResolveOpType(int iOpIndex);
int ResolveOpAsInt(int iOpIndex);
float ResolveOpAsFloat(int iOpIndex);
char* ResolveOpAsString(int iOpIndex);
int ResolveOpAsInstructionIndex(int iOpIndex);
int ResolveOpAsFuncIndex(int iOpIndex);
char* ResolveOpAsHostAPICall(int iOpIndex);
Value* ResolveOpPntr(int iOpIndex);

// ---- Runtime Stack Interface -----------------------------------------------------------

Value GetStackValue(int iThreadIndex, int iIndex);
void SetStackValue(int iThreadIndex, int iIndex, Value Val);
void Push(int iThreadIndex, Value Val);
Value Pop(int iThreadIndex);
void PushFrame(int iThreadIndex, int iSize);
void PopFrame(int iSize);



Func GetFunc(int iThreadIndex, int iIndex);



char* GetHostAPICall(int iIndex);



int GetCurrTime();



void CallFunc(int iThreadIndex, int iIndex);


/// <summary>
/// ��ʼ������ʱ����
/// </summary>
void XS_Init()
{
	// ---- Initialize the script array
	//��ʼ���ű�����
	for (int iCurrentScriptIndex = 0; iCurrentScriptIndex < MAX_THREAD_COUNT; ++iCurrentScriptIndex)
	{
		g_Scripts[iCurrentScriptIndex].iIsActive = FALSE;

		g_Scripts[iCurrentScriptIndex].iIsRunning = FALSE;
		g_Scripts[iCurrentScriptIndex].iIsMainFuncPresent = FALSE;
		g_Scripts[iCurrentScriptIndex].iIsPaused = FALSE;

		g_Scripts[iCurrentScriptIndex].InstructionStream.pInstructions = NULL;
		g_Scripts[iCurrentScriptIndex].Stack.pElmnts = NULL;
		g_Scripts[iCurrentScriptIndex].FuncTable.pFuncs = NULL;
		g_Scripts[iCurrentScriptIndex].HostAPICallTable.ppstrCalls = NULL;
	}

	// ---- Initialize the host API

	for (int iCurrentHostAPIFunc = 0; iCurrentHostAPIFunc < MAX_HOST_API_SIZE; ++iCurrentHostAPIFunc)
	{
		g_HostAPI[iCurrentHostAPIFunc].iIsActive = FALSE;
		g_HostAPI[iCurrentHostAPIFunc].pstrName = NULL;
	}

	// ---- Set up the threads
	
	g_iCurrentThreadMode = THREAD_MODE_MULTI;
	//����ǰ�߳���������Ϊ0
	g_iCurrentThread = 0;
}


/// <summary>
/// �ر�����ʱ����
/// </summary>
void XS_ShutDown()
{
	// ---- Unload any scripts that may still be in memory

	for (int iCurrentScriptIndex = 0; iCurrentScriptIndex < MAX_THREAD_COUNT; ++iCurrentScriptIndex)
		XS_UnloadScript(iCurrentScriptIndex);

	// ---- Free the host API's function name strings

	for (int iCurrentHostAPIFunc = 0; iCurrentHostAPIFunc < MAX_HOST_API_SIZE; ++iCurrentHostAPIFunc)
		if (g_HostAPI[iCurrentHostAPIFunc].pstrName)
			free(g_HostAPI[iCurrentHostAPIFunc].pstrName);
}



/// <summary>
/// ����XSE�ļ������ڴ�
/// Loads an .XSE file into memory.
/// </summary>
/// <param name="pstrFilename"></param>
/// <param name="iThreadIndex"></param>
/// <param name="iThreadTimeslice"></param>
/// <returns></returns>
int XS_LoadScript(char* pstrFilename, int& iThreadIndex, int iThreadTimeslice)
{
	// ---- Find the next free script index
	//������һ�����нű�������
	int iFreeThreadFound = FALSE;
	for (int iCurrentThreadIndex = 0; iCurrentThreadIndex < MAX_THREAD_COUNT; ++iCurrentThreadIndex)
	{
		// If the Current thread is not in use, use it
		//��ǰ�̲߳���ʹ���У���ʹ����
		if (!g_Scripts[iCurrentThreadIndex].iIsActive)
		{
			iThreadIndex = iCurrentThreadIndex;
			iFreeThreadFound = TRUE;
			break;
		}
	}

	// If a thread wasn't found, return an out of threads error
	//δ�ҵ��߳�
	if (!iFreeThreadFound)
		return XS_LOAD_ERROR_OUT_OF_THREADS;

	//! ���ļ�

	FILE* pScriptFile;
	if (!(pScriptFile = fopen(pstrFilename, "rb")))
		return XS_LOAD_ERROR_FILE_IO;

	//! ��ȡ�ļ�ͷ

	// Create a buffer to hold the file's ID string (4 bytes + 1 null terminator = 5)
	//���������������ļ���ͷ��ID�ַ��� 5���ֽ�
	char* pstrIDString;
	if (!(pstrIDString = (char*)malloc(5)))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read the string (4 bytes) and append a null terminator
	//��ȡ�ַ�����4�� ����׷��һ���ս��Null
	fread(pstrIDString, 4, 1, pScriptFile);
	pstrIDString[strlen(XSE_ID_STRING)] = '\0';

	// Compare the data read from the file to the ID string and exit on an error if they don't match
	//�Ƚ��ļ�ͷ�ַ�
	if (strcmp(pstrIDString, XSE_ID_STRING) != 0)
		return XS_LOAD_ERROR_INVALID_XSE;

	// Free the buffer
	//�ͷŻ�����
	free(pstrIDString);

	// Read the script version (2 bytes total)
	//��ȡ�ű��İ汾
	int iMajorVersion = 0,
		iMinorVersion = 0;

	fread(&iMajorVersion, 1, 1, pScriptFile);
	fread(&iMinorVersion, 1, 1, pScriptFile);

	// Validate the version, since this prototype only supports version 0.8 scripts
	//У��汾
	if (iMajorVersion != 0 || iMinorVersion != 8)
		return XS_LOAD_ERROR_UNSUPPORTED_VERS;

	// Read the stack size (4 bytes)
	//��ȡ��ջ�Ĵ�С
	fread(&g_Scripts[iThreadIndex].Stack.iSize, 4, 1, pScriptFile);

	// Check for a default stack size request
	//�����Ҫ��Ĭ�϶�ջ��С
	if (g_Scripts[iThreadIndex].Stack.iSize == 0)
		g_Scripts[iThreadIndex].Stack.iSize = DEF_STACK_SIZE;

	// Allocate the runtime stack
	//��������ʱ��ջ
	int iStackSize = g_Scripts[iThreadIndex].Stack.iSize;
	if (!(g_Scripts[iThreadIndex].Stack.pElmnts = (Value*)malloc(iStackSize * sizeof(Value))))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read the global data size (4 bytes)
	//��ȡȫ�����ݴ�С
	fread(&g_Scripts[iThreadIndex].iGlobalDataSize, 4, 1, pScriptFile);

	// Check for presence of _Main () (1 byte)
	//���_Main�����Ƿ����
	fread(&g_Scripts[iThreadIndex].iIsMainFuncPresent, 1, 1, pScriptFile);

	// Read _Main ()'s function index (4 bytes)
	//��ȡ_Main����������
	fread(&g_Scripts[iThreadIndex].iMainFuncIndex, 4, 1, pScriptFile);

	// Read the priority type (1 byte)

	int iPriorityType = 0;
	fread(&iPriorityType, 1, 1, pScriptFile);

	// Read the user-defined priority (4 bytes)

	fread(&g_Scripts[iThreadIndex].iTimesliceDur, 4, 1, pScriptFile);

	// Override the script-specified priority if necessary

	if (iThreadTimeslice != XS_THREAD_PRIORITY_USER)
		iPriorityType = iThreadTimeslice;

	// If the priority type is not set to user-defined, fill in the appropriate timeslice
	// duration

	switch (iPriorityType)
	{
	case XS_THREAD_PRIORITY_LOW:
		g_Scripts[iThreadIndex].iTimesliceDur = THREAD_PRIORITY_DUR_LOW;
		break;

	case XS_THREAD_PRIORITY_MED:
		g_Scripts[iThreadIndex].iTimesliceDur = THREAD_PRIORITY_DUR_MED;
		break;

	case XS_THREAD_PRIORITY_HIGH:
		g_Scripts[iThreadIndex].iTimesliceDur = THREAD_PRIORITY_DUR_HIGH;
		break;
	}

	//! ��ȡָ����

	// Read the Instruction count (4 bytes)
	//��ȡָ����
	fread(&g_Scripts[iThreadIndex].InstructionStream.iSize, 4, 1, pScriptFile);

	// Allocate the stream
	//����ָ�����ڴ�
	if (!(g_Scripts[iThreadIndex].InstructionStream.pInstructions = (Instruction*)malloc(g_Scripts[iThreadIndex].InstructionStream.iSize * sizeof(Instruction))))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read the Instruction data
	//ѭ����ȡ��ָ������
	for (int iCurrentInstructionIndex = 0; iCurrentInstructionIndex < g_Scripts[iThreadIndex].InstructionStream.iSize; ++iCurrentInstructionIndex)
	{
		// Read the opcode (2 bytes)
		//��ȡ������
		g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].iOpcode = 0;
		fread(&g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].iOpcode, 2, 1, pScriptFile);

		// Read the operand count (1 byte)
		//��ȡ�������ĸ���
		g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].iOpCount = 0;
		fread(&g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].iOpCount, 1, 1, pScriptFile);

		int iOpCount = g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].iOpCount;//�������ĸ���

		// Allocate space for the operand list in a temporary pointer
		//����ʱָ����Ϊ�������б�����ڴ�
		Value* pOpList;
		if (!(pOpList = (Value*)malloc(iOpCount * sizeof(Value))))
			return XS_LOAD_ERROR_OUT_OF_MEMORY;

		// Read in the operand list (N bytes)
		//��ȡ�������б�
		for (int iCurrentOpIndex = 0; iCurrentOpIndex < iOpCount; ++iCurrentOpIndex)
		{
			// Read in the operand type (1 byte)
			//��ȡ����������
			pOpList[iCurrentOpIndex].iType = 0;
			fread(&pOpList[iCurrentOpIndex].iType, 1, 1, pScriptFile);

			// Depending on the type, read in the operand data
			//�������ͣ���ȡ����
			switch (pOpList[iCurrentOpIndex].iType)
			{
			// Integer literal
		    //����
			case OP_TYPE_INT:
				fread(&pOpList[iCurrentOpIndex].iIntLiteral, sizeof(int), 1, pScriptFile);
				break;

			// Floating-point literal
            //����
			case OP_TYPE_FLOAT:
				fread(&pOpList[iCurrentOpIndex].fFloatLiteral, sizeof(float), 1, pScriptFile);
				break;

			// String index
            //�ַ�������
			case OP_TYPE_STRING:

				// Since there's no field in the Value structure for string table indices, 
				// read the index into the integer literal field and set its type to string index

				fread(&pOpList[iCurrentOpIndex].iIntLiteral, sizeof(int), 1, pScriptFile);
				pOpList[iCurrentOpIndex].iType = OP_TYPE_STRING;
				break;

			// Instruction index
			//ָ������
			case OP_TYPE_Instruction_INDEX:
				fread(&pOpList[iCurrentOpIndex].iInstructionIndex, sizeof(int), 1, pScriptFile);
				break;

			// Absolute stack index
            //���Զ�ջ����
			case OP_TYPE_ABS_STACK_INDEX:
				fread(&pOpList[iCurrentOpIndex].iStackIndex, sizeof(int), 1, pScriptFile);
				break;

			// Relative stack index
			//��Զ�ջ����
			case OP_TYPE_REL_STACK_INDEX:
				fread(&pOpList[iCurrentOpIndex].iStackIndex, sizeof(int), 1, pScriptFile);
				fread(&pOpList[iCurrentOpIndex].iOffsetIndex, sizeof(int), 1, pScriptFile);
				break;

			// Function index
			//��������
			case OP_TYPE_FUNC_INDEX:
				fread(&pOpList[iCurrentOpIndex].iFuncIndex, sizeof(int), 1, pScriptFile);
				break;

			// Host API call index
			//host api��������
			case OP_TYPE_HOST_API_CALL_INDEX:
				fread(&pOpList[iCurrentOpIndex].iHostAPICallIndex, sizeof(int), 1, pScriptFile);
				break;

			// Register
            //�Ĵ���
			case OP_TYPE_REG:
				fread(&pOpList[iCurrentOpIndex].iReg, sizeof(int), 1, pScriptFile);
				break;
			}
		}

		// Assign the operand list pointer to the Instruction stream
		//�ò������б�ָ��ָ����
		g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].pOpList = pOpList;
	}

	//! ��ȡ�ַ�����

	// Read the table size (4 bytes)
	//��ȡ�ַ������С
	int iStringTableSize;
	fread(&iStringTableSize, 4, 1, pScriptFile);

	// If the string table exists, read it
	//����ַ�������ڣ���ȡ
	if (iStringTableSize)
	{
		// Allocate a string table of this size
		//�����ַ�����ռ�
		char** ppstrStringTable;
		if (!(ppstrStringTable = (char**)malloc(iStringTableSize * sizeof(char*))))
			return XS_LOAD_ERROR_OUT_OF_MEMORY;

		// Read in each string
		//��ȡÿ��string
		for (int iCurrentStringIndex = 0; iCurrentStringIndex < iStringTableSize; ++iCurrentStringIndex)
		{
			// Read in the string size (4 bytes)
			//��ȡstring ��С
			int iStringSize;
			fread(&iStringSize, 4, 1, pScriptFile);

			// Allocate space for the string plus a null terminator
			//Ϊ�ַ���+�ս������ռ�
			char* pstrCurrentString;
			if (!(pstrCurrentString = (char*)malloc(iStringSize + 1)))
				return XS_LOAD_ERROR_OUT_OF_MEMORY;

			// Read in the string data (N bytes) and append the null terminator
			//��ȡ�ַ������ݲ�������ս��
			fread(pstrCurrentString, iStringSize, 1, pScriptFile);
			pstrCurrentString[iStringSize] = '\0';

			// Assign the string pointer to the string table
			//���ַ���ָ��ָ���ַ�����
			ppstrStringTable[iCurrentStringIndex] = pstrCurrentString;
		}

		// Run through each operand in the Instruction stream and assign copies of string operand's corresponding string literals
		//��ָ�������ÿ�����������д����ҽ���Ӧ�ַ�����ֵ�����ַ���������
		for (int iCurrentInstructionIndex = 0; iCurrentInstructionIndex < g_Scripts[iThreadIndex].InstructionStream.iSize; ++iCurrentInstructionIndex)
		{
			// Get the Instruction's operand count and a copy of it's operand list
			//��ȡָ��������������Ҹ�����������б�
			int iOpCount = g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].iOpCount;
			Value* pOpList = g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].pOpList;

			// Loop through each operand
			//ѭ��ÿ��������
			for (int iCurrentOpIndex = 0; iCurrentOpIndex < iOpCount; ++iCurrentOpIndex)
			{
				// If the operand is a string index, make a local copy of it's corresponding string in the table
				//������������ַ����������������ڱ��е���Ӧ�ַ���
				if (pOpList[iCurrentOpIndex].iType == OP_TYPE_STRING)
				{
					// Get the string index from the operand's integer literal field
					//��ȡ�ַ�������
					int iStringIndex = pOpList[iCurrentOpIndex].iIntLiteral;

					// Allocate a new string to hold a copy of the one in the table
					//����һ�����ַ������ڴ洢
					char* pstrStringCopy;
					if (!(pstrStringCopy = (char*)malloc(strlen(ppstrStringTable[iStringIndex]) + 1)))
						return XS_LOAD_ERROR_OUT_OF_MEMORY;

					// Make a copy of the string
					//���Ʊ��е��ַ���
					strcpy(pstrStringCopy, ppstrStringTable[iStringIndex]);

					// Save the string pointer in the operand list
					//���ַ���ָ�뱣���ڲ������б���
					pOpList[iCurrentOpIndex].pstrStringLiteral = pstrStringCopy;
				}
			}
		}

		//�ͷ�ԭʼ�ַ���
		for (int iCurrentStringIndex = 0; iCurrentStringIndex < iStringTableSize; ++iCurrentStringIndex)
			free(ppstrStringTable[iCurrentStringIndex]);

		//�ͷ��ַ�������
		free(ppstrStringTable);
	}

	//! ��ȡ������ ���������ű���������Ϣ

	// Read the function count (4 bytes)
	//��ȡ��������
	int iFuncTableSize;
	fread(&iFuncTableSize, 4, 1, pScriptFile);

	g_Scripts[iThreadIndex].FuncTable.iSize = iFuncTableSize;

	// Allocate the table
	//���亯����
	if (!(g_Scripts[iThreadIndex].FuncTable.pFuncs = (Func*)malloc(iFuncTableSize * sizeof(Func))))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read each function
	//��ȡÿһ������
	for (int iCurrentFuncIndex = 0; iCurrentFuncIndex < iFuncTableSize; ++iCurrentFuncIndex)
	{
		// Read the entry point (4 bytes)
		//��ȡ��ڵ�
		int iEntryPoint;
		fread(&iEntryPoint, 4, 1, pScriptFile);

		// Read the parameter count (1 byte)
		//��ȡ��������
		int iParamCount = 0;
		fread(&iParamCount, 1, 1, pScriptFile);

		// Read the local data size (4 bytes)
		//��ȡ�ֲ����ݴ�С
		int iLocalDataSize;
		fread(&iLocalDataSize, 4, 1, pScriptFile);

		// Calculate the stack size
		//����ջ��С = ��������+1+�ֲ����ݴ�С
		int iStackFrameSize = iParamCount + 1 + iLocalDataSize;

		// Read the function name length (1 byte)
		//��ȡ�������Ƴ���
		int iFuncNameLength = 0;
		fread(&iFuncNameLength, 1, 1, pScriptFile);

		// Read the function name (N bytes) and append a null-terminator
		//��ȡ�������Ʋ��Ҽ����ս��
		fread(&g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrentFuncIndex].pstrName, iFuncNameLength, 1, pScriptFile);
		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrentFuncIndex].pstrName[iFuncNameLength] = '\0';

		// Write everything to the function table
		//������Ϣд�뺯����
		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrentFuncIndex].iEntryPoint = iEntryPoint;
		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrentFuncIndex].iParamCount = iParamCount;
		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrentFuncIndex].iLocalDataSize = iLocalDataSize;
		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrentFuncIndex].iStackFrameSize = iStackFrameSize;
	}

	//! ��ȡhost api ����

	// Read the host API call count
	//��ȡapi call ����
	fread(&g_Scripts[iThreadIndex].HostAPICallTable.iSize, 4, 1, pScriptFile);

	// Allocate the table
	//����api���ñ�
	if (!(g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls = (char**)malloc(g_Scripts[iThreadIndex].HostAPICallTable.iSize * sizeof(char*))))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read each host API call
	//��ȡÿ��call
	for (int iCurrentCallIndex = 0; iCurrentCallIndex < g_Scripts[iThreadIndex].HostAPICallTable.iSize; ++iCurrentCallIndex)
	{
		// Read the host API call string size (1 byte)
		//��ȡapi���õ��ַ�����С
		int iCallLength = 0;
		fread(&iCallLength, 1, 1, pScriptFile);

		// Allocate space for the string plus the null terminator in a temporary pointer
		//����ʱָ����Ϊ�ַ���+�ս�������С
		char* pstrCurrentCall;
		if (!(pstrCurrentCall = (char*)malloc(iCallLength + 1)))
			return XS_LOAD_ERROR_OUT_OF_MEMORY;

		// Read the host API call string data and append the null terminator
		//��ȡhost api�����ַ������ݲ���׷���ս��
		fread(pstrCurrentCall, iCallLength, 1, pScriptFile);
		pstrCurrentCall[iCallLength] = '\0';

		// Assign the temporary pointer to the table
		//��ָ��ָ��api���ñ�
		g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls[iCurrentCallIndex] = pstrCurrentCall;
	}

	//! �����ļ���ȡ
	fclose(pScriptFile);

	// The script is fully loaded and ready to go, so set the active flag
	//�ű��Ѿ���ȫװ�أ�����active flag
	g_Scripts[iThreadIndex].iIsActive = TRUE;

	// Reset the script
	//reset �ű�
	XS_ResetScript(iThreadIndex);

	// Return a success code
	//����ok
	return XS_LOAD_OK;
}



/// <summary>
/// ���ڴ���ж�ؽű�
/// </summary>
/// <param name="iThreadIndex"></param>
void XS_UnloadScript(int iThreadIndex)
{
	// Exit if the script isn't active

	if (!g_Scripts[iThreadIndex].iIsActive)
		return;

	//! �ͷ�ָ����

	// First check to see if any Instructionuctions have string operands, and free them if they do
	//���ȼ���Ƿ���ָ������ַ�������������������ͷ�����
	for (int iCurrentInstructionIndex = 0; iCurrentInstructionIndex < g_Scripts[iThreadIndex].InstructionStream.iSize; ++iCurrentInstructionIndex)
	{
		// Make a local copy of the operand count and operand list
		//�Բ����������Ͳ������б����ֲ�����
		int iOpCount = g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].iOpCount;
		Value* pOpList = g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].pOpList;

		// Loop through each operand and free its string pointer
		//�Բ������ĸ���ѭ�����ͷ��ַ���ָ��
		for (int iCurrentOpIndex = 0; iCurrentOpIndex < iOpCount; ++iCurrentOpIndex)
			if (pOpList[iCurrentOpIndex].pstrStringLiteral)
				pOpList[iCurrentOpIndex].pstrStringLiteral;
	}

	// Now free the stream itself
	//�ͷ�ָ��������
	if (g_Scripts[iThreadIndex].InstructionStream.pInstructions)
		free(g_Scripts[iThreadIndex].InstructionStream.pInstructions);

	//! �ͷ�����ʱ��ջ

	// Free any strings that are still on the stack

	for (int iCurrentElementIndex = 0; iCurrentElementIndex < g_Scripts[iThreadIndex].Stack.iSize; ++iCurrentElementIndex)
		if (g_Scripts[iThreadIndex].Stack.pElmnts[iCurrentElementIndex].iType == OP_TYPE_STRING)
			free(g_Scripts[iThreadIndex].Stack.pElmnts[iCurrentElementIndex].pstrStringLiteral);

	// Now free the stack itself
	//�ͷ�ָ��������
	if (g_Scripts[iThreadIndex].Stack.pElmnts)
		free(g_Scripts[iThreadIndex].Stack.pElmnts);

	//! �ͷź�����

	if (g_Scripts[iThreadIndex].FuncTable.pFuncs)
		free(g_Scripts[iThreadIndex].FuncTable.pFuncs);

	//! �ͷ�host API call������

	// First free each string in the table individually
	//�ͷ��ַ���
	for (int iCurrentCallIndex = 0; iCurrentCallIndex < g_Scripts[iThreadIndex].HostAPICallTable.iSize; ++iCurrentCallIndex)
		if (g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls[iCurrentCallIndex])
			free(g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls[iCurrentCallIndex]);

	// Now free the table itself
	//�ͷű�
	if (g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls)
		free(g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls);
}


/// <summary>
/// Resets the script. This function accepts a thread index rather than relying on the Currently active thread, because scripts can (and will) need to be reset arbitrarily.
/// ��ʼ�������
/// </summary>
/// <param name="iThreadIndex"></param>
void XS_ResetScript(int iThreadIndex)
{
	// Get _Main ()'s function index in case we need it
	//����Ҫ��ʱ���ȡ_Main ()������
	int iMainFuncIndex = g_Scripts[iThreadIndex].iMainFuncIndex;

	// If the function table is present, set the entry point
	//�����������ڣ�������ڵ�
	if (g_Scripts[iThreadIndex].FuncTable.pFuncs)
	{
		// If _Main () is present, read _Main ()'s index of the function table to get its entry point
		//���_Main()�������ڣ���ȡ���ں������е���������ȡ��ڵ�
		if (g_Scripts[iThreadIndex].iIsMainFuncPresent)
		{
			g_Scripts[iThreadIndex].InstructionStream.iCurrentInstruction = g_Scripts[iThreadIndex].FuncTable.pFuncs[iMainFuncIndex].iEntryPoint;
		}
	}

	// Clear the stack
	//��ն�ջ
	g_Scripts[iThreadIndex].Stack.iTopIndex = 0;
	g_Scripts[iThreadIndex].Stack.iFrameIndex = 0;

	// Set the entire stack to null
	//��ջֵ����Ϊnull
	for (int iCurrentElmntIndex = 0; iCurrentElmntIndex < g_Scripts[iThreadIndex].Stack.iSize; ++iCurrentElmntIndex)
		g_Scripts[iThreadIndex].Stack.pElmnts[iCurrentElmntIndex].iType = OP_TYPE_NULL;

	// Unpause the script
	//����pause flag
	g_Scripts[iThreadIndex].iIsPaused = FALSE;

	// Allocate space for the globals
	//Ϊȫ�ֱ�������ռ�
	PushFrame(iThreadIndex, g_Scripts[iThreadIndex].iGlobalDataSize);

	// If _Main () is present, push its stack frame (plus one extra stack element to compensate for the function index that usually sits on top of stack frames and causes indices to start from -2)
	//���_Main�������ڣ�ѹ������ջ֡ Ϊ���ֲ����������ٶ����һ��
	PushFrame(iThreadIndex, g_Scripts[iThreadIndex].FuncTable.pFuncs[iMainFuncIndex].iLocalDataSize + 1);
}


/// <summary>
/// Runs the Currenty loaded script array for a given timeslice duration.
/// ���нű�
/// </summary>
/// <param name="iTimesliceDur"></param>
void XS_RunScripts(int iTimesliceDur)
{
	// Begin a loop that runs until a keypress. The Instruction pointer has already been
	// initialized with a prior call to ResetScripts (), so execution can begin

	// Create a flag that Instructionuctions can use to break the execution loop

	int iExitExecLoop = FALSE;

	// Create a variable to hold the time at which the main timeslice started

	int iMainTimesliceStartTime = GetCurrTime();

	// Create a variable to hold the Current time
	//����������¼��ǰʱ��
	int iCurrentTime;

	while (TRUE)
	{

		

		// Check to see if all threads have terminated, and if so, break the execution cycle
		//����Ƿ������߳̽���������������
		int iIsStillActive = FALSE;
		for (int iCurrentThreadIndex = 0; iCurrentThreadIndex < MAX_THREAD_COUNT; ++iCurrentThreadIndex)
		{
			if (g_Scripts[iCurrentThreadIndex].iIsActive && g_Scripts[iCurrentThreadIndex].iIsRunning)
				iIsStillActive = TRUE;
		}
		if (!iIsStillActive)
			break;

		// Update the Current time
		//���µ�ǰʱ��
		iCurrentTime = GetCurrTime();

		// Check for a context switch if the threading mode is set for multithreading
		//! ִ���������л�  ����Ƕ��߳�ģʽ
		if (g_iCurrentThreadMode == THREAD_MODE_MULTI)
		{
			// If the Current thread's timeslice has elapsed, or if it's terminated switch to the next valid thread
			//�����ǰ�̵߳�ʱ��Ƭ��������߳̽������л�����һ���Ϸ��߳�
			if (iCurrentTime > g_iCurrentThreadActiveTime + g_Scripts[g_iCurrentThread].iTimesliceDur ||
				!g_Scripts[g_iCurrentThread].iIsRunning)
			{
				// Loop until the next thread is found
				//����ѭ��ֱ���Ҵ���һ���߳�
				while (TRUE)
				{
					// Move to the next thread in the array
					//�ƶ�����һ���߳�
					++g_iCurrentThread;

					// If we're past the end of the array, loop back around
					//�������������Ľ�β���������²鿴
					if (g_iCurrentThread >= MAX_THREAD_COUNT)
						g_iCurrentThread = 0;

					// If the thread we've chosen is active and running, break the loop
					//����߳��ǻ�Ծ���������еģ�������ѭ��
					if (g_Scripts[g_iCurrentThread].iIsActive && g_Scripts[g_iCurrentThread].iIsRunning)
						break;
				}

				// Reset the timeslice
				//����ʱ��Ƭ
				g_iCurrentThreadActiveTime = iCurrentTime;
			}
		}

		// Is the script Currently paused?
		//��ǰ�ű��Ƿ���ͣ
		if (g_Scripts[g_iCurrentThread].iIsPaused)
		{
			// Has the pause duration elapsed yet?
			//��ͣ�ĳ���ʱ���Ѿ���ȥ����
			if (iCurrentTime >= g_Scripts[g_iCurrentThread].iPauseEndTime)
			{
				// Yes, so unpause the script
				//�Ѿ����ڣ�������ͣ
				g_Scripts[g_iCurrentThread].iIsPaused = FALSE;
			}
			else
			{
				// No, so skip this iteration of the execution cycle
				//û�й���
				continue;
			}
		}

		// Make a copy of the Instruction pointer to compare later
		//���浱ǰָ���ָ�� ������ip
		int iCurrentInstruction = g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction;

		// Get the Current opcode
		//��ȡ��ǰ��opcode
		int iOpcode = g_Scripts[g_iCurrentThread].InstructionStream.pInstructions[iCurrentInstruction].iOpcode;

		// Execute the Current Instruction based on its opcode, as long as we aren't Currently paused
		//ִ��opcode
		switch (iOpcode)
		{
			// ---- Binary Operations

			// All of the binary operation Instructionuctions (move, arithmetic, and bitwise)
			// are combined into a single case that keeps us from having to rewrite the
			// otherwise redundant operand resolution and result storage phases over and
			// over. We then use an additional switch block to determine which operation
			// should be performed.

			// Move

		case Instruction_MOV:

			// Arithmetic Operations

		case Instruction_ADD:
		case Instruction_SUB:
		case Instruction_MUL:
		case Instruction_DIV:
		case Instruction_MOD:
		case Instruction_EXP:

			// Bitwise Operations

		case Instruction_AND:
		case Instruction_OR:
		case Instruction_XOR:
		case Instruction_SHL:
		case Instruction_SHR:
		{
			// Get a local copy of the destination operand (operand index 0)

			Value Dest = ResolveOpValue(0);

			// Get a local copy of the source operand (operand index 1)

			Value Source = ResolveOpValue(1);

			// Depending on the Instruction, perform a binary operation
			//opcodeִ��
			switch (iOpcode)
			{
			// Move
			case Instruction_MOV:

				// Skip cases where the two operands are the same
				//�������������һ��������
				if (ResolveOpPntr(0) == ResolveOpPntr(1))
					break;

				// Copy the source operand into the destination
				//����
				CopyValue(&Dest, Source);

				break;

				// The arithmetic Instructionuctions only work with destination types that
				// are either integers or floats. They first check for integers and
				// assume that anything else is a float. Mod only works with integers.

			// Add
			case Instruction_ADD:
				//Դ��������Ŀ���������� �ӵ�Ŀ���������
				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral += ResolveOpAsInt(1);
				else
					Dest.fFloatLiteral += ResolveOpAsFloat(1);

				break;

			// Subtract
			case Instruction_SUB:

				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral -= ResolveOpAsInt(1);
				else
					Dest.fFloatLiteral -= ResolveOpAsFloat(1);

				break;

			// Multiply
			case Instruction_MUL:

				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral *= ResolveOpAsInt(1);
				else
					Dest.fFloatLiteral *= ResolveOpAsFloat(1);

				break;

			// Divide
			case Instruction_DIV:

				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral /= ResolveOpAsInt(1);
				else
					Dest.fFloatLiteral /= ResolveOpAsFloat(1);

				break;

			// Modulus
			case Instruction_MOD:

				// Remember, Mod works with integers only

				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral %= ResolveOpAsInt(1);

				break;

			// Exponentiate
			case Instruction_EXP:

				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral = (int)pow(Dest.iIntLiteral, ResolveOpAsInt(1));
				else
					Dest.fFloatLiteral = (float)pow(Dest.fFloatLiteral, ResolveOpAsFloat(1));

				break;

				// The bitwise Instructionuctions only work with integers. They do nothing
				// when the destination data type is anything else.

				// And

			case Instruction_AND:

				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral &= ResolveOpAsInt(1);

				break;

			// Or
			case Instruction_OR:

				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral |= ResolveOpAsInt(1);

				break;

			// Exclusive Or
			case Instruction_XOR:

				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral ^= ResolveOpAsInt(1);

				break;

			// Shift Left
			case Instruction_SHL:

				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral <<= ResolveOpAsInt(1);

				break;

			// Shift Right
			case Instruction_SHR:

				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral >>= ResolveOpAsInt(1);

				break;
			}

			// Use ResolveOpPntr () to get a pointer to the destination Value structure and move the result there
			//�ô˺�����ȡһ��ָ��Ŀ���������value�ṹ���ָ�룬���������ֵ����
			*ResolveOpPntr(0) = Dest;

			break;
		}

		// ---- Unary Operations

		// These Instructionuctions work much like the binary operations in the sense that
		// they only work with integers and floats (except Not, which works with
		// integers only). Any other destination data type will be ignored.

		case Instruction_NEG:
		case Instruction_NOT:
		case Instruction_INC:
		case Instruction_DEC:
		{
			// Get the destination type (operand index 0)

			int iDestStoreType = GetOpType(0);

			// Get a local copy of the destination itself

			Value Dest = ResolveOpValue(0);

			switch (iOpcode)
			{
				// Negate

			case Instruction_NEG:

				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral = -Dest.iIntLiteral;
				else
					Dest.fFloatLiteral = -Dest.fFloatLiteral;

				break;

				// Not

			case Instruction_NOT:

				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral = ~Dest.iIntLiteral;

				break;

				// Increment

			case Instruction_INC:

				if (Dest.iType == OP_TYPE_INT)
					++Dest.iIntLiteral;
				else
					++Dest.fFloatLiteral;

				break;

				// Decrement

			case Instruction_DEC:

				if (Dest.iType == OP_TYPE_INT)
					--Dest.iIntLiteral;
				else
					--Dest.fFloatLiteral;

				break;
			}

			// Move the result to the destination

			*ResolveOpPntr(0) = Dest;

			break;
		}

		// ---- String Processing

		case Instruction_CONCAT:
		{
			// Get a local copy of the destination operand (operand index 0)

			Value Dest = ResolveOpValue(0);

			// Get a local copy of the source string (operand index 1)

			char* pstrSourceString = ResolveOpAsString(1);

			// If the destination isn't a string, do nothing

			if (Dest.iType != OP_TYPE_STRING)
				break;

			// Determine the length of the new string and allocate space for it (with a
			// null terminator)

			int iNewStringLength = strlen(Dest.pstrStringLiteral) + strlen(pstrSourceString);
			char* pstrNewString = (char*)malloc(iNewStringLength + 1);

			// Copy the old string to the new one

			strcpy(pstrNewString, Dest.pstrStringLiteral);

			// Concatenate the destination with the source

			strcat(pstrNewString, pstrSourceString);

			// Free the existing string in the destination structure and replace it
			// with the new string

			free(Dest.pstrStringLiteral);
			Dest.pstrStringLiteral = pstrNewString;

			// Copy the concatenated string pointer to its destination

			*ResolveOpPntr(0) = Dest;

			break;
		}

		case Instruction_GETCHAR:
		{
			// Get a local copy of the destination operand (operand index 0)

			Value Dest = ResolveOpValue(0);

			// Get a local copy of the source string (operand index 1)

			char* pstrSourceString = ResolveOpAsString(1);

			// Find out whether or not the destination is already a string

			char* pstrNewString;
			if (Dest.iType == OP_TYPE_STRING)
			{
				// If it is, we can use it's existing string buffer as long as it's at
				// least 1 character

				if (strlen(Dest.pstrStringLiteral) >= 1)
				{
					pstrNewString = Dest.pstrStringLiteral;
				}
				else
				{
					free(Dest.pstrStringLiteral);
					pstrNewString = (char*)malloc(2);
				}
			}
			else
			{
				// Otherwise allocate a new string and set the type

				pstrNewString = (char*)malloc(2);
				Dest.iType = OP_TYPE_STRING;
			}

			// Get the index of the character (operand index 2)

			int iSourceIndex = ResolveOpAsInt(2);

			// Copy the character and append a null-terminator

			pstrNewString[0] = pstrSourceString[iSourceIndex];
			pstrNewString[1] = '\0';

			// Set the string pointer in the destination Value structure

			Dest.pstrStringLiteral = pstrNewString;

			// Copy the concatenated string pointer to its destination

			*ResolveOpPntr(0) = Dest;

			break;
		}

		case Instruction_SETCHAR:
		{
			// Get the destination index (operand index 1)

			int iDestIndex = ResolveOpAsInt(1);

			// If the destination isn't a string, do nothing

			if (ResolveOpType(0) != OP_TYPE_STRING)
				break;

			// Get the source character (operand index 2)

			char* pstrSourceString = ResolveOpAsString(2);

			// Set the specified character in the destination (operand index 0)

			ResolveOpPntr(0)->pstrStringLiteral[iDestIndex] = pstrSourceString[0];

			break;
		}

		// ---- Conditional Branching
		//��ָ֧��
		case Instruction_JMP:
		{
			// Get the index of the target Instruction (opcode index 0)
			//��ȡĿ��Ĳ����� ������������0
			int iTargetIndex = ResolveOpAsInstructionIndex(0);

			// Move the Instruction pointer to the target
			//�ƶ�ָ��ָ�루ip����Ŀ������
			g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction = iTargetIndex;

			break;
		}

		//JCCָ��
		//JCC Op0,Op1 ,Target
		case Instruction_JE:
		case Instruction_JNE:
		case Instruction_JG:
		case Instruction_JL:
		case Instruction_JGE:
		case Instruction_JLE:
		{
			// Get the two operands
			//��ȡ����������
			Value Op0 = ResolveOpValue(0);
			Value Op1 = ResolveOpValue(1);

			// Get the index of the target Instruction (opcode index 2)
			//��ȡ����������������תĿ��
			int iTargetIndex = ResolveOpAsInstructionIndex(2);

			// Perform the specified comparison and jump if it evaluates to true
			//���жԱ� ���Ϊ������ת
			int iJump = FALSE;

			switch (iOpcode)
			{
			// Jump if Equal
			case Instruction_JE:
			{
				switch (Op0.iType)
				{
				case OP_TYPE_INT:
					if (Op0.iIntLiteral == Op1.iIntLiteral)
						iJump = TRUE;
					break;

				case OP_TYPE_FLOAT:
					if (Op0.fFloatLiteral == Op1.fFloatLiteral)
						iJump = TRUE;
					break;

				case OP_TYPE_STRING:
					if (strcmp(Op0.pstrStringLiteral, Op1.pstrStringLiteral) == 0)
						iJump = TRUE;
					break;
				}
				break;
			}

			// Jump if Not Equal
			case Instruction_JNE:
			{
				switch (Op0.iType)
				{
				case OP_TYPE_INT:
					if (Op0.iIntLiteral != Op1.iIntLiteral)
						iJump = TRUE;
					break;

				case OP_TYPE_FLOAT:
					if (Op0.fFloatLiteral != Op1.fFloatLiteral)
						iJump = TRUE;
					break;

				case OP_TYPE_STRING:
					if (strcmp(Op0.pstrStringLiteral, Op1.pstrStringLiteral) != 0)
						iJump = TRUE;
					break;
				}
				break;
			}

			// Jump if Greater
			case Instruction_JG:

				if (Op0.iType == OP_TYPE_INT)
				{
					if (Op0.iIntLiteral > Op1.iIntLiteral)
						iJump = TRUE;
				}
				else
				{
					if (Op0.fFloatLiteral > Op1.fFloatLiteral)
						iJump = TRUE;
				}

				break;

			// Jump if Less
			case Instruction_JL:

				if (Op0.iType == OP_TYPE_INT)
				{
					if (Op0.iIntLiteral < Op1.iIntLiteral)
						iJump = TRUE;
				}
				else
				{
					if (Op0.fFloatLiteral < Op1.fFloatLiteral)
						iJump = TRUE;
				}

				break;

			// Jump if Greater or Equal
			case Instruction_JGE:

				if (Op0.iType == OP_TYPE_INT)
				{
					if (Op0.iIntLiteral >= Op1.iIntLiteral)
						iJump = TRUE;
				}
				else
				{
					if (Op0.fFloatLiteral >= Op1.fFloatLiteral)
						iJump = TRUE;
				}

				break;

			// Jump if Less or Equal
			case Instruction_JLE:

				if (Op0.iType == OP_TYPE_INT)
				{
					if (Op0.iIntLiteral <= Op1.iIntLiteral)
						iJump = TRUE;
				}
				else
				{
					if (Op0.fFloatLiteral <= Op1.fFloatLiteral)
						iJump = TRUE;
				}

				break;
			}

			// If the comparison evaluated to TRUE, make the jump
			//���Ϊ�棬��ת���ƶ�ip
			if (iJump)
				g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction = iTargetIndex;

			break;
		}

		// ---- The Stack Interface

		case Instruction_PUSH:
		{
			// Get a local copy of the source operand (operand index 0)

			Value Source = ResolveOpValue(0);

			// Push the value onto the stack

			Push(g_iCurrentThread, Source);

			break;
		}

		case Instruction_POP:
		{
			// Pop the top of the stack into the destination

			*ResolveOpPntr(0) = Pop(g_iCurrentThread);

			break;
		}


		//�������� �ӿ�
		case Instruction_CALL:
		{
			// Get a local copy of the function index
			//��ȡ��������
			int iFuncIndex = ResolveOpAsFuncIndex(0);

			// Advance the Instruction pointer so it points to the Instruction immediately following the call

			++g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction;

			// Call the function
			//���ú���
			CallFunc(g_iCurrentThread, iFuncIndex);

			break;
		}

		//Retָ��
		case Instruction_RET:
		{
			// Get the Current function index off the top of the stack and use it to get the corresponding function structure
			//�ӵ�ǰջ��������������������ȡ��غ����ṹ����Ϣ
			Value FuncIndex = Pop(g_iCurrentThread);

			// Check for the presence of a stack base marker

			if (FuncIndex.iType = OP_TYPE_STACK_BASE_MARKER)
				iExitExecLoop = TRUE;

			// Get the previous function index
			//��ȡǰһ������������
			Func CurrentFunc = GetFunc(g_iCurrentThread, FuncIndex.iFuncIndex);
			int iFrameIndex = FuncIndex.iOffsetIndex;

			// Read the return address structure from the stack, which is stored one index below the local data
			//�Ӷ�ջ�ж�ȡ���ص�ַ�Ľṹ�����洢�ھֲ������·���������
			Value ReturnAddr = GetStackValue(g_iCurrentThread, g_Scripts[g_iCurrentThread].Stack.iTopIndex - (CurrentFunc.iLocalDataSize + 1));

			// Pop the stack frame along with the return address
			//��ջ֡���� ��ͬ���ص�ַ
			PopFrame(CurrentFunc.iStackFrameSize);

			// Restore the previous frame index
			//�洢֮ǰ�Ķ�ջ����
			g_Scripts[g_iCurrentThread].Stack.iFrameIndex = iFrameIndex;

			// Make the jump to the return address
			//��ת�����ص�ַ
			g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction = ReturnAddr.iInstructionIndex;

			break;
		}

		case Instruction_CALLHOST:
		{
			// Use operand zero to index into the host API call table and get the host API function name
			//ʹ�ò�����0��ȡhost api���ñ��������һ�ú�������
			Value HostAPICall = ResolveOpValue(0);
			int iHostAPICallIndex = HostAPICall.iHostAPICallIndex;

			// Get the name of the host API function
			//��ȡ��������
			char* pstrFuncName = GetHostAPICall(iHostAPICallIndex);

			// Search through the host API until the matching function is found
			//����host apiֱ���ҵ�ƥ���
			int HostAPIFuncIndex;
			int iMatchFound = FALSE;
			for (int iHostAPIFuncIndex = 0; iHostAPIFuncIndex < MAX_HOST_API_SIZE; ++iHostAPIFuncIndex)
			{
				// Get a pointer to the name of the Current host API function
				//��ȡָ��ǰhost api�������Ƶ�ָ��
				char* pstrCurrentHostAPIFunc = g_HostAPI[iHostAPIFuncIndex].pstrName;

				// If it equals the requested name, it might be a match

				if (strcmp(pstrFuncName, pstrCurrentHostAPIFunc) == 0)
				{
					// Make sure the function is visible to the Current thread
					//ȷ���ú����Ե�ǰ�߳̿ɼ�
					int iThreadIndex = g_HostAPI[iHostAPIFuncIndex].iThreadIndex;
					if (iThreadIndex == g_iCurrentThread || iThreadIndex == XS_GLOBAL_FUNC)
					{
						iMatchFound = TRUE;
						HostAPIFuncIndex = iHostAPIFuncIndex;
						break;
					}
				}
			}

			// If a match was found, call the host API funcfion and pass the Current thread index
			//ƥ�䵽�����ã����ݵ���ǰ�߳�����
			if (iMatchFound)
				//g_HostAPI[iHostAPIFuncIndex].fnFunc(g_iCurrentThread);
				g_HostAPI[HostAPIFuncIndex].fnFunc(g_iCurrentThread);

			break;
		}

		// ---- Misc
		//��ͣ
		case Instruction_PAUSE:
		{
			// Get the pause duration
			//��ȡ��ʱ����ʱ��
			int iPauseDuration = ResolveOpAsInt(0);

			// Determine the ending pause time
			//ȷ����ͣ����ʱ��=��ǰʱ��+����ʱ��
			g_Scripts[g_iCurrentThread].iPauseEndTime = iCurrentTime + iPauseDuration;

			// Pause the script
			//��ͣ�ű�
			g_Scripts[g_iCurrentThread].iIsPaused = TRUE;

			break;
		}

		case Instruction_EXIT:
		{
			// Resolve operand zero to find the exit code
			//����������������exit����
			Value ExitCode = ResolveOpValue(0);

			// Get it from the integer field
			//�������л�ȡ
			int iExitCode = ExitCode.iIntLiteral;

			// Tell the XVM to stop executing the script
			//���������ֹͣ����
			g_Scripts[g_iCurrentThread].iIsRunning = FALSE;

			break;
		}
		}

		// If the Instruction pointer hasn't been changed by an Instruction, increment it
		//��ָ���ִ���ڼ�ip�Ƿ񱻸ı䣿 
		if (iCurrentInstruction == g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction)
			++g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction;//ipδ���ı䣬��������ָ����һ��ָ��

		// If we aren't running indefinitely, check to see if the main timeslice has ended

		if (iTimesliceDur != XS_INFINITE_TIMESLICE)
			if (iCurrentTime > iMainTimesliceStartTime + iTimesliceDur)
				break;

		// Exit the execution loop if the script has terminated

		if (iExitExecLoop)
			break;
	}
}

/// <summary>
/// Starts the execution of a script.
/// </summary>
/// <param name="iThreadIndex"></param>
void XS_StartScript(int iThreadIndex)
{
	// Make sure the thread index is valid and active
	//�߳������Ϸ���Ч
	if (!IsThreadActive(iThreadIndex))
		return;

	// Set the thread's execution flag
	//�����̵߳�ִ�б�־
	g_Scripts[iThreadIndex].iIsRunning = TRUE;

	// Set the Current thread to the script
	//����ǰ�̷߳�����ű�
	g_iCurrentThread = iThreadIndex;

	// Set the activation time for the Current thread to get things rolling
	//��Ƶ�ǰ�̻߳�Ծʱ��
	g_iCurrentThreadActiveTime = GetCurrTime();
}

/// <summary>
/// Stops the execution of a script.
/// </summary>
/// <param name="iThreadIndex"></param>
void XS_StopScript(int iThreadIndex)
{
	// Make sure the thread index is valid and active

	if (!IsThreadActive(iThreadIndex))
		return;

	// Clear the thread's execution flag

	g_Scripts[iThreadIndex].iIsRunning = FALSE;
}


/// <summary>
/// Pauses a script for a specified duration.
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iDur"></param>
void XS_PauseScript(int iThreadIndex, int iDur)
{
	// Make sure the thread index is valid and active

	if (!IsThreadActive(iThreadIndex))
		return;

	// Set the pause flag

	g_Scripts[iThreadIndex].iIsPaused = TRUE;

	// Set the duration of the pause

	g_Scripts[iThreadIndex].iPauseEndTime = GetCurrTime() + iDur;
}

/// <summary>
/// XS_UnpauseScript ()
/// </summary>
/// <param name="iThreadIndex"></param>
void XS_UnpauseScript(int iThreadIndex)
{
	// Make sure the thread index is valid and active

	if (!IsThreadActive(iThreadIndex))
		return;

	// Clear the pause flag

	g_Scripts[iThreadIndex].iIsPaused = FALSE;
}

/// <summary>
/// Returns the last returned value as an integer.
/// </summary>
/// <param name="iThreadIndex"></param>
/// <returns></returns>
int XS_GetReturnValueAsInt(int iThreadIndex)
{
	// Make sure the thread index is valid and active

	if (!IsThreadActive(iThreadIndex))
		return 0;

	// Return _RetVal's integer field

	return g_Scripts[iThreadIndex]._RetVal.iIntLiteral;
}


/// <summary>
/// Returns the last returned value as an float.
/// </summary>
/// <param name="iThreadIndex"></param>
/// <returns></returns>
float XS_GetReturnValueAsFloat(int iThreadIndex)
{
	// Make sure the thread index is valid and active

	if (!IsThreadActive(iThreadIndex))
		return 0;

	// Return _RetVal's floating-point field

	return g_Scripts[iThreadIndex]._RetVal.fFloatLiteral;
}


/// <summary>
/// Returns the last returned value as a string.
/// </summary>
/// <param name="iThreadIndex"></param>
/// <returns></returns>
char* XS_GetReturnValueAsString(int iThreadIndex)
{
	// Make sure the thread index is valid and active

	if (!IsThreadActive(iThreadIndex))
		return NULL;

	// Return _RetVal's string field

	return g_Scripts[iThreadIndex]._RetVal.pstrStringLiteral;
}


/// <summary>
/// Copies a value structure to another, taking strings into account.
/// ����ֵ
/// </summary>
/// <param name="pDest"></param>
/// <param name="Source"></param>
void CopyValue(Value* pDest, Value Source)
{
	// If the destination already contains a string, make sure to free it first
	//���Ŀ���Ѿ�����һ���ַ��������ͷ���
	if (pDest->iType == OP_TYPE_STRING)
		free(pDest->pstrStringLiteral);

	// Copy the object
	//�����������
	*pDest = Source;

	// Make a physical copy of the source string, if necessary
	//�б�Ҫ�Ļ�����һ��Դ�ַ�����������
	if (Source.iType == OP_TYPE_STRING)
	{
		pDest->pstrStringLiteral = (char*)malloc(strlen(Source.pstrStringLiteral) + 1);
		strcpy(pDest->pstrStringLiteral, Source.pstrStringLiteral);
	}
}


/// <summary>
/// Coerces a Value structure from it's Current type to an integer value.
/// ǿ�ƽ�value�ṹת��Ϊint
/// </summary>
/// <param name="Val"></param>
/// <returns></returns>
int CoerceValueToInt(Value Val)
{
	// Determine which type the Value Currently is
	//�жϵ�ǰֵ��ʲô����
	switch (Val.iType)
	{
	// It's an integer, so return it as-is
    //���ͷ������Լ�
	case OP_TYPE_INT:
		return Val.iIntLiteral;

	// It's a float, so cast it to an integer
    //����ת��
	case OP_TYPE_FLOAT:
		return (int)Val.fFloatLiteral;

	// It's a string, so convert it to an integer
	//�ַ���ת��
	case OP_TYPE_STRING:
		return atoi(Val.pstrStringLiteral);

	// Anything else is invalid
    //�������Ϸ�
	default:
		return 0;
	}
}

/// <summary>
/// Coerces a Value structure from it's Current type to an float value.
/// ǿ�ƽ�value�ṹ��תΪ����
/// </summary>
/// <param name="Val"></param>
/// <returns></returns>
float CoerceValueToFloat(Value Val)
{
	// Determine which type the Value Currently is
	//�жϵ�ǰֵ������
	switch (Val.iType)
	{
	// It's an integer, so cast it to a float
	//intǿתΪfloat
	case OP_TYPE_INT:
		return (float)Val.iIntLiteral;

	// It's a float, so return it as-is
	//���㣬��������
	case OP_TYPE_FLOAT:
		return Val.fFloatLiteral;

	// It's a string, so convert it to an float

	case OP_TYPE_STRING:
		return (float)atof(Val.pstrStringLiteral);

		// Anything else is invalid

	default:
		return 0;
	}
}


/// <summary>
/// Coerces a Value structure from it's Current type to a string value.
/// ǿתΪstring
/// </summary>
/// <param name="Val"></param>
/// <returns></returns>
char* CoerceValueToString(Value Val)
{

	char* pstrCoercion = NULL;
	if (Val.iType != OP_TYPE_STRING)
		pstrCoercion = (char*)malloc(MAX_COERCION_STRING_SIZE + 1);

	// Determine which type the Value Currently is

	switch (Val.iType)
	{
		// It's an integer, so convert it to a string

	case OP_TYPE_INT:
		_itoa(Val.iIntLiteral, pstrCoercion, 10);
		return pstrCoercion;

		// It's a float, so use sprintf () to convert it since there's no built-in function
		// for converting floats to strings

	case OP_TYPE_FLOAT:
		sprintf(pstrCoercion, "%f", Val.fFloatLiteral);
		return pstrCoercion;

		// It's a string, so return it as-is

	case OP_TYPE_STRING:
		return Val.pstrStringLiteral;

		// Anything else is invalid

	default:
		return NULL;
	}
}



/// <summary>
/// ���ص�ǰָ����ָ���Ĳ�����������
/// Returns the type of the specified operand in the Current Instruction.
/// </summary>
/// <param name="iOpIndex"></param>
/// <returns></returns>
inline int GetOpType(int iOpIndex)
{
	// Get the Current Instruction
	//��ȡ��ǰָ��
	int iCurrentInstruction = g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction;

	// Return the type
	//��������
	return g_Scripts[g_iCurrentThread].InstructionStream.pInstructions[iCurrentInstruction].pOpList[iOpIndex].iType;
}

/// <summary>
/// Resolves an operand's stack index, whether it's absolute or relative.
/// </summary>
/// <param name="iOpIndex"></param>
/// <returns></returns>
inline int ResolveOpStackIndex(int iOpIndex)
{
	// Get the Current Instruction
	//��ȡ��ǰָ��
	int iCurrentInstruction = g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction;

	// Get the operand type type
	//��ȡ����������
	Value OpValue = g_Scripts[g_iCurrentThread].InstructionStream.pInstructions[iCurrentInstruction].pOpList[iOpIndex];

	// Resolve the stack index based on its type
	
	switch (OpValue.iType)
	{
	// It's an absolute index so return it as-is
    //���Զ�ջ����
	case OP_TYPE_ABS_STACK_INDEX:
		return OpValue.iStackIndex;

	// It's a relative index so resolve it
	//��Զ�ջ����
	case OP_TYPE_REL_STACK_INDEX:
	{
		// First get the base index

		int iBaseIndex = OpValue.iStackIndex;

		// Now get the index of the variable

		int iOffsetIndex = OpValue.iOffsetIndex;

		// Get the variable's value

		Value StackValue = GetStackValue(g_iCurrentThread, iOffsetIndex);

		// Now add the variable's integer field to the base index to produce the
		// absolute index

		return iBaseIndex + StackValue.iIntLiteral;
	}

	// Return zero for everything else, but we shouldn't encounter this case

	default:
		return 0;
	}
}


/// <summary>
/// Resolves an operand and returns it's associated Value structure.
/// </summary>
/// <param name="iOpIndex"></param>
/// <returns></returns>
inline Value ResolveOpValue(int iOpIndex)
{
	// Get the Current Instruction
	//��ȡ��ǰָ��
	int iCurrentInstruction = g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction;

	// Get the operand type
	//��ȡ��ǰ����������
	Value OpValue = g_Scripts[g_iCurrentThread].InstructionStream.pInstructions[iCurrentInstruction].pOpList[iOpIndex];

	// Determine what to return based on the value's type
	//�������;�������ʲô����
	switch (OpValue.iType)
	{
	// It's a stack index so resolve it
	//��ջ���� ���Խ�����
	case OP_TYPE_ABS_STACK_INDEX:
	case OP_TYPE_REL_STACK_INDEX:
	{
		// Resolve the index and use it to return the corresponding stack element
		//�����������ҷ�����Ӧ�Ķ�ջԪ��
		int iAbsIndex = ResolveOpStackIndex(iOpIndex);
		return GetStackValue(g_iCurrentThread, iAbsIndex);
	}

	// It's in _RetVal
	//��_RetVal �Ĵ�����
	case OP_TYPE_REG:
		return g_Scripts[g_iCurrentThread]._RetVal;

		// Anything else can be returned as-is
		//�������Ͳ�������������
	default:
		return OpValue;
	}
}

/// <summary>
/// Resolves the type of the specified operand in the Current Instruction and returns the resolved type.
/// </summary>
/// <param name="iOpIndex"></param>
/// <returns></returns>
inline int ResolveOpType(int iOpIndex)
{
	// Resolve the operand's value

	Value OpValue = ResolveOpValue(iOpIndex);

	// Return the value type

	return OpValue.iType;
}

/// <summary>
/// Resolves and coerces an operand's value to an integer value.
/// </summary>
/// <param name="iOpIndex"></param>
/// <returns></returns>
inline int ResolveOpAsInt(int iOpIndex)
{
	// Resolve the operand's value

	Value OpValue = ResolveOpValue(iOpIndex);

	// Coerce it to an int and return it

	int iInt = CoerceValueToInt(OpValue);
	return iInt;
}

/// <summary>
/// Resolves and coerces an operand's value to a floating-point value.
/// </summary>
/// <param name="iOpIndex"></param>
/// <returns></returns>
inline float ResolveOpAsFloat(int iOpIndex)
{
	// Resolve the operand's value

	Value OpValue = ResolveOpValue(iOpIndex);

	// Coerce it to a float and return it

	float fFloat = CoerceValueToFloat(OpValue);
	return fFloat;
}

/// <summary>
/// Resolves and coerces an operand's value to a string value, allocating the space for a new string if necessary.
/// </summary>
/// <param name="iOpIndex"></param>
/// <returns></returns>
inline char* ResolveOpAsString(int iOpIndex)
{
	// Resolve the operand's value

	Value OpValue = ResolveOpValue(iOpIndex);

	// Coerce it to a string and return it

	char* pstrString = CoerceValueToString(OpValue);
	return pstrString;
}

/// <summary>
/// Resolves an operand as an intruction index.
/// </summary>
/// <param name="iOpIndex"></param>
/// <returns></returns>
inline int ResolveOpAsInstructionIndex(int iOpIndex)
{
	// Resolve the operand's value

	Value OpValue = ResolveOpValue(iOpIndex);

	// Return it's Instruction index

	return OpValue.iInstructionIndex;
}

/// <summary>
/// Resolves an operand as a function index.
/// </summary>
/// <param name="iOpIndex"></param>
/// <returns></returns>
inline int ResolveOpAsFuncIndex(int iOpIndex)
{
	// Resolve the operand's value

	Value OpValue = ResolveOpValue(iOpIndex);

	// Return the function index

	return OpValue.iFuncIndex;
}

/// <summary>
/// Resolves an operand as a host API call
/// </summary>
/// <param name="iOpIndex"></param>
/// <returns></returns>
inline char* ResolveOpAsHostAPICall(int iOpIndex)
{
	// Resolve the operand's value

	Value OpValue = ResolveOpValue(iOpIndex);

	// Get the value's host API call index

	int iHostAPICallIndex = OpValue.iHostAPICallIndex;

	// Return the host API call

	return GetHostAPICall(iHostAPICallIndex);
}



/// <summary>
/// Resolves an operand and returns a pointer to its Value structure.
/// 
/// </summary>
/// <param name="iOpIndex"></param>
/// <returns></returns>
inline Value* ResolveOpPntr(int iOpIndex)
{
	// Get the method of indirection
	//��ȡ����������
	int iIndirMethod = GetOpType(iOpIndex);

	// Return a pointer to wherever the operand lies
	//��������λ�÷���һ��ָ��
	switch (iIndirMethod)
	{
	// It's on the stack
	//��ջ��
	case OP_TYPE_ABS_STACK_INDEX:
	case OP_TYPE_REL_STACK_INDEX:
	{
		int iStackIndex = ResolveOpStackIndex(iOpIndex);
		return &g_Scripts[g_iCurrentThread].Stack.pElmnts[ResolveStackIndex(iStackIndex)];
	}

	// It's _RetVal
	//�ڷ��ؼĴ�����
	case OP_TYPE_REG:
		return &g_Scripts[g_iCurrentThread]._RetVal;
	}

	// Return NULL for anything else

	return NULL;
}



/// <summary>
/// Returns the specified stack value.
/// ����ָ���Ķ�ջ��ֵ
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iIndex"></param>
/// <returns></returns>
inline Value GetStackValue(int iThreadIndex, int iIndex)
{
	// Use ResolveStackIndex () to return the element at the specified index

	return g_Scripts[iThreadIndex].Stack.pElmnts[ResolveStackIndex(iIndex)];
}


/// <summary>
/// Sets the specified stack value.
/// �趨ָ���Ķ�ջ��ֵ
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iIndex"></param>
/// <param name="Val"></param>
inline void SetStackValue(int iThreadIndex, int iIndex, Value Val)
{
	// Use ResolveStackIndex () to set the element at the specified index

	g_Scripts[iThreadIndex].Stack.pElmnts[ResolveStackIndex(iIndex)] = Val;
}



/// <summary>
/// Pushes an element onto the stack.
/// ѹջ
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="Val"></param>
inline void Push(int iThreadIndex, Value Val)
{
	// Get the Current top element
	//��ȡջ��Ԫ��
	int iTopIndex = g_Scripts[iThreadIndex].Stack.iTopIndex;

	// Put the value into the Current top index
	//��ֵ���뵱ǰջ��������
	CopyValue(&g_Scripts[iThreadIndex].Stack.pElmnts[iTopIndex], Val);

	// Increment the top index
	//����ջ������
	++g_Scripts[iThreadIndex].Stack.iTopIndex;
}

/// <summary>
/// Pops the element off the top of the stack.
/// ��ջ
/// </summary>
/// <param name="iThreadIndex"></param>
/// <returns></returns>
inline Value Pop(int iThreadIndex)
{
	// Decrement the top index to clear the old element for overwriting
	//��Сջ������
	--g_Scripts[iThreadIndex].Stack.iTopIndex;

	// Get the Current top element

	int iTopIndex = g_Scripts[iThreadIndex].Stack.iTopIndex;

	// Use this index to read the top element

	Value Val;
	CopyValue(&Val, g_Scripts[iThreadIndex].Stack.pElmnts[iTopIndex]);

	// Return the value to the caller

	return Val;
}

/// <summary>
/// Pushes a stack frame.
/// ѹ��ջ֡
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iSize"></param>
inline void PushFrame(int iThreadIndex, int iSize)
{
	// Increment the top index by the size of the frame
	//����֡�Ĵ�С��ջ������
	g_Scripts[iThreadIndex].Stack.iTopIndex += iSize;

	// Move the frame index to the new top of the stack
	//��ջ֡�����Ƶ���ջ��
	g_Scripts[iThreadIndex].Stack.iFrameIndex = g_Scripts[iThreadIndex].Stack.iTopIndex;
}


/// <summary>
/// Pops a stack frame.
/// ����ջ֡
/// </summary>
/// <param name="iSize"></param>
inline void PopFrame(int iSize)
{
	// Decrement the top index by the size of the frame

	g_Scripts[g_iCurrentThread].Stack.iTopIndex -= iSize;

	// Move the frame index to the new top of the stack
}



/// <summary>
/// Returns the function corresponding to the specified index.
/// ���������Ӻ������ж�ȡ����
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iIndex"></param>
/// <returns></returns>
inline Func GetFunc(int iThreadIndex, int iIndex)
{
	return g_Scripts[iThreadIndex].FuncTable.pFuncs[iIndex];
}


/// <summary>
/// Returns the host API call corresponding to the specified index.
/// ����������host api���ñ��ж�ȡ����
/// </summary>
/// <param name="iIndex"></param>
/// <returns></returns>
inline char* GetHostAPICall(int iIndex)
{
	return g_Scripts[g_iCurrentThread].HostAPICallTable.ppstrCalls[iIndex];
}


/// <summary>
/// Wrapper for the system-dependant method of determining the Current time in milliseconds.
/// ��ȡ��ǰʱ��
/// </summary>
/// <returns></returns>
inline int GetCurrTime()
{
	// This function is Currently implemented with the WinAPI function GetTickCount ().
	// Change this line to make it compatible with other systems.

	return GetTickCount64();
}



/// <summary>
/// Calls a function based on its index.
/// ���ú���
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iIndex"></param>
void CallFunc(int iThreadIndex, int iIndex)
{
	//��������
	//�ֲ�����
	//���ص�ַ 
	//���� 
	
	//��ȡ��ǰĿ�꺯����Ϣ
	Func DestFunc = GetFunc(iThreadIndex, iIndex);

	// Save the Current stack frame index
	//���浱ǰջ֡����
	int iFrameIndex = g_Scripts[iThreadIndex].Stack.iFrameIndex;

	// Push the return address, which is the Current Instruction
	//push���ص�ַ������ǰָ���ַ
	Value ReturnAddr;
	ReturnAddr.iInstructionIndex = g_Scripts[iThreadIndex].InstructionStream.iCurrentInstruction;
	Push(iThreadIndex, ReturnAddr);

	// Push the stack frame + 1 (the extra space is for the function index we'll put on the stack after it
	//ѹ��ջ֡��С+1,����ռ��Ǹ���������ʹ�õ�
	PushFrame(iThreadIndex, DestFunc.iLocalDataSize + 1);

	// Write the function index and old stack frame to the top of the stack
	//�����������;ɵ�ջ֡����д��ջ��
	Value FuncIndex;
	FuncIndex.iFuncIndex = iIndex;//��������
	FuncIndex.iOffsetIndex = iFrameIndex;//�ɵ�ջ֡����
	SetStackValue(iThreadIndex, g_Scripts[iThreadIndex].Stack.iTopIndex - 1, FuncIndex);

	// Let the caller make the jump to the entry point
	//ת��������ڵ�
	g_Scripts[iThreadIndex].InstructionStream.iCurrentInstruction = DestFunc.iEntryPoint;
}

/// <summary>
/// Passes an integer parameter from the host to a script function.
/// ��host���򴫵�һ��int�������ű�����
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iInt"></param>
void XS_PassIntParam(int iThreadIndex, int iInt)
{
	// Create a Value structure to encapsulate the parameter
	//����value�ṹ��װ����
	Value Param;
	Param.iType = OP_TYPE_INT;
	Param.iIntLiteral = iInt;

	// Push the parameter onto the stack
	//ѹջ����
	Push(iThreadIndex, Param);
}

/// <summary>
/// Passes a floating-point parameter from the host to a script function.
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="fFloat"></param>
void XS_PassFloatParam(int iThreadIndex, float fFloat)
{
	// Create a Value structure to encapsulate the parameter

	Value Param;
	Param.iType = OP_TYPE_FLOAT;
	Param.fFloatLiteral = fFloat;

	// Push the parameter onto the stack

	Push(iThreadIndex, Param);
}

/// <summary>
/// Passes a string parameter from the host to a script function.
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="pstrString"></param>
void XS_PassStringParam(int iThreadIndex, char* pstrString)
{
	// Create a Value structure to encapsulate the parameter

	Value Param;
	Param.iType = OP_TYPE_STRING;
	Param.pstrStringLiteral = (char*)malloc(strlen(pstrString) + 1);
	strcpy(Param.pstrStringLiteral, pstrString);

	// Push the parameter onto the stack

	Push(iThreadIndex, Param);
}

/// <summary>
/// Returns the index into the function table corresponding to the specified name.
/// �������ƻ�ȡ��������
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="pstrName"></param>
/// <returns></returns>
int GetFuncIndexByName(int iThreadIndex, char* pstrName)
{
	// Loop through each function and look for a matching name
	//ƥ������
	for (int iFuncIndex = 0; iFuncIndex < g_Scripts[iThreadIndex].FuncTable.iSize; ++iFuncIndex)
	{
		// If the names match, return the index

		if (_stricmp(pstrName, g_Scripts[iThreadIndex].FuncTable.pFuncs[iFuncIndex].pstrName) == 0)
			return iFuncIndex;
	}

	// A match wasn't found, so return -1

	return -1;
}

/// <summary>
/// Calls a script function from the host application.
/// ��host api�е��ýű�����
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="pstrName"></param>
void XS_CallScriptFunc(int iThreadIndex, char* pstrName)
{
	// Make sure the thread index is valid and active

	if (!IsThreadActive(iThreadIndex))
		return;

	// ---- Calling the function

	// Preserve the Current state of the VM
	//���浱ǰ�����״̬
	int iPrevThreadMode = g_iCurrentThreadMode;
	int iPrevThread = g_iCurrentThread;

	// Set the threading mode for single-threaded execution
	//�����߳�Ϊ���߳�ģʽ
	g_iCurrentThreadMode = THREAD_MODE_SINGLE;

	// Set the active thread to the one specified
	//����Ծ�߳�����Ϊָ������
	g_iCurrentThread = iThreadIndex;

	// Get the function's index based on it's name
	//��ȡ��������
	int iFuncIndex = GetFuncIndexByName(iThreadIndex, pstrName);

	// Make sure the function name was valid

	if (iFuncIndex == -1)
		return;

	// Call the function
	//����
	CallFunc(iThreadIndex, iFuncIndex);

	// Set the stack base
	//����ջ��ַ
	Value StackBase = GetStackValue(g_iCurrentThread, g_Scripts[g_iCurrentThread].Stack.iTopIndex - 1);
	StackBase.iType = OP_TYPE_STACK_BASE_MARKER;
	SetStackValue(g_iCurrentThread, g_Scripts[g_iCurrentThread].Stack.iTopIndex - 1, StackBase);

	// Allow the script code to execute uninterrupted until the function returns
	//���нű���ִͣ��ֱ������
	XS_RunScripts(XS_INFINITE_TIMESLICE);

	//! ����������

	// Restore the VM state
	//���������״̬
	g_iCurrentThreadMode = iPrevThreadMode;
	g_iCurrentThread = iPrevThread;
}

/// <summary>
/// Invokes a script function from the host application, meaning the call executes in sync with the script.
/// ͬ�����ýű�����
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="pstrName"></param>
void XS_InvokeScriptFunc(int iThreadIndex, char* pstrName)
{
	// Make sure the thread index is valid and active
	//ȷ���߳������Ϸ�
	if (!IsThreadActive(iThreadIndex))
		return;

	// Get the function's index based on its name
	//��ȡ��������
	int iFuncIndex = GetFuncIndexByName(iThreadIndex, pstrName);

	// Make sure the function name was valid
	//ȷ���������Ϸ�
	if (iFuncIndex == -1)
		return;

	// Call the function
	//���ú���
	CallFunc(iThreadIndex, iFuncIndex);
}


/// <summary>
/// Registers a function with the host API.
/// Ϊhost apiע�ắ��
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="pstrName"></param>
/// <param name="fnFunc"></param>
void XS_RegisterHostAPIFunc(int iThreadIndex, char* pstrName, HostAPIFuncPntr fnFunc)
{
	// Loop through each function in the host API until a free index is found
	//ѭ��host api���ҵ���������
	for (int iCurrentHostAPIFunc = 0; iCurrentHostAPIFunc < MAX_HOST_API_SIZE; ++iCurrentHostAPIFunc)
	{
		// If the Current index is free, use it
		//��ǰ�������У�ʹ��
		if (!g_HostAPI[iCurrentHostAPIFunc].iIsActive)
		{
			// Set the function's parameters
			//���ú�������
			g_HostAPI[iCurrentHostAPIFunc].iThreadIndex = iThreadIndex;
			g_HostAPI[iCurrentHostAPIFunc].pstrName = (char*)malloc(strlen(pstrName) + 1);
			strcpy(g_HostAPI[iCurrentHostAPIFunc].pstrName, pstrName);
			_strupr(g_HostAPI[iCurrentHostAPIFunc].pstrName);
			g_HostAPI[iCurrentHostAPIFunc].fnFunc = fnFunc;

			// Set the function to active
			//���ú�������
			g_HostAPI[iCurrentHostAPIFunc].iIsActive = TRUE;
		}
	}
}


/// <summary>
/// Returns the specified integer parameter to a host API function.
/// ������ת��������host api
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iParamIndex"></param>
/// <returns></returns>
int XS_GetParamAsInt(int iThreadIndex, int iParamIndex)
{
	// Get the Current top element
	//��ȡջ��Ԫ��
	int iTopIndex = g_Scripts[g_iCurrentThread].Stack.iTopIndex;
	Value Param = g_Scripts[iThreadIndex].Stack.pElmnts[iTopIndex - (iParamIndex + 1)];

	// Coerce the top element of the stack to an integer
	//��ջ��Ԫ��ǿתΪ����
	int iInt = CoerceValueToInt(Param);

	// Return the value

	return iInt;
}


/// <summary>
/// Returns the specified floating-point parameter to a host API function.
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iParamIndex"></param>
/// <returns></returns>
float XS_GetParamAsFloat(int iThreadIndex, int iParamIndex)
{
	// Get the Current top element

	int iTopIndex = g_Scripts[g_iCurrentThread].Stack.iTopIndex;
	Value Param = g_Scripts[iThreadIndex].Stack.pElmnts[iTopIndex - (iParamIndex + 1)];

	// Coerce the top element of the stack to a float

	float fFloat = CoerceValueToFloat(Param);

	// Return the value

	return fFloat;
}

/// <summary>
/// Returns the specified string parameter to a host API function.
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iParamIndex"></param>
/// <returns></returns>
char* XS_GetParamAsString(int iThreadIndex, int iParamIndex)
{
	// Get the Current top element

	int iTopIndex = g_Scripts[g_iCurrentThread].Stack.iTopIndex;
	Value Param = g_Scripts[iThreadIndex].Stack.pElmnts[iTopIndex - (iParamIndex + 1)];

	// Coerce the top element of the stack to a string

	char* pstrString = CoerceValueToString(Param);

	// Return the value

	return pstrString;
}

/// <summary>
/// Returns from a host API function.
/// ��host api�����з���
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iParamCount"></param>
void XS_ReturnFromHost(int iThreadIndex, int iParamCount)
{
	// Clear the parameters off the stack
	//�����ջ�еĲ���
	g_Scripts[iThreadIndex].Stack.iTopIndex -= iParamCount;
}

/// <summary>
/// Returns an integer from a host API function.
/// ��host api��������һ��int
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iParamCount"></param>
/// <param name="iInt"></param>
void XS_ReturnIntFromHost(int iThreadIndex, int iParamCount, int iInt)
{
	// Clear the parameters off the stack
	//�Ӷ�ջ���������
	g_Scripts[iThreadIndex].Stack.iTopIndex -= iParamCount;

	// Put the return value and type in _RetVal
	//������ֵ�����ͷ��� Ret�Ĵ���
	g_Scripts[iThreadIndex]._RetVal.iType = OP_TYPE_INT;
	g_Scripts[iThreadIndex]._RetVal.iIntLiteral = iInt;
}

/// <summary>
/// Returns a float from a host API function.
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iParamCount"></param>
/// <param name="fFloat"></param>
void XS_ReturnFloatFromHost(int iThreadIndex, int iParamCount, float fFloat)
{
	// Clear the parameters off the stack

	g_Scripts[iThreadIndex].Stack.iTopIndex -= iParamCount;

	// Put the return value and type in _RetVal

	g_Scripts[iThreadIndex]._RetVal.iType = OP_TYPE_FLOAT;
	g_Scripts[iThreadIndex]._RetVal.fFloatLiteral = fFloat;
}

/// <summary>
/// Returns a string from a host API function.
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iParamCount"></param>
/// <param name="pstrString"></param>
void XS_ReturnStringFromHost(int iThreadIndex, int iParamCount, char* pstrString)
{
	// Clear the parameters off the stack

	g_Scripts[iThreadIndex].Stack.iTopIndex -= iParamCount;

	// Put the return value and type in _RetVal

	Value ReturnValue;
	ReturnValue.iType = OP_TYPE_STRING;
	ReturnValue.pstrStringLiteral = pstrString;
	CopyValue(&g_Scripts[iThreadIndex]._RetVal, ReturnValue);
}