//编译器主模块文件头

#ifndef XSC_MAIN
#define XSC_MAIN



#include "globals.h"
#include "linked_list.h"
#include "stack.h"

// ---- Constants -----------------------------------------------------------------------------

	// ---- General ---------------------------------------------------------------------------

#ifndef TRUE
#define TRUE                    1           // True
#endif

#ifndef FALSE
#define FALSE                   0           // False
#endif

// ---- Program ---------------------------------------------------------------------------

#define VERSION_MAJOR               0           // Major version number
#define VERSION_MINOR               8           // Minor version number

// ---- Filename --------------------------------------------------------------------------

#define MAX_FILENAME_SIZE           2048        // Maximum filename length

#define SOURCE_FILE_EXT             ".XSS"      // Extension of a source code file 源代码文件后缀
#define OUTPUT_FILE_EXT             ".XASM"     // Extension of an output assembly file 输出汇编文件后缀

// ---- Source Code -----------------------------------------------------------------------

#define MAX_SOURCE_LINE_SIZE        4096        // Maximum source line length

#define MAX_IDENT_SIZE              256        // Maximum identifier size

//! Priority Types  优先级

#define PRIORITY_NONE               0           // A priority wasn't specified
#define PRIORITY_USER               1           // User-defined priority
#define PRIORITY_LOW                2           // Low priority
#define PRIORITY_MED                3           // Medium priority
#define PRIORITY_HIGH               4           // High priority

#define PRIORITY_LOW_KEYWORD        "Low"       // Low priority keyword
#define PRIORITY_MED_KEYWORD        "Med"       // Low priority keyword
#define PRIORITY_HIGH_KEYWORD       "High"      // Low priority keyword

// ---- Functions -------------------------------------------------------------------------

#define MAIN_FUNC_NAME				"_Main"		// _Main ()'s name

// ---- Register Codes---------------------------------------------------------------------

#define REG_CODE_RETVAL             0           // _RetVal

// ---- Internal Script Entities ----------------------------------------------------------

#define TEMP_VAR_0                  "_T0"       // Temporary variable 0
#define TEMP_VAR_1                  "_T1"       // Temporary variable 1

// ---- Data Structures -----------------------------------------------------------------------



//脚本头 Script header data
typedef struct _ScriptHeader                    
{
	int iStackSize;                             // Requested stack size 需要的堆栈大小

	int iIsMainFuncPresent;                     // Is _Main () present? Main()函数是否存在
	int iMainFuncIndex;							// _Main ()'s function index  _Main ()函数索引

	int iPriorityType;                          // The thread priority type 线程优先级类型
	int iUserPriority;                          // The user-defined priority (if any) 用户定义的优先级
}ScriptHeader;

// ---- Global Variables ----------------------------------------------------------------------

	// ---- Source Code -----------------------------------------------------------------------

extern char g_pstrSourceFilename[MAX_FILENAME_SIZE],
g_pstrOutputFilename[MAX_FILENAME_SIZE];

extern LinkedList g_SourceCode;

//Script 脚本头实例
extern ScriptHeader g_ScriptHeader;

//Function Table
extern LinkedList g_FuncTable;

//Symbol Table
extern LinkedList g_SymbolTable;

//String Table
extern LinkedList g_StringTable;

// ---- Expression Evaluation -------------------------------------------------------------

extern int g_iTempVar0SymbolIndex,
g_iTempVar1SymbolIndex;

// ---- Function Prototypes -------------------------------------------------------------------

void PrintLogo();
void PrintUsage();

void VerifyFilenames(int argc, char* argv[]);
void ReadCmmndLineParams(int argc, char* argv[]);

void Init();
void ShutDown();

void LoadSourceFile();
void CompileSourceFile();
void PrintCompiletats();

void AssmblOutputFile();

void Exit();

#endif
