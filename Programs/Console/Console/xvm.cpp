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
#define OP_TYPE_FLOAT               1           // Floating-point literal value 浮点
#define OP_TYPE_STRING		        2           // String literal value 字符串
#define OP_TYPE_ABS_STACK_INDEX     3           // Absolute array index 绝对数组索引
#define OP_TYPE_REL_STACK_INDEX     4           // Relative array index 相对数组索引
#define OP_TYPE_Instruction_INDEX         5           // Instruction index 指令索引
#define OP_TYPE_FUNC_INDEX          6           // Function index 函数索引
#define OP_TYPE_HOST_API_CALL_INDEX 7           // Host API call index host api 调用索引
#define OP_TYPE_REG                 8           // Register 寄存器

#define OP_TYPE_STACK_BASE_MARKER   9           // Marks a stack base 堆栈基址标记


//! 指令的opcode
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

//! 多线程

#define THREAD_MODE_MULTI           0           // Multithreaded execution 多线程执行
#define THREAD_MODE_SINGLE          1           // Single-threaded execution 单线程执行

#define THREAD_PRIORITY_DUR_LOW     20          // Low-priority thread timeslice 低优先级线程时间片
#define THREAD_PRIORITY_DUR_MED     40          // Medium-priority thread timeslice 中优先级线程时间片
#define THREAD_PRIORITY_DUR_HIGH    80          // High-priority thread timeslice 高优先级线程时间片

// ---- The Host API ----------------------------------------------------------------------

#define MAX_HOST_API_SIZE           1024        // Maximum number of functions in the
												// host API

// ---- Functions -------------------------------------------------------------------------

#define MAX_FUNC_NAME_SIZE          256         // Maximum size of a function's name

// ---- Data Structures -----------------------------------------------------------------------


//runtime value 运行时 值 结构
typedef struct _Value							
{
	int iType;                                  // Type
	union                                       // The value
	{
		int iIntLiteral;                        // Integer literal
		float fFloatLiteral;                    // Float literal
		char* pstrStringLiteral;				// String literal
		int iStackIndex;                        // Stack Index 栈索引
		int iInstructionIndex;                  // Instruction index 指令索引
		int iFuncIndex;                         // Function index 函数索引
		int iHostAPICallIndex;                  // Host API Call index Host API Call索引
		int iReg;                               // Register code 寄存器
	};
	int iOffsetIndex;                           // Index of the offset
}Value;


//运行时堆栈
typedef struct _RuntimeStack					
{
	Value* pElmnts;							    // The stack elements 堆栈元素
	int iSize;									// The number of elements in the stack 堆栈中的元素个数
	int iTopIndex;								// The top index 栈顶索引
	int iFrameIndex;                            // Index of the top of the Current 当前栈帧顶的索引

}RuntimeStack;


//函数表
typedef struct _Func							
{
	int iEntryPoint;							// The entry point 入口点
	int iParamCount;							// The parameter count 参数个数
	int iLocalDataSize;							// Total size of all local data 本地数据大小
	int iStackFrameSize;						// Total size of the stack frame 栈帧大小
	char pstrName[MAX_FUNC_NAME_SIZE + 1];      // The function's name 函数名称
}Func;


//指令结构体
typedef struct _Instruction                           
{
	int iOpcode;                                // The opcode 操作码
	int iOpCount;                               // The number of operands 操作码的数量
	Value* pOpList;                            // The operand list
}Instruction;

//指令流结构
typedef struct _InstructionStream                     // An Instruction stream
{
	Instruction* pInstructions;							    // The Instructionuctions themselves 指针指向自身指令的指针
	int iSize;                                  // The number of Instructionuctions in the stream 流中指令的个数
	int iCurrentInstruction;                             // The Instruction pointer 当前指令指针
}InstructionStream;

//函数表
typedef struct _FuncTable                       // A function table
{
	Func* pFuncs;                              // Pointer to the function array
	int iSize;                                  // The number of functions in the array
}
FuncTable;


//host 程序api调用表
typedef struct _HostAPICallTable			
{
	char** ppstrCalls;							// Pointer to the call array 指向调用数字的指针
	int iSize;									// The number of calls in the array 数组中的调用个数
}HostAPICallTable;



//脚本结构 封装一个完整的脚本
typedef struct _Script							
{
	int iIsActive;								// Is this script structure in use? 这个脚本结构体是否激活

	// Header data

	int iGlobalDataSize;						// The size of the script's global data 脚本全局数据大小
	int iIsMainFuncPresent;                     // Is _Main () present?  _Main ()函数是否存在
	int iMainFuncIndex;							// _Main ()'s function index _Main ()的索引

	// Runtime tracking

	int iIsRunning;								// Is the script running? 脚本是否在跑
	int iIsPaused;								// Is the script Currently paused? 脚本是否暂停
	int iPauseEndTime;			                // If so, when should it resume? 如果暂停，何时恢复

	// Threading

	int iTimesliceDur;                          // The thread's timeslice duration 线程时间片

	// Register file

	Value _RetVal;								// The _RetVal register _RetVal寄存器

	// Script data 脚本数据

	InstructionStream InstructionStream;        // The Instruction stream 指令流
	RuntimeStack Stack;                         // The runtime stack 运行时堆栈
	FuncTable FuncTable;                        // The function table 函数表
	HostAPICallTable HostAPICallTable;			// The host API call table host应用api调用表
}Script;

//Host API function
typedef struct _HostAPIFunc                  
{
	int iIsActive;                              // Is this slot in use? 该函数是否使用

	int iThreadIndex;                           // The thread to which this function 对于哪个线程这个函数可见
	// is visible
	char* pstrName;                            // The function name 函数名称
	HostAPIFuncPntr fnFunc;                     // Pointer to the function definition 指向函数定义的指针
}HostAPIFunc;

//! 全局


//全局脚本实例 数组运行多个脚本被装载
Script g_Scripts[MAX_THREAD_COUNT];		    // The script array 全局脚本实例数组

//! 线程

int g_iCurrentThreadMode;                          // The Current threading mode 线程模式
int g_iCurrentThread;								// The Currently running thread 当前运行的线程
int g_iCurrentThreadActiveTime;					// The time at which the Current thread was activated

//! Host API
HostAPIFunc g_HostAPI[MAX_HOST_API_SIZE];    

// ---- Macros --------------------------------------------------------------------------------

//Resolves a stack index by translating negative indices relative to the top of the stack, to positive ones relative to the bottom.
#define ResolveStackIndex( iIndex )	\
										\
		( iIndex < 0 ? iIndex += g_Scripts [ g_iCurrentThread ].Stack.iFrameIndex : iIndex )



