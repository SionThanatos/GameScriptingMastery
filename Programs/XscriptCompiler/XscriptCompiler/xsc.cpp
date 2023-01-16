//编译器主模块

// ---- Include Files -------------------------------------------------------------------------

#include "xsc.h"

#include "error.h"
#include "func_table.h"
#include "symbol_table.h"

#include "preprocessor.h"
#include "lexer.h"
#include "parser.h"
#include "i_code.h"
#include "code_emit.h"

// ---- Globals -------------------------------------------------------------------------------

// ---- Source Code -----------------------------------------------------------------------

char g_pstrSourceFilename[MAX_FILENAME_SIZE],	// Source code filename  源代码文件名
g_pstrOutputFilename[MAX_FILENAME_SIZE];	// Executable filename 可执行文件名

LinkedList g_SourceCode;                        // Source code linked list

// ---- Script ----------------------------------------------------------------------------

ScriptHeader g_ScriptHeader;                    // Script header data

// ---- I-Code Stream ---------------------------------------------------------------------

LinkedList g_ICodeStream;                       // I-code stream

// ---- Function Table --------------------------------------------------------------------

LinkedList g_FuncTable;                         // The function table

// ---- Symbol Table ----------------------------------------------------------------------

LinkedList g_SymbolTable;                       // The symbol table

// ---- String Table ----------------------------------------------------------------------

LinkedList g_StringTable;						// The string table

// ---- XASM Invocation -------------------------------------------------------------------

int g_iPreserveOutputFile;                      // Preserve the assembly file?
int g_iGenerateXSE;                             // Generate an .XSE executable?

// ---- Expression Evaluation -------------------------------------------------------------
//为表达式求值临时添加的两个临时变量 Temporary variable symbol indices
int g_iTempVar0SymbolIndex,                     
	g_iTempVar1SymbolIndex;

// ---- Functions -----------------------------------------------------------------------------