//Returns TRUE if the specified thread index is within the bounds of the array, FALSE otherwise.
//确保线程索引在合适的范围内
#define IsValidThreadIndex( iIndex )    \
                                            \
        ( iIndex < 0 || iIndex > MAX_THREAD_COUNT ? FALSE : TRUE )



//Returns TRUE if the specified thread is both a valid index and active, FALSE otherwise.
//确保线程索引指向的是活跃的线程
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
/// 初始化运行时环境
/// </summary>
void XS_Init()
{
	// ---- Initialize the script array
	//初始化脚本数组
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
	//将当前线程索引设置为0
	g_iCurrentThread = 0;
}


/// <summary>
/// 关闭运行时环境
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
/// 加载XSE文件进入内存
/// Loads an .XSE file into memory.
/// </summary>
/// <param name="pstrFilename"></param>
/// <param name="iThreadIndex"></param>
/// <param name="iThreadTimeslice"></param>
/// <returns></returns>
int XS_LoadScript(char* pstrFilename, int& iThreadIndex, int iThreadTimeslice)
{
	// ---- Find the next free script index
	//查找下一个空闲脚本的索引
	int iFreeThreadFound = FALSE;
	for (int iCurrentThreadIndex = 0; iCurrentThreadIndex < MAX_THREAD_COUNT; ++iCurrentThreadIndex)
	{
		// If the Current thread is not in use, use it
		//当前线程不在使用中，则使用它
		if (!g_Scripts[iCurrentThreadIndex].iIsActive)
		{
			iThreadIndex = iCurrentThreadIndex;
			iFreeThreadFound = TRUE;
			break;
		}
	}

	// If a thread wasn't found, return an out of threads error
	//未找到线程
	if (!iFreeThreadFound)
		return XS_LOAD_ERROR_OUT_OF_THREADS;

	//! 打开文件

	FILE* pScriptFile;
	if (!(pScriptFile = fopen(pstrFilename, "rb")))
		return XS_LOAD_ERROR_FILE_IO;

	//! 读取文件头

	// Create a buffer to hold the file's ID string (4 bytes + 1 null terminator = 5)
	//创建缓冲区保存文件的头部ID字符串 5个字节
	char* pstrIDString;
	if (!(pstrIDString = (char*)malloc(5)))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read the string (4 bytes) and append a null terminator
	//读取字符串（4） 并且追加一个终结符Null
	fread(pstrIDString, 4, 1, pScriptFile);
	pstrIDString[strlen(XSE_ID_STRING)] = '\0';

	// Compare the data read from the file to the ID string and exit on an error if they don't match
	//比较文件头字符
	if (strcmp(pstrIDString, XSE_ID_STRING) != 0)
		return XS_LOAD_ERROR_INVALID_XSE;

	// Free the buffer
	//释放缓冲区
	free(pstrIDString);

	// Read the script version (2 bytes total)
	//读取脚本的版本
	int iMajorVersion = 0,
		iMinorVersion = 0;

	fread(&iMajorVersion, 1, 1, pScriptFile);
	fread(&iMinorVersion, 1, 1, pScriptFile);

	// Validate the version, since this prototype only supports version 0.8 scripts
	//校验版本
	if (iMajorVersion != 0 || iMinorVersion != 8)
		return XS_LOAD_ERROR_UNSUPPORTED_VERS;

	// Read the stack size (4 bytes)
	//读取堆栈的大小
	fread(&g_Scripts[iThreadIndex].Stack.iSize, 4, 1, pScriptFile);

	// Check for a default stack size request
	//检查需要的默认堆栈大小
	if (g_Scripts[iThreadIndex].Stack.iSize == 0)
		g_Scripts[iThreadIndex].Stack.iSize = DEF_STACK_SIZE;

	// Allocate the runtime stack
	//分配运行时堆栈
	int iStackSize = g_Scripts[iThreadIndex].Stack.iSize;
	if (!(g_Scripts[iThreadIndex].Stack.pElmnts = (Value*)malloc(iStackSize * sizeof(Value))))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read the global data size (4 bytes)
	//读取全局数据大小
	fread(&g_Scripts[iThreadIndex].iGlobalDataSize, 4, 1, pScriptFile);

	// Check for presence of _Main () (1 byte)
	//检查_Main函数是否存在
	fread(&g_Scripts[iThreadIndex].iIsMainFuncPresent, 1, 1, pScriptFile);

	// Read _Main ()'s function index (4 bytes)
	//读取_Main函数的索引
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

	//! 读取指令流

	// Read the Instruction count (4 bytes)
	//读取指令数
	fread(&g_Scripts[iThreadIndex].InstructionStream.iSize, 4, 1, pScriptFile);

	// Allocate the stream
	//分配指令流内存
	if (!(g_Scripts[iThreadIndex].InstructionStream.pInstructions = (Instruction*)malloc(g_Scripts[iThreadIndex].InstructionStream.iSize * sizeof(Instruction))))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read the Instruction data
	//循环读取都指令数据
	for (int iCurrentInstructionIndex = 0; iCurrentInstructionIndex < g_Scripts[iThreadIndex].InstructionStream.iSize; ++iCurrentInstructionIndex)
	{
		// Read the opcode (2 bytes)
		//读取操作码
		g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].iOpcode = 0;
		fread(&g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].iOpcode, 2, 1, pScriptFile);

		// Read the operand count (1 byte)
		//读取操作数的个数
		g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].iOpCount = 0;
		fread(&g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].iOpCount, 1, 1, pScriptFile);

		int iOpCount = g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].iOpCount;//操作数的个数

		// Allocate space for the operand list in a temporary pointer
		//在临时指针中为操作数列表分配内存
		Value* pOpList;
		if (!(pOpList = (Value*)malloc(iOpCount * sizeof(Value))))
			return XS_LOAD_ERROR_OUT_OF_MEMORY;

		// Read in the operand list (N bytes)
		//读取操作数列表
		for (int iCurrentOpIndex = 0; iCurrentOpIndex < iOpCount; ++iCurrentOpIndex)
		{
			// Read in the operand type (1 byte)
			//读取操作数类型
			pOpList[iCurrentOpIndex].iType = 0;
			fread(&pOpList[iCurrentOpIndex].iType, 1, 1, pScriptFile);

			// Depending on the type, read in the operand data
			//根据类型，读取数据
			switch (pOpList[iCurrentOpIndex].iType)
			{
			// Integer literal
		    //整形
			case OP_TYPE_INT:
				fread(&pOpList[iCurrentOpIndex].iIntLiteral, sizeof(int), 1, pScriptFile);
				break;

			// Floating-point literal
            //浮点
			case OP_TYPE_FLOAT:
				fread(&pOpList[iCurrentOpIndex].fFloatLiteral, sizeof(float), 1, pScriptFile);
				break;

			// String index
            //字符串索引
			case OP_TYPE_STRING:

				// Since there's no field in the Value structure for string table indices, 
				// read the index into the integer literal field and set its type to string index

				fread(&pOpList[iCurrentOpIndex].iIntLiteral, sizeof(int), 1, pScriptFile);
				pOpList[iCurrentOpIndex].iType = OP_TYPE_STRING;
				break;

			// Instruction index
			//指令索引
			case OP_TYPE_Instruction_INDEX:
				fread(&pOpList[iCurrentOpIndex].iInstructionIndex, sizeof(int), 1, pScriptFile);
				break;

			// Absolute stack index
            //绝对堆栈索引
			case OP_TYPE_ABS_STACK_INDEX:
				fread(&pOpList[iCurrentOpIndex].iStackIndex, sizeof(int), 1, pScriptFile);
				break;

			// Relative stack index
			//相对堆栈索引
			case OP_TYPE_REL_STACK_INDEX:
				fread(&pOpList[iCurrentOpIndex].iStackIndex, sizeof(int), 1, pScriptFile);
				fread(&pOpList[iCurrentOpIndex].iOffsetIndex, sizeof(int), 1, pScriptFile);
				break;

			// Function index
			//函数索引
			case OP_TYPE_FUNC_INDEX:
				fread(&pOpList[iCurrentOpIndex].iFuncIndex, sizeof(int), 1, pScriptFile);
				break;

			// Host API call index
			//host api调用索引
			case OP_TYPE_HOST_API_CALL_INDEX:
				fread(&pOpList[iCurrentOpIndex].iHostAPICallIndex, sizeof(int), 1, pScriptFile);
				break;

			// Register
            //寄存器
			case OP_TYPE_REG:
				fread(&pOpList[iCurrentOpIndex].iReg, sizeof(int), 1, pScriptFile);
				break;
			}
		}

		// Assign the operand list pointer to the Instruction stream
		//让操作数列表指向指令流
		g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].pOpList = pOpList;
	}

	//! 读取字符串表

	// Read the table size (4 bytes)
	//读取字符串表大小
	int iStringTableSize;
	fread(&iStringTableSize, 4, 1, pScriptFile);

	// If the string table exists, read it
	//如果字符串表存在，读取
	if (iStringTableSize)
	{
		// Allocate a string table of this size
		//分配字符串表空间
		char** ppstrStringTable;
		if (!(ppstrStringTable = (char**)malloc(iStringTableSize * sizeof(char*))))
			return XS_LOAD_ERROR_OUT_OF_MEMORY;

		// Read in each string
		//读取每个string
		for (int iCurrentStringIndex = 0; iCurrentStringIndex < iStringTableSize; ++iCurrentStringIndex)
		{
			// Read in the string size (4 bytes)
			//读取string 大小
			int iStringSize;
			fread(&iStringSize, 4, 1, pScriptFile);

			// Allocate space for the string plus a null terminator
			//为字符串+终结符分配空间
			char* pstrCurrentString;
			if (!(pstrCurrentString = (char*)malloc(iStringSize + 1)))
				return XS_LOAD_ERROR_OUT_OF_MEMORY;

			// Read in the string data (N bytes) and append the null terminator
			//读取字符串数据并且添加终结符
			fread(pstrCurrentString, iStringSize, 1, pScriptFile);
			pstrCurrentString[iStringSize] = '\0';

			// Assign the string pointer to the string table
			//让字符串指针指向字符串表
			ppstrStringTable[iCurrentStringIndex] = pstrCurrentString;
		}

		// Run through each operand in the Instruction stream and assign copies of string operand's corresponding string literals
		//对指令流里的每个操作数进行处理并且将对应字符串赋值给的字符串操作数
		for (int iCurrentInstructionIndex = 0; iCurrentInstructionIndex < g_Scripts[iThreadIndex].InstructionStream.iSize; ++iCurrentInstructionIndex)
		{
			// Get the Instruction's operand count and a copy of it's operand list
			//获取指令操作数个数并且复制其操作数列表
			int iOpCount = g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].iOpCount;
			Value* pOpList = g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].pOpList;

			// Loop through each operand
			//循环每个操作数
			for (int iCurrentOpIndex = 0; iCurrentOpIndex < iOpCount; ++iCurrentOpIndex)
			{
				// If the operand is a string index, make a local copy of it's corresponding string in the table
				//如果操作数是字符串索引，则复制它在表中的相应字符串
				if (pOpList[iCurrentOpIndex].iType == OP_TYPE_STRING)
				{
					// Get the string index from the operand's integer literal field
					//获取字符串索引
					int iStringIndex = pOpList[iCurrentOpIndex].iIntLiteral;

					// Allocate a new string to hold a copy of the one in the table
					//分配一个新字符串用于存储
					char* pstrStringCopy;
					if (!(pstrStringCopy = (char*)malloc(strlen(ppstrStringTable[iStringIndex]) + 1)))
						return XS_LOAD_ERROR_OUT_OF_MEMORY;

					// Make a copy of the string
					//复制表中的字符串
					strcpy(pstrStringCopy, ppstrStringTable[iStringIndex]);

					// Save the string pointer in the operand list
					//将字符串指针保存在操作数列表中
					pOpList[iCurrentOpIndex].pstrStringLiteral = pstrStringCopy;
				}
			}
		}

		//释放原始字符串
		for (int iCurrentStringIndex = 0; iCurrentStringIndex < iStringTableSize; ++iCurrentStringIndex)
			free(ppstrStringTable[iCurrentStringIndex]);

		//释放字符串表本身
		free(ppstrStringTable);
	}

	//! 读取函数表 函数包含脚本函数的信息

	// Read the function count (4 bytes)
	//读取函数数量
	int iFuncTableSize;
	fread(&iFuncTableSize, 4, 1, pScriptFile);

	g_Scripts[iThreadIndex].FuncTable.iSize = iFuncTableSize;

	// Allocate the table
	//分配函数表
	if (!(g_Scripts[iThreadIndex].FuncTable.pFuncs = (Func*)malloc(iFuncTableSize * sizeof(Func))))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read each function
	//读取每一个函数
	for (int iCurrentFuncIndex = 0; iCurrentFuncIndex < iFuncTableSize; ++iCurrentFuncIndex)
	{
		// Read the entry point (4 bytes)
		//读取入口点
		int iEntryPoint;
		fread(&iEntryPoint, 4, 1, pScriptFile);

		// Read the parameter count (1 byte)
		//读取参数个数
		int iParamCount = 0;
		fread(&iParamCount, 1, 1, pScriptFile);

		// Read the local data size (4 bytes)
		//读取局部数据大小
		int iLocalDataSize;
		fread(&iLocalDataSize, 4, 1, pScriptFile);

		// Calculate the stack size
		//计算栈大小 = 参数数量+1+局部数据大小
		int iStackFrameSize = iParamCount + 1 + iLocalDataSize;

		// Read the function name length (1 byte)
		//读取函数名称长度
		int iFuncNameLength = 0;
		fread(&iFuncNameLength, 1, 1, pScriptFile);

		// Read the function name (N bytes) and append a null-terminator
		//读取函数名称并且加入终结符
		fread(&g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrentFuncIndex].pstrName, iFuncNameLength, 1, pScriptFile);
		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrentFuncIndex].pstrName[iFuncNameLength] = '\0';

		// Write everything to the function table
		//所有信息写入函数表
		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrentFuncIndex].iEntryPoint = iEntryPoint;
		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrentFuncIndex].iParamCount = iParamCount;
		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrentFuncIndex].iLocalDataSize = iLocalDataSize;
		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrentFuncIndex].iStackFrameSize = iStackFrameSize;
	}

	//! 读取host api 调用

	// Read the host API call count
	//读取api call 数量
	fread(&g_Scripts[iThreadIndex].HostAPICallTable.iSize, 4, 1, pScriptFile);

	// Allocate the table
	//分配api调用表
	if (!(g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls = (char**)malloc(g_Scripts[iThreadIndex].HostAPICallTable.iSize * sizeof(char*))))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read each host API call
	//读取每个call
	for (int iCurrentCallIndex = 0; iCurrentCallIndex < g_Scripts[iThreadIndex].HostAPICallTable.iSize; ++iCurrentCallIndex)
	{
		// Read the host API call string size (1 byte)
		//读取api调用的字符串大小
		int iCallLength = 0;
		fread(&iCallLength, 1, 1, pScriptFile);

		// Allocate space for the string plus the null terminator in a temporary pointer
		//在临时指针中为字符串+终结符分配大小
		char* pstrCurrentCall;
		if (!(pstrCurrentCall = (char*)malloc(iCallLength + 1)))
			return XS_LOAD_ERROR_OUT_OF_MEMORY;

		// Read the host API call string data and append the null terminator
		//读取host api调用字符串数据并且追加终结符
		fread(pstrCurrentCall, iCallLength, 1, pScriptFile);
		pstrCurrentCall[iCallLength] = '\0';

		// Assign the temporary pointer to the table
		//让指针指向api调用表
		g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls[iCurrentCallIndex] = pstrCurrentCall;
	}

	//! 结束文件读取
	fclose(pScriptFile);

	// The script is fully loaded and ready to go, so set the active flag
	//脚本已经完全装载，设置active flag
	g_Scripts[iThreadIndex].iIsActive = TRUE;

	// Reset the script
	//reset 脚本
	XS_ResetScript(iThreadIndex);

	// Return a success code
	//返回ok
	return XS_LOAD_OK;
}



/// <summary>
/// 从内存中卸载脚本
/// </summary>
/// <param name="iThreadIndex"></param>
void XS_UnloadScript(int iThreadIndex)
{
	// Exit if the script isn't active

	if (!g_Scripts[iThreadIndex].iIsActive)
		return;

	//! 释放指令流

	// First check to see if any Instructionuctions have string operands, and free them if they do
	//首先检查是否有指令包含字符串操作数，如果有则释放他们
	for (int iCurrentInstructionIndex = 0; iCurrentInstructionIndex < g_Scripts[iThreadIndex].InstructionStream.iSize; ++iCurrentInstructionIndex)
	{
		// Make a local copy of the operand count and operand list
		//对操作数个数和操作数列表做局部备份
		int iOpCount = g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].iOpCount;
		Value* pOpList = g_Scripts[iThreadIndex].InstructionStream.pInstructions[iCurrentInstructionIndex].pOpList;

		// Loop through each operand and free its string pointer
		//对操作数的个数循环，释放字符串指针
		for (int iCurrentOpIndex = 0; iCurrentOpIndex < iOpCount; ++iCurrentOpIndex)
			if (pOpList[iCurrentOpIndex].pstrStringLiteral)
				pOpList[iCurrentOpIndex].pstrStringLiteral;
	}

	// Now free the stream itself
	//释放指令流本身
	if (g_Scripts[iThreadIndex].InstructionStream.pInstructions)
		free(g_Scripts[iThreadIndex].InstructionStream.pInstructions);

	//! 释放运行时堆栈

	// Free any strings that are still on the stack

	for (int iCurrentElementIndex = 0; iCurrentElementIndex < g_Scripts[iThreadIndex].Stack.iSize; ++iCurrentElementIndex)
		if (g_Scripts[iThreadIndex].Stack.pElmnts[iCurrentElementIndex].iType == OP_TYPE_STRING)
			free(g_Scripts[iThreadIndex].Stack.pElmnts[iCurrentElementIndex].pstrStringLiteral);

	// Now free the stack itself
	//释放指令流本身
	if (g_Scripts[iThreadIndex].Stack.pElmnts)
		free(g_Scripts[iThreadIndex].Stack.pElmnts);

	//! 释放函数表

	if (g_Scripts[iThreadIndex].FuncTable.pFuncs)
		free(g_Scripts[iThreadIndex].FuncTable.pFuncs);

	//! 释放host API call函数表

	// First free each string in the table individually
	//释放字符串
	for (int iCurrentCallIndex = 0; iCurrentCallIndex < g_Scripts[iThreadIndex].HostAPICallTable.iSize; ++iCurrentCallIndex)
		if (g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls[iCurrentCallIndex])
			free(g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls[iCurrentCallIndex]);

	// Now free the table itself
	//释放表
	if (g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls)
		free(g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls);
}