/// <summary>
/// Prints out logo/credits information.
/// 打印程序信息
/// </summary>
void PrintLogo()
{
	printf("XSC\n");
	printf("XtremeScript Compiler Version %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
	printf("2022 Version\n");
	printf("\n");
}

/// <summary>
///  Prints out usage information.
/// 打印说明
/// </summary>
void PrintUsage()
{
	printf("Usage:\tXSC Source.XSS [Output.XASM] [Options]\n");
	printf("\n");
	printf("\t-S:Size      Sets the stack size (must be decimal integer value)\n");
	printf("\t-P:Priority  Sets the thread priority: Low, Med, High or timeslice\n");
	printf("\t             duration (must be decimal integer value)\n");
	printf("\t-A           Preserve assembly output file\n");
	printf("\t-N           Don't generate .XSE (preserves assembly output file)\n");
	printf("\n");
	printf("Notes:\n");
	printf("\t- File extensions are not required.\n");
	printf("\t- Executable name is optional; source name is used by default.\n");
	printf("\n");
}

/// <summary>
/// Verifies the input and output filenames.
/// 校验输入输出文件名
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
void VerifyFilenames(int argc, char* argv[])
{
	// First make a global copy of the source filename and convert it to uppercase
	//首先对原文件名字进行复制并且改为大写
	strcpy(g_pstrSourceFilename, argv[1]);
	_strupr(g_pstrSourceFilename);

	// Check for the presence of the .XSS extension and add it if it's not there
	//检查后缀.XSS是否存在，没有的话就加上
	if (!strstr(g_pstrSourceFilename, SOURCE_FILE_EXT))
	{
		// The extension was not found, so add it to string
		//没有后缀就加上
		strcat(g_pstrSourceFilename, SOURCE_FILE_EXT);
	}

	// Was an executable filename specified?
	//是否指定了可执行文件名字
	if (argv[2] && argv[2][0] != '-')
	{
		// Yes, so repeat the validation process
		//是，再次验证名称
		strcpy(g_pstrOutputFilename, argv[2]);
		_strupr(g_pstrOutputFilename);

		// Check for the presence of the .XASM extension and add it if it's not there
		//检查是否有后缀.XASM
		if (!strstr(g_pstrOutputFilename, OUTPUT_FILE_EXT))
		{
			// The extension was not found, so add it to string
			//没有就加上
			strcat(g_pstrOutputFilename, OUTPUT_FILE_EXT);
		}
	}
	else
	{
		// No, so base it on the source filename
		//没有之丁宁可执行文件名称，所以使用原文件名称
		// First locate the start of the extension, and use pointer subtraction to find the index
		//首先找到后缀开始的地方，与开始指针加减获得偏移
		int ExtOffset = strrchr(g_pstrSourceFilename, '.') - g_pstrSourceFilename;
		strncpy(g_pstrOutputFilename, g_pstrSourceFilename, ExtOffset);

		// Append null terminator
		//添加终结符
		g_pstrOutputFilename[ExtOffset] = '\0';

		// Append executable extension
		//添加可执行文件后缀
		strcat(g_pstrOutputFilename, OUTPUT_FILE_EXT);
	}
}

/// <summary>
/// Reads and verifies the command line parameters.
/// 读取并校验命令行参数
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
void ReadCmmndLineParams(int argc, char* argv[])
{
	char pstrCurrOption[32];
	char pstrCurrValue[32];
	char pstrErrorMssg[256];

	for (int iCurrOptionIndex = 0; iCurrOptionIndex < argc; ++iCurrOptionIndex)
	{
		// Convert the argument to uppercase to keep things neat and tidy
		//为了保持一致，参数全部转换为大写
		_strupr(argv[iCurrOptionIndex]);

		// Is this command line argument an option?
		//这个命令行参数是否是选项
		if (argv[iCurrOptionIndex][0] == '-')
		{
			// Parse the option and value from the string
			//从字符串中解析选项和值
			int iCurrCharIndex;
			int iOptionSize;
			char cCurrChar;

			// Read the option up till the colon or the end of the string
			//读取选项直至遇到冒号或者字符串结尾
			iCurrCharIndex = 1;
			while (TRUE)
			{
				cCurrChar = argv[iCurrOptionIndex][iCurrCharIndex];
				if (cCurrChar == ':' || cCurrChar == '\0')
					break;
				else
					pstrCurrOption[iCurrCharIndex - 1] = cCurrChar;
				++iCurrCharIndex;
			}
			pstrCurrOption[iCurrCharIndex - 1] = '\0';

			// Read the value till the end of the string, if it has one
			//如果有值的话就读入
			if (strstr(argv[iCurrOptionIndex], ":"))
			{
				++iCurrCharIndex;
				iOptionSize = iCurrCharIndex;

				pstrCurrValue[0] = '\0';
				while (TRUE)
				{
					if (iCurrCharIndex > (int)strlen(argv[iCurrOptionIndex]))
						break;
					else
					{
						cCurrChar = argv[iCurrOptionIndex][iCurrCharIndex];
						pstrCurrValue[iCurrCharIndex - iOptionSize] = cCurrChar;
					}
					++iCurrCharIndex;
				}
				pstrCurrValue[iCurrCharIndex - iOptionSize] = '\0';

				// Make sure the value is valid
				//确保值是合法的
				if (!strlen(pstrCurrValue))
				{
					sprintf(pstrErrorMssg, "Invalid value for -%s option", pstrCurrOption);
					ExitOnError(pstrErrorMssg);
				}
			}

			//! Perform the option's action 执行选项

			// Set the stack size
			//设置堆栈大小
			if (_stricmp(pstrCurrOption, "S") == 0)
			{
				// Convert the value to an integer stack size
				//将值转换为一个整形堆栈大小
				g_ScriptHeader.iStackSize = atoi(pstrCurrValue);
			}

			// Set the priority
			//设定优先级
			else if (_stricmp(pstrCurrOption, "P") == 0)
			{
				//! Determine what type of priority was specified 决定指定什么类型的优先级

				// Low rank 低优先级

				if (_stricmp(pstrCurrValue, PRIORITY_LOW_KEYWORD) == 0)
				{
					g_ScriptHeader.iPriorityType = PRIORITY_LOW;
				}

				// Medium rank 中优先级

				else if (_stricmp(pstrCurrValue, PRIORITY_MED_KEYWORD) == 0)
				{
					g_ScriptHeader.iPriorityType = PRIORITY_MED;
				}

				// High rank 高优先级

				else if (_stricmp(pstrCurrValue, PRIORITY_HIGH_KEYWORD) == 0)
				{
					g_ScriptHeader.iPriorityType = PRIORITY_HIGH;
				}

				// User-defined timeslice
				//用户定义时间片
				else
				{
					g_ScriptHeader.iPriorityType = PRIORITY_USER;
					g_ScriptHeader.iUserPriority = atoi(pstrCurrValue);
				}
			}

			// Preserve the assembly file
			//保留汇编文件
			else if (_stricmp(pstrCurrOption, "A") == 0)
			{
				g_iPreserveOutputFile = TRUE;
			}

			// Don't generate an .XSE executable
			//不要生成XSE执行文件
			else if (_stricmp(pstrCurrOption, "N") == 0)
			{
				g_iGenerateXSE = FALSE;
				g_iPreserveOutputFile = TRUE;
			}

			// Anything else is invalid
			//其他都是不合法的
			else
			{
				sprintf(pstrErrorMssg, "Unrecognized option: \"%s\"", pstrCurrOption);
				ExitOnError(pstrErrorMssg);
			}
		}
	}
}


/// <summary>
/// Initializes the compiler.
/// 初始化编译器
/// </summary>
void Init()
{
	//! Initialize the script header 初始化脚本头
	g_ScriptHeader.iIsMainFuncPresent = FALSE;
	g_ScriptHeader.iStackSize = 0;
	g_ScriptHeader.iPriorityType = PRIORITY_NONE;

	//! Initialize the main settings
	
	// Mark the assembly file for deletion
	//汇编文件删除的标记
	g_iPreserveOutputFile = FALSE;

	// Generate an .XSE executable
	//生成XSE执行体文件
	g_iGenerateXSE = TRUE;

	// Initialize the source code list
	//初始化源代码链表
	InitLinkedList(&g_SourceCode);

	// Initialize the tables
	//初始化函数、符号、字符串表
	InitLinkedList(&g_FuncTable);
	InitLinkedList(&g_SymbolTable);
	InitLinkedList(&g_StringTable);
}

/// <summary>
/// Shuts down the compiler.
/// 关闭编译器
/// </summary>
void ShutDown()
{
	// Free the source code
	//释放
	FreeLinkedList(&g_SourceCode);

	// Free the tables

	FreeLinkedList(&g_FuncTable);
	FreeLinkedList(&g_SymbolTable);
	FreeLinkedList(&g_StringTable);
}

/// <summary>
/// Loads the source file into memory.
/// 加载源代码文件到内存中
/// </summary>
void LoadSourceFile()
{
	//! Open the input file 打开输入文件

	FILE* pSourceFile;

	if (!(pSourceFile = fopen(g_pstrSourceFilename, "r")))
		ExitOnError("Could not open source file for input");

	//! Load the source code 加载源代码文件

	// Loop through each line of code in the file
	//循环代码的每一行
	while (!feof(pSourceFile))
	{
		// Allocate space for the next line
		//为下一行分配空间
		char* pstrCurrLine = (char*)malloc(MAX_SOURCE_LINE_SIZE + 1);

		// Read the line from the file
		//从文件中读取这一行
		fgets(pstrCurrLine, MAX_SOURCE_LINE_SIZE, pSourceFile);

		// Add it to the source code linked list
		//添加到源代码链表中
		AddNode(&g_SourceCode, pstrCurrLine);
	}

	// ---- Close the file
	//关闭文件
	fclose(pSourceFile);
}

/// <summary>
/// Compiles the high-level source file to its XVM assembly equivelent.
/// 将高级语言的原文件编译为XVM汇编
/// </summary>
void CompileSourceFile()
{
	/*
		For the purposes of chapter 14, this function will only serve the purpose of encapsulating our hardcoded script.
		就第 14 章而言，此函数注释的部分仅用于封装我们的硬编码脚本。

	// Hardcode a _Main () function into the table and save its index
	
	//
	int iMainIndex = AddFunc("_Main", FALSE);

	// Hardcode symbols into the table and save their indices

	int iMyGlobalIndex = AddSymbol("MyGlobal", 1, SCOPE_GLOBAL, SYMBOL_TYPE_VAR);
	int iXIndex = AddSymbol("X", 1, iMainIndex, SYMBOL_TYPE_VAR);
	int iYIndex = AddSymbol("Y", 4, iMainIndex, SYMBOL_TYPE_VAR);

	// Allocate strings to hold each line of the high-level script

	char* pstrLine0 = (char*)malloc(MAX_SOURCE_LINE_SIZE);
	strcpy(pstrLine0, "MyGlobal = 2;");

	char* pstrLine1 = (char*)malloc(MAX_SOURCE_LINE_SIZE);
	strcpy(pstrLine1, "X = 8;");

	char* pstrLine2 = (char*)malloc(MAX_SOURCE_LINE_SIZE);
	strcpy(pstrLine2, "Y [ 1 ] = MyGlobal ^ X;");

	// Hardcode the instructions and source annotation into the I-code module

	int iInstrIndex;

	// MyGlobal = 2;

	AddICodeSourceLine(iMainIndex, pstrLine0);
	iInstrIndex = AddICodeInstr(iMainIndex, INSTR_MOV);
	AddVarICodeOp(iMainIndex, iInstrIndex, iMyGlobalIndex);
	AddIntICodeOp(iMainIndex, iInstrIndex, 2);

	// X = 8;

	AddICodeSourceLine(iMainIndex, pstrLine1);
	iInstrIndex = AddICodeInstr(iMainIndex, INSTR_MOV);
	AddVarICodeOp(iMainIndex, iInstrIndex, iXIndex);
	AddIntICodeOp(iMainIndex, iInstrIndex, 8);

	// Y [ 1 ] = MyGlobal ^ X;

	AddICodeSourceLine(iMainIndex, pstrLine2);

	iInstrIndex = AddICodeInstr(iMainIndex, INSTR_EXP);
	AddVarICodeOp(iMainIndex, iInstrIndex, iMyGlobalIndex);
	AddVarICodeOp(iMainIndex, iInstrIndex, iXIndex);

	iInstrIndex = AddICodeInstr(iMainIndex, INSTR_MOV);
	AddArrayIndexAbsICodeOp(iMainIndex, iInstrIndex, iYIndex, 1);
	AddVarICodeOp(iMainIndex, iInstrIndex, iMyGlobalIndex);
	*/
	// Add two temporary variables for evaluating expressions

	g_iTempVar0SymbolIndex = AddSymbol(TEMP_VAR_0, 1, SCOPE_GLOBAL, SYMBOL_TYPE_VAR);
	g_iTempVar1SymbolIndex = AddSymbol(TEMP_VAR_1, 1, SCOPE_GLOBAL, SYMBOL_TYPE_VAR);

	// Parse the source file to create an I-code representation

	ParseSourceCode();
	
}

/// <summary>
/// Prints miscellaneous compilation stats.
/// 打印编译数据
/// </summary>
void PrintCompileStats()
{
	//! Calculate statistics 统计

	// Symbols

	int iVarCount = 0,
		iArrayCount = 0,
		iGlobalCount = 0;

	// Traverse the list to count each symbol type

	for (int iCurrSymbolIndex = 0; iCurrSymbolIndex < g_SymbolTable.iNodeCount; ++iCurrSymbolIndex)
	{
		// Create a pointer to the current symbol structure

		SymbolNode* pCurrSymbol = GetSymbolByIndex(iCurrSymbolIndex);

		// It's an array if the size is greater than 1

		if (pCurrSymbol->iSize > 1)
			++iArrayCount;

		// It's a variable otherwise

		else
			++iVarCount;

		// It's a global if it's stack index is nonnegative

		if (pCurrSymbol->iScope == 0)
			++iGlobalCount;
	}

	// Instructions

	int iInstrCount = 0;

	// Host API Calls

	int iHostAPICallCount = 0;

	// Traverse the list to count each symbol type

	for (int iCurrFuncIndex = 1; iCurrFuncIndex <= g_FuncTable.iNodeCount; ++iCurrFuncIndex)
	{
		// Create a pointer to the current function structure

		FuncNode* pCurrFunc = GetFuncByIndex(iCurrFuncIndex);

		// Determine if the function is part of the host API

		++iHostAPICallCount;

		// Add the function's I-code instructions to the running total

		iInstrCount += pCurrFunc->ICodeStream.iNodeCount;
	}

	// Print out final calculations

	printf("%s created successfully!\n\n", g_pstrOutputFilename);
	printf("Source Lines Processed: %d\n", g_SourceCode.iNodeCount);
	printf("            Stack Size: ");
	if (g_ScriptHeader.iStackSize)
		printf("%d", g_ScriptHeader.iStackSize);
	else
		printf("Default");

	printf("\n");

	printf("              Priority: ");
	switch (g_ScriptHeader.iPriorityType)
	{
	case PRIORITY_USER:
		printf("%dms Timeslice", g_ScriptHeader.iUserPriority);
		break;

	case PRIORITY_LOW:
		printf(PRIORITY_LOW_KEYWORD);
		break;

	case PRIORITY_MED:
		printf(PRIORITY_MED_KEYWORD);
		break;

	case PRIORITY_HIGH:
		printf(PRIORITY_HIGH_KEYWORD);
		break;

	default:
		printf("Default");
		break;
	}
	printf("\n");

	printf("  Instructions Emitted: %d\n", iInstrCount);
	printf("             Variables: %d\n", iVarCount);
	printf("                Arrays: %d\n", iArrayCount);
	printf("               Globals: %d\n", iGlobalCount);
	printf("       String Literals: %d\n", g_StringTable.iNodeCount);
	printf("        Host API Calls: %d\n", iHostAPICallCount);
	printf("             Functions: %d\n", g_FuncTable.iNodeCount);

	printf("      _Main () Present: ");
	if (g_ScriptHeader.iIsMainFuncPresent)
		printf("Yes (Index %d)\n", g_ScriptHeader.iMainFuncIndex);
	else
		printf("No\n");
	printf("\n");
}

//! 调用XASM汇编器(外部)生成最终的可执行文件.XSE

/// <summary>
/// Invokes the XASM assembler to create an executable .XSE file from the resulting .XASM assembly file.
///调用XASM汇编器(外部)生成最终的可执行文件.XSE
/// </summary>
void AssmblOutputFile()
{
	// Command-line parameters to pass to XASM
	//传递给XASM的命令行参数
	char* ppstrCmmndLineParams[3];

	// Set the first parameter to "XASM" (not that it really matters)
	//设置第一个参数为XASM
	ppstrCmmndLineParams[0] = (char*)malloc(strlen("XASM") + 1);
	strcpy(ppstrCmmndLineParams[0], "XASM");

	// Copy the .XASM filename into the second parameter
	//将XASM文件名复制到第二个参数
	ppstrCmmndLineParams[1] = (char*)malloc(strlen(g_pstrOutputFilename) + 1);
	strcpy(ppstrCmmndLineParams[1], g_pstrOutputFilename);

	// Set the third parameter to NULL
	//设置第三个参数为NULL
	ppstrCmmndLineParams[2] = NULL;

	//! Invoke the assembler
	//调用汇编器
	_spawnv(P_WAIT, "XASM.exe", ppstrCmmndLineParams);//该函数创建新进程

	// Free the command-line parameters
	//释放参数
	free(ppstrCmmndLineParams[0]);
	free(ppstrCmmndLineParams[1]);
}

/// <summary>
/// Exits the program.
/// 退出程序
/// </summary>
void Exit()
{
	// Give allocated resources a chance to be freed

	ShutDown();

	// Exit the program

	exit(0);
}


//! 主函数

/// <summary>
/// 主函数
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns></returns>
int main(int argc, char* argv[])
{
	// Print the logo
	//打印logo
	PrintLogo();

	/*
		The following section is commented out to allow our hardcoded script to be added
		to the I-code modules and tables without disturbance from other modules. 
		Of course,this will all be vitally important in chapter 15, when the compiler actually functions as it should.
	*/

	
	// Validate the command line argument count
	//检查参数个数
	if ( argc < 2 )
	{
		// If at least one filename isn't present, print the usage info and exit
		//如果没有，打印使用说明
		PrintUsage ();
		return 0;
	}

	// Verify the filenames
	//校验文件名
	VerifyFilenames ( argc, argv );

	// Initialize the compiler
	//初始化编译器
	Init ();

	// Read in the command line parameters
	//读取命令行参数
	ReadCmmndLineParams ( argc, argv );

	// ---- Begin the compilation process (front end)
	//! 开始编译过程 前端
	
	// Load the source file into memory
	//加载源代码到内存中
	LoadSourceFile ();

	// Preprocess the source file
	//对原文件进行预处理
	PreprocessSourceFile ();
	

	//! Compile the source code to I-code 将源代码转为中间代码
	printf("Compiling %s...\n\n", g_pstrSourceFilename);
	CompileSourceFile();

	//! Emit XVM assembly from the I-code representation (back end) 根据中间代码生成XVM代码 后端

	EmitCode();

	// Print out compilation statistics
	//打印编译统计数据
	PrintCompileStats();

	// Free resources and perform general cleanup
	//释放资源，进行清理
	ShutDown();

	// Invoke XASM to assemble the output file to create the .XSE, unless the user requests otherwise
	//调用XASM汇编输出文件创建.XSE文件
	if (g_iGenerateXSE)
		AssmblOutputFile();

	// Delete the output (assembly) file unless the user requested it to be preserved
	//删除输出汇编文件除非要求保留
	if (!g_iPreserveOutputFile)
		remove(g_pstrOutputFilename);

	return 0;
}