/// <summary>
/// Resets the script. This function accepts a thread index rather than relying on the Currently active thread, because scripts can (and will) need to be reset arbitrarily.
/// 初始化虚拟机
/// </summary>
/// <param name="iThreadIndex"></param>
void XS_ResetScript(int iThreadIndex)
{
	// Get _Main ()'s function index in case we need it
	//在需要的时候获取_Main ()的索引
	int iMainFuncIndex = g_Scripts[iThreadIndex].iMainFuncIndex;

	// If the function table is present, set the entry point
	//如果函数表存在，设置入口点
	if (g_Scripts[iThreadIndex].FuncTable.pFuncs)
	{
		// If _Main () is present, read _Main ()'s index of the function table to get its entry point
		//如果_Main()函数存在，读取它在函数表中的索引来获取入口点
		if (g_Scripts[iThreadIndex].iIsMainFuncPresent)
		{
			g_Scripts[iThreadIndex].InstructionStream.iCurrentInstruction = g_Scripts[iThreadIndex].FuncTable.pFuncs[iMainFuncIndex].iEntryPoint;
		}
	}

	// Clear the stack
	//清空堆栈
	g_Scripts[iThreadIndex].Stack.iTopIndex = 0;
	g_Scripts[iThreadIndex].Stack.iFrameIndex = 0;

	// Set the entire stack to null
	//堆栈值设置为null
	for (int iCurrentElmntIndex = 0; iCurrentElmntIndex < g_Scripts[iThreadIndex].Stack.iSize; ++iCurrentElmntIndex)
		g_Scripts[iThreadIndex].Stack.pElmnts[iCurrentElmntIndex].iType = OP_TYPE_NULL;

	// Unpause the script
	//设置pause flag
	g_Scripts[iThreadIndex].iIsPaused = FALSE;

	// Allocate space for the globals
	//为全局变量分配空间
	PushFrame(iThreadIndex, g_Scripts[iThreadIndex].iGlobalDataSize);

	// If _Main () is present, push its stack frame (plus one extra stack element to compensate for the function index that usually sits on top of stack frames and causes indices to start from -2)
	//如果_Main（）存在，压入它的栈帧 为了弥补索引所以再额外加一个
	PushFrame(iThreadIndex, g_Scripts[iThreadIndex].FuncTable.pFuncs[iMainFuncIndex].iLocalDataSize + 1);
}


/// <summary>
/// Runs the Currenty loaded script array for a given timeslice duration.
/// 运行脚本
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
	//创建变量记录当前时间
	int iCurrentTime;

	while (TRUE)
	{

		

		// Check to see if all threads have terminated, and if so, break the execution cycle
		//检查是否所有线程结束，结束则跳出
		int iIsStillActive = FALSE;
		for (int iCurrentThreadIndex = 0; iCurrentThreadIndex < MAX_THREAD_COUNT; ++iCurrentThreadIndex)
		{
			if (g_Scripts[iCurrentThreadIndex].iIsActive && g_Scripts[iCurrentThreadIndex].iIsRunning)
				iIsStillActive = TRUE;
		}
		if (!iIsStillActive)
			break;

		// Update the Current time
		//更新当前时间
		iCurrentTime = GetCurrTime();

		// Check for a context switch if the threading mode is set for multithreading
		//! 执行上下文切换  如果是多线程模式
		if (g_iCurrentThreadMode == THREAD_MODE_MULTI)
		{
			// If the Current thread's timeslice has elapsed, or if it's terminated switch to the next valid thread
			//如果当前线程的时间片用完或者线程结束，切换到下一个合法线程
			if (iCurrentTime > g_iCurrentThreadActiveTime + g_Scripts[g_iCurrentThread].iTimesliceDur ||
				!g_Scripts[g_iCurrentThread].iIsRunning)
			{
				// Loop until the next thread is found
				//进行循环直到找打下一个线程
				while (TRUE)
				{
					// Move to the next thread in the array
					//移动到下一个线程
					++g_iCurrentThread;

					// If we're past the end of the array, loop back around
					//如果到达了数组的结尾，回退重新查看
					if (g_iCurrentThread >= MAX_THREAD_COUNT)
						g_iCurrentThread = 0;

					// If the thread we've chosen is active and running, break the loop
					//如果线程是活跃并且在运行的，则跳出循环
					if (g_Scripts[g_iCurrentThread].iIsActive && g_Scripts[g_iCurrentThread].iIsRunning)
						break;
				}

				// Reset the timeslice
				//重置时间片
				g_iCurrentThreadActiveTime = iCurrentTime;
			}
		}

		// Is the script Currently paused?
		//当前脚本是否被暂停
		if (g_Scripts[g_iCurrentThread].iIsPaused)
		{
			// Has the pause duration elapsed yet?
			//暂停的持续时间已经过去了吗
			if (iCurrentTime >= g_Scripts[g_iCurrentThread].iPauseEndTime)
			{
				// Yes, so unpause the script
				//已经过期，结束暂停
				g_Scripts[g_iCurrentThread].iIsPaused = FALSE;
			}
			else
			{
				// No, so skip this iteration of the execution cycle
				//没有过期
				continue;
			}
		}

		// Make a copy of the Instruction pointer to compare later
		//保存当前指令的指针 即保存ip
		int iCurrentInstruction = g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction;

		// Get the Current opcode
		//获取当前的opcode
		int iOpcode = g_Scripts[g_iCurrentThread].InstructionStream.pInstructions[iCurrentInstruction].iOpcode;

		// Execute the Current Instruction based on its opcode, as long as we aren't Currently paused
		//执行opcode
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
			//opcode执行
			switch (iOpcode)
			{
			// Move
			case Instruction_MOV:

				// Skip cases where the two operands are the same
				//如果两个操作数一样就跳过
				if (ResolveOpPntr(0) == ResolveOpPntr(1))
					break;

				// Copy the source operand into the destination
				//复制
				CopyValue(&Dest, Source);

				break;

				// The arithmetic Instructionuctions only work with destination types that
				// are either integers or floats. They first check for integers and
				// assume that anything else is a float. Mod only works with integers.

			// Add
			case Instruction_ADD:
				//源操作数和目标操作数相加 加到目标操作数上
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
			//用此函数获取一个指向目标操作数的value结构体的指针，并将结果赋值给他
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
		//分支指令
		case Instruction_JMP:
		{
			// Get the index of the target Instruction (opcode index 0)
			//获取目标的操作数 即操作数索引0
			int iTargetIndex = ResolveOpAsInstructionIndex(0);

			// Move the Instruction pointer to the target
			//移动指令指针（ip）到目标索引
			g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction = iTargetIndex;

			break;
		}

		//JCC指令
		//JCC Op0,Op1 ,Target
		case Instruction_JE:
		case Instruction_JNE:
		case Instruction_JG:
		case Instruction_JL:
		case Instruction_JGE:
		case Instruction_JLE:
		{
			// Get the two operands
			//获取两个操作数
			Value Op0 = ResolveOpValue(0);
			Value Op1 = ResolveOpValue(1);

			// Get the index of the target Instruction (opcode index 2)
			//获取第三个索引，即跳转目标
			int iTargetIndex = ResolveOpAsInstructionIndex(2);

			// Perform the specified comparison and jump if it evaluates to true
			//进行对比 结果为真则跳转
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
			//结果为真，跳转，移动ip
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


		//函数调用 接口
		case Instruction_CALL:
		{
			// Get a local copy of the function index
			//获取函数索引
			int iFuncIndex = ResolveOpAsFuncIndex(0);

			// Advance the Instruction pointer so it points to the Instruction immediately following the call

			++g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction;

			// Call the function
			//调用函数
			CallFunc(g_iCurrentThread, iFuncIndex);

			break;
		}

		//Ret指令
		case Instruction_RET:
		{
			// Get the Current function index off the top of the stack and use it to get the corresponding function structure
			//从当前栈顶弹出函数索引，并获取相关函数结构体信息
			Value FuncIndex = Pop(g_iCurrentThread);

			// Check for the presence of a stack base marker

			if (FuncIndex.iType = OP_TYPE_STACK_BASE_MARKER)
				iExitExecLoop = TRUE;

			// Get the previous function index
			//获取前一个函数的索引
			Func CurrentFunc = GetFunc(g_iCurrentThread, FuncIndex.iFuncIndex);
			int iFrameIndex = FuncIndex.iOffsetIndex;

			// Read the return address structure from the stack, which is stored one index below the local data
			//从堆栈中读取返回地址的结构，它存储在局部数据下方的索引中
			Value ReturnAddr = GetStackValue(g_iCurrentThread, g_Scripts[g_iCurrentThread].Stack.iTopIndex - (CurrentFunc.iLocalDataSize + 1));

			// Pop the stack frame along with the return address
			//把栈帧弹出 连同返回地址
			PopFrame(CurrentFunc.iStackFrameSize);

			// Restore the previous frame index
			//存储之前的堆栈索引
			g_Scripts[g_iCurrentThread].Stack.iFrameIndex = iFrameIndex;

			// Make the jump to the return address
			//跳转到返回地址
			g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction = ReturnAddr.iInstructionIndex;

			break;
		}

		case Instruction_CALLHOST:
		{
			// Use operand zero to index into the host API call table and get the host API function name
			//使用操作数0获取host api调用表索引并且获得函数名称
			Value HostAPICall = ResolveOpValue(0);
			int iHostAPICallIndex = HostAPICall.iHostAPICallIndex;

			// Get the name of the host API function
			//获取函数名称
			char* pstrFuncName = GetHostAPICall(iHostAPICallIndex);

			// Search through the host API until the matching function is found
			//搜索host api直到找到匹配的
			int HostAPIFuncIndex;
			int iMatchFound = FALSE;
			for (int iHostAPIFuncIndex = 0; iHostAPIFuncIndex < MAX_HOST_API_SIZE; ++iHostAPIFuncIndex)
			{
				// Get a pointer to the name of the Current host API function
				//获取指向当前host api函数名称的指针
				char* pstrCurrentHostAPIFunc = g_HostAPI[iHostAPIFuncIndex].pstrName;

				// If it equals the requested name, it might be a match

				if (strcmp(pstrFuncName, pstrCurrentHostAPIFunc) == 0)
				{
					// Make sure the function is visible to the Current thread
					//确保该函数对当前线程可见
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
			//匹配到，调用，传递到当前线程索引
			if (iMatchFound)
				//g_HostAPI[iHostAPIFuncIndex].fnFunc(g_iCurrentThread);
				g_HostAPI[HostAPIFuncIndex].fnFunc(g_iCurrentThread);

			break;
		}

		// ---- Misc
		//暂停
		case Instruction_PAUSE:
		{
			// Get the pause duration
			//获取暂时持续时间
			int iPauseDuration = ResolveOpAsInt(0);

			// Determine the ending pause time
			//确定暂停结束时间=当前时间+持续时间
			g_Scripts[g_iCurrentThread].iPauseEndTime = iCurrentTime + iPauseDuration;

			// Pause the script
			//暂停脚本
			g_Scripts[g_iCurrentThread].iIsPaused = TRUE;

			break;
		}

		case Instruction_EXIT:
		{
			// Resolve operand zero to find the exit code
			//解析操作数来发现exit代码
			Value ExitCode = ResolveOpValue(0);

			// Get it from the integer field
			//从整数中获取
			int iExitCode = ExitCode.iIntLiteral;

			// Tell the XVM to stop executing the script
			//告诉虚拟机停止运行
			g_Scripts[g_iCurrentThread].iIsRunning = FALSE;

			break;
		}
		}

		// If the Instruction pointer hasn't been changed by an Instruction, increment it
		//在指令的执行期间ip是否被改变？ 
		if (iCurrentInstruction == g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction)
			++g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction;//ip未被改变，增大它，指向下一条指令

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
	//线程索引合法有效
	if (!IsThreadActive(iThreadIndex))
		return;

	// Set the thread's execution flag
	//设置线程的执行标志
	g_Scripts[iThreadIndex].iIsRunning = TRUE;

	// Set the Current thread to the script
	//将当前线程分配给脚本
	g_iCurrentThread = iThreadIndex;

	// Set the activation time for the Current thread to get things rolling
	//设计当前线程活跃时间
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
/// 复制值
/// </summary>
/// <param name="pDest"></param>
/// <param name="Source"></param>
void CopyValue(Value* pDest, Value Source)
{
	// If the destination already contains a string, make sure to free it first
	//如果目标已经包含一个字符串，则释放它
	if (pDest->iType == OP_TYPE_STRING)
		free(pDest->pstrStringLiteral);

	// Copy the object
	//复制这个对象
	*pDest = Source;

	// Make a physical copy of the source string, if necessary
	//有必要的话，做一个源字符串的物理备份
	if (Source.iType == OP_TYPE_STRING)
	{
		pDest->pstrStringLiteral = (char*)malloc(strlen(Source.pstrStringLiteral) + 1);
		strcpy(pDest->pstrStringLiteral, Source.pstrStringLiteral);
	}
}


/// <summary>
/// Coerces a Value structure from it's Current type to an integer value.
/// 强制将value结构转换为int
/// </summary>
/// <param name="Val"></param>
/// <returns></returns>
int CoerceValueToInt(Value Val)
{
	// Determine which type the Value Currently is
	//判断当前值是什么类型
	switch (Val.iType)
	{
	// It's an integer, so return it as-is
    //整型返回它自己
	case OP_TYPE_INT:
		return Val.iIntLiteral;

	// It's a float, so cast it to an integer
    //浮点转换
	case OP_TYPE_FLOAT:
		return (int)Val.fFloatLiteral;

	// It's a string, so convert it to an integer
	//字符串转换
	case OP_TYPE_STRING:
		return atoi(Val.pstrStringLiteral);

	// Anything else is invalid
    //其他不合法
	default:
		return 0;
	}
}

/// <summary>
/// Coerces a Value structure from it's Current type to an float value.
/// 强制将value结构体转为浮点
/// </summary>
/// <param name="Val"></param>
/// <returns></returns>
float CoerceValueToFloat(Value Val)
{
	// Determine which type the Value Currently is
	//判断当前值的类型
	switch (Val.iType)
	{
	// It's an integer, so cast it to a float
	//int强转为float
	case OP_TYPE_INT:
		return (float)Val.iIntLiteral;

	// It's a float, so return it as-is
	//浮点，返回自身
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
/// 强转为string
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
/// 返回当前指令中指定的操作数的类型
/// Returns the type of the specified operand in the Current Instruction.
/// </summary>
/// <param name="iOpIndex"></param>
/// <returns></returns>
inline int GetOpType(int iOpIndex)
{
	// Get the Current Instruction
	//获取当前指令
	int iCurrentInstruction = g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction;

	// Return the type
	//返回类型
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
	//获取当前指令
	int iCurrentInstruction = g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction;

	// Get the operand type type
	//获取操作数类型
	Value OpValue = g_Scripts[g_iCurrentThread].InstructionStream.pInstructions[iCurrentInstruction].pOpList[iOpIndex];

	// Resolve the stack index based on its type
	
	switch (OpValue.iType)
	{
	// It's an absolute index so return it as-is
    //绝对堆栈索引
	case OP_TYPE_ABS_STACK_INDEX:
		return OpValue.iStackIndex;

	// It's a relative index so resolve it
	//相对堆栈索引
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
	//获取当前指令
	int iCurrentInstruction = g_Scripts[g_iCurrentThread].InstructionStream.iCurrentInstruction;

	// Get the operand type
	//获取当前操作数类型
	Value OpValue = g_Scripts[g_iCurrentThread].InstructionStream.pInstructions[iCurrentInstruction].pOpList[iOpIndex];

	// Determine what to return based on the value's type
	//根据类型决定返回什么内容
	switch (OpValue.iType)
	{
	// It's a stack index so resolve it
	//堆栈索引 所以解析它
	case OP_TYPE_ABS_STACK_INDEX:
	case OP_TYPE_REL_STACK_INDEX:
	{
		// Resolve the index and use it to return the corresponding stack element
		//解析索引并且返回相应的堆栈元素
		int iAbsIndex = ResolveOpStackIndex(iOpIndex);
		return GetStackValue(g_iCurrentThread, iAbsIndex);
	}

	// It's in _RetVal
	//在_RetVal 寄存器中
	case OP_TYPE_REG:
		return g_Scripts[g_iCurrentThread]._RetVal;

		// Anything else can be returned as-is
		//其他类型操作数返回自身
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
	//获取操作数类型
	int iIndirMethod = GetOpType(iOpIndex);

	// Return a pointer to wherever the operand lies
	//根据所在位置返回一个指针
	switch (iIndirMethod)
	{
	// It's on the stack
	//在栈上
	case OP_TYPE_ABS_STACK_INDEX:
	case OP_TYPE_REL_STACK_INDEX:
	{
		int iStackIndex = ResolveOpStackIndex(iOpIndex);
		return &g_Scripts[g_iCurrentThread].Stack.pElmnts[ResolveStackIndex(iStackIndex)];
	}

	// It's _RetVal
	//在返回寄存器上
	case OP_TYPE_REG:
		return &g_Scripts[g_iCurrentThread]._RetVal;
	}

	// Return NULL for anything else

	return NULL;
}



/// <summary>
/// Returns the specified stack value.
/// 返回指定的堆栈的值
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
/// 设定指定的堆栈的值
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
/// 压栈
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="Val"></param>
inline void Push(int iThreadIndex, Value Val)
{
	// Get the Current top element
	//获取栈顶元素
	int iTopIndex = g_Scripts[iThreadIndex].Stack.iTopIndex;

	// Put the value into the Current top index
	//将值放入当前栈顶的索引
	CopyValue(&g_Scripts[iThreadIndex].Stack.pElmnts[iTopIndex], Val);

	// Increment the top index
	//增加栈顶索引
	++g_Scripts[iThreadIndex].Stack.iTopIndex;
}

/// <summary>
/// Pops the element off the top of the stack.
/// 出栈
/// </summary>
/// <param name="iThreadIndex"></param>
/// <returns></returns>
inline Value Pop(int iThreadIndex)
{
	// Decrement the top index to clear the old element for overwriting
	//减小栈顶索引
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
/// 压入栈帧
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iSize"></param>
inline void PushFrame(int iThreadIndex, int iSize)
{
	// Increment the top index by the size of the frame
	//增加帧的大小的栈顶索引
	g_Scripts[iThreadIndex].Stack.iTopIndex += iSize;

	// Move the frame index to the new top of the stack
	//将栈帧索引移到新栈顶
	g_Scripts[iThreadIndex].Stack.iFrameIndex = g_Scripts[iThreadIndex].Stack.iTopIndex;
}


/// <summary>
/// Pops a stack frame.
/// 弹出栈帧
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
/// 根据索引从函数表中读取函数
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
/// 根据索引从host api调用表中读取函数
/// </summary>
/// <param name="iIndex"></param>
/// <returns></returns>
inline char* GetHostAPICall(int iIndex)
{
	return g_Scripts[g_iCurrentThread].HostAPICallTable.ppstrCalls[iIndex];
}


/// <summary>
/// Wrapper for the system-dependant method of determining the Current time in milliseconds.
/// 获取当前时间
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
/// 调用函数
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iIndex"></param>
void CallFunc(int iThreadIndex, int iIndex)
{
	//函数索引
	//局部变量
	//返回地址 
	//参数 
	
	//获取当前目标函数信息
	Func DestFunc = GetFunc(iThreadIndex, iIndex);

	// Save the Current stack frame index
	//保存当前栈帧索引
	int iFrameIndex = g_Scripts[iThreadIndex].Stack.iFrameIndex;

	// Push the return address, which is the Current Instruction
	//push返回地址，即当前指令地址
	Value ReturnAddr;
	ReturnAddr.iInstructionIndex = g_Scripts[iThreadIndex].InstructionStream.iCurrentInstruction;
	Push(iThreadIndex, ReturnAddr);

	// Push the stack frame + 1 (the extra space is for the function index we'll put on the stack after it
	//压入栈帧大小+1,额外空间是给函数索引使用的
	PushFrame(iThreadIndex, DestFunc.iLocalDataSize + 1);

	// Write the function index and old stack frame to the top of the stack
	//将函数索引和旧的栈帧索引写入栈顶
	Value FuncIndex;
	FuncIndex.iFuncIndex = iIndex;//函数索引
	FuncIndex.iOffsetIndex = iFrameIndex;//旧的栈帧索引
	SetStackValue(iThreadIndex, g_Scripts[iThreadIndex].Stack.iTopIndex - 1, FuncIndex);

	// Let the caller make the jump to the entry point
	//转到函数入口点
	g_Scripts[iThreadIndex].InstructionStream.iCurrentInstruction = DestFunc.iEntryPoint;
}

/// <summary>
/// Passes an integer parameter from the host to a script function.
/// 从host程序传递一个int参数给脚本函数
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iInt"></param>
void XS_PassIntParam(int iThreadIndex, int iInt)
{
	// Create a Value structure to encapsulate the parameter
	//创建value结构封装参数
	Value Param;
	Param.iType = OP_TYPE_INT;
	Param.iIntLiteral = iInt;

	// Push the parameter onto the stack
	//压栈参数
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
/// 根据名称获取函数索引
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="pstrName"></param>
/// <returns></returns>
int GetFuncIndexByName(int iThreadIndex, char* pstrName)
{
	// Loop through each function and look for a matching name
	//匹配名称
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
/// 从host api中调用脚本函数
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
	//保存当前虚拟机状态
	int iPrevThreadMode = g_iCurrentThreadMode;
	int iPrevThread = g_iCurrentThread;

	// Set the threading mode for single-threaded execution
	//设置线程为单线程模式
	g_iCurrentThreadMode = THREAD_MODE_SINGLE;

	// Set the active thread to the one specified
	//将活跃线程设置为指定进程
	g_iCurrentThread = iThreadIndex;

	// Get the function's index based on it's name
	//获取函数索引
	int iFuncIndex = GetFuncIndexByName(iThreadIndex, pstrName);

	// Make sure the function name was valid

	if (iFuncIndex == -1)
		return;

	// Call the function
	//调用
	CallFunc(iThreadIndex, iFuncIndex);

	// Set the stack base
	//设置栈基址
	Value StackBase = GetStackValue(g_iCurrentThread, g_Scripts[g_iCurrentThread].Stack.iTopIndex - 1);
	StackBase.iType = OP_TYPE_STACK_BASE_MARKER;
	SetStackValue(g_iCurrentThread, g_Scripts[g_iCurrentThread].Stack.iTopIndex - 1, StackBase);

	// Allow the script code to execute uninterrupted until the function returns
	//运行脚本不停执行直到返回
	XS_RunScripts(XS_INFINITE_TIMESLICE);

	//! 处理函数返回

	// Restore the VM state
	//保存虚拟机状态
	g_iCurrentThreadMode = iPrevThreadMode;
	g_iCurrentThread = iPrevThread;
}

/// <summary>
/// Invokes a script function from the host application, meaning the call executes in sync with the script.
/// 同步调用脚本函数
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="pstrName"></param>
void XS_InvokeScriptFunc(int iThreadIndex, char* pstrName)
{
	// Make sure the thread index is valid and active
	//确保线程索引合法
	if (!IsThreadActive(iThreadIndex))
		return;

	// Get the function's index based on its name
	//获取函数索引
	int iFuncIndex = GetFuncIndexByName(iThreadIndex, pstrName);

	// Make sure the function name was valid
	//确保函数名合法
	if (iFuncIndex == -1)
		return;

	// Call the function
	//调用函数
	CallFunc(iThreadIndex, iFuncIndex);
}


/// <summary>
/// Registers a function with the host API.
/// 为host api注册函数
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="pstrName"></param>
/// <param name="fnFunc"></param>
void XS_RegisterHostAPIFunc(int iThreadIndex, char* pstrName, HostAPIFuncPntr fnFunc)
{
	// Loop through each function in the host API until a free index is found
	//循环host api，找到空闲索引
	for (int iCurrentHostAPIFunc = 0; iCurrentHostAPIFunc < MAX_HOST_API_SIZE; ++iCurrentHostAPIFunc)
	{
		// If the Current index is free, use it
		//当前索引空闲，使用
		if (!g_HostAPI[iCurrentHostAPIFunc].iIsActive)
		{
			// Set the function's parameters
			//设置函数参数
			g_HostAPI[iCurrentHostAPIFunc].iThreadIndex = iThreadIndex;
			g_HostAPI[iCurrentHostAPIFunc].pstrName = (char*)malloc(strlen(pstrName) + 1);
			strcpy(g_HostAPI[iCurrentHostAPIFunc].pstrName, pstrName);
			_strupr(g_HostAPI[iCurrentHostAPIFunc].pstrName);
			g_HostAPI[iCurrentHostAPIFunc].fnFunc = fnFunc;

			// Set the function to active
			//设置函数激活
			g_HostAPI[iCurrentHostAPIFunc].iIsActive = TRUE;
		}
	}
}


/// <summary>
/// Returns the specified integer parameter to a host API function.
/// 将参数转换，适配host api
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iParamIndex"></param>
/// <returns></returns>
int XS_GetParamAsInt(int iThreadIndex, int iParamIndex)
{
	// Get the Current top element
	//获取栈顶元素
	int iTopIndex = g_Scripts[g_iCurrentThread].Stack.iTopIndex;
	Value Param = g_Scripts[iThreadIndex].Stack.pElmnts[iTopIndex - (iParamIndex + 1)];

	// Coerce the top element of the stack to an integer
	//将栈顶元素强转为整型
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
/// 从host api函数中返回
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iParamCount"></param>
void XS_ReturnFromHost(int iThreadIndex, int iParamCount)
{
	// Clear the parameters off the stack
	//清除堆栈中的参数
	g_Scripts[iThreadIndex].Stack.iTopIndex -= iParamCount;
}

/// <summary>
/// Returns an integer from a host API function.
/// 从host api函数返回一个int
/// </summary>
/// <param name="iThreadIndex"></param>
/// <param name="iParamCount"></param>
/// <param name="iInt"></param>
void XS_ReturnIntFromHost(int iThreadIndex, int iParamCount, int iInt)
{
	// Clear the parameters off the stack
	//从堆栈中清除参数
	g_Scripts[iThreadIndex].Stack.iTopIndex -= iParamCount;

	// Put the return value and type in _RetVal
	//将返回值和类型放入 Ret寄存器
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