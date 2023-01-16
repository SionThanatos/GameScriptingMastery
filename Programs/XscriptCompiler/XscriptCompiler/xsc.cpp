//��������ģ��

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

char g_pstrSourceFilename[MAX_FILENAME_SIZE],	// Source code filename  Դ�����ļ���
g_pstrOutputFilename[MAX_FILENAME_SIZE];	// Executable filename ��ִ���ļ���

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
//Ϊ���ʽ��ֵ��ʱ��ӵ�������ʱ���� Temporary variable symbol indices
int g_iTempVar0SymbolIndex,                     
	g_iTempVar1SymbolIndex;

// ---- Functions -----------------------------------------------------------------------------

/// <summary>
/// Prints out logo/credits information.
/// ��ӡ������Ϣ
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
/// ��ӡ˵��
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
/// У����������ļ���
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
void VerifyFilenames(int argc, char* argv[])
{
	// First make a global copy of the source filename and convert it to uppercase
	//���ȶ�ԭ�ļ����ֽ��и��Ʋ��Ҹ�Ϊ��д
	strcpy(g_pstrSourceFilename, argv[1]);
	_strupr(g_pstrSourceFilename);

	// Check for the presence of the .XSS extension and add it if it's not there
	//����׺.XSS�Ƿ���ڣ�û�еĻ��ͼ���
	if (!strstr(g_pstrSourceFilename, SOURCE_FILE_EXT))
	{
		// The extension was not found, so add it to string
		//û�к�׺�ͼ���
		strcat(g_pstrSourceFilename, SOURCE_FILE_EXT);
	}

	// Was an executable filename specified?
	//�Ƿ�ָ���˿�ִ���ļ�����
	if (argv[2] && argv[2][0] != '-')
	{
		// Yes, so repeat the validation process
		//�ǣ��ٴ���֤����
		strcpy(g_pstrOutputFilename, argv[2]);
		_strupr(g_pstrOutputFilename);

		// Check for the presence of the .XASM extension and add it if it's not there
		//����Ƿ��к�׺.XASM
		if (!strstr(g_pstrOutputFilename, OUTPUT_FILE_EXT))
		{
			// The extension was not found, so add it to string
			//û�оͼ���
			strcat(g_pstrOutputFilename, OUTPUT_FILE_EXT);
		}
	}
	else
	{
		// No, so base it on the source filename
		//û��֮������ִ���ļ����ƣ�����ʹ��ԭ�ļ�����
		// First locate the start of the extension, and use pointer subtraction to find the index
		//�����ҵ���׺��ʼ�ĵط����뿪ʼָ��Ӽ����ƫ��
		int ExtOffset = strrchr(g_pstrSourceFilename, '.') - g_pstrSourceFilename;
		strncpy(g_pstrOutputFilename, g_pstrSourceFilename, ExtOffset);

		// Append null terminator
		//����ս��
		g_pstrOutputFilename[ExtOffset] = '\0';

		// Append executable extension
		//��ӿ�ִ���ļ���׺
		strcat(g_pstrOutputFilename, OUTPUT_FILE_EXT);
	}
}

/// <summary>
/// Reads and verifies the command line parameters.
/// ��ȡ��У�������в���
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
		//Ϊ�˱���һ�£�����ȫ��ת��Ϊ��д
		_strupr(argv[iCurrOptionIndex]);

		// Is this command line argument an option?
		//��������в����Ƿ���ѡ��
		if (argv[iCurrOptionIndex][0] == '-')
		{
			// Parse the option and value from the string
			//���ַ����н���ѡ���ֵ
			int iCurrCharIndex;
			int iOptionSize;
			char cCurrChar;

			// Read the option up till the colon or the end of the string
			//��ȡѡ��ֱ������ð�Ż����ַ�����β
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
			//�����ֵ�Ļ��Ͷ���
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
				//ȷ��ֵ�ǺϷ���
				if (!strlen(pstrCurrValue))
				{
					sprintf(pstrErrorMssg, "Invalid value for -%s option", pstrCurrOption);
					ExitOnError(pstrErrorMssg);
				}
			}

			//! Perform the option's action ִ��ѡ��

			// Set the stack size
			//���ö�ջ��С
			if (_stricmp(pstrCurrOption, "S") == 0)
			{
				// Convert the value to an integer stack size
				//��ֵת��Ϊһ�����ζ�ջ��С
				g_ScriptHeader.iStackSize = atoi(pstrCurrValue);
			}

			// Set the priority
			//�趨���ȼ�
			else if (_stricmp(pstrCurrOption, "P") == 0)
			{
				//! Determine what type of priority was specified ����ָ��ʲô���͵����ȼ�

				// Low rank �����ȼ�

				if (_stricmp(pstrCurrValue, PRIORITY_LOW_KEYWORD) == 0)
				{
					g_ScriptHeader.iPriorityType = PRIORITY_LOW;
				}

				// Medium rank �����ȼ�

				else if (_stricmp(pstrCurrValue, PRIORITY_MED_KEYWORD) == 0)
				{
					g_ScriptHeader.iPriorityType = PRIORITY_MED;
				}

				// High rank �����ȼ�

				else if (_stricmp(pstrCurrValue, PRIORITY_HIGH_KEYWORD) == 0)
				{
					g_ScriptHeader.iPriorityType = PRIORITY_HIGH;
				}

				// User-defined timeslice
				//�û�����ʱ��Ƭ
				else
				{
					g_ScriptHeader.iPriorityType = PRIORITY_USER;
					g_ScriptHeader.iUserPriority = atoi(pstrCurrValue);
				}
			}

			// Preserve the assembly file
			//��������ļ�
			else if (_stricmp(pstrCurrOption, "A") == 0)
			{
				g_iPreserveOutputFile = TRUE;
			}

			// Don't generate an .XSE executable
			//��Ҫ����XSEִ���ļ�
			else if (_stricmp(pstrCurrOption, "N") == 0)
			{
				g_iGenerateXSE = FALSE;
				g_iPreserveOutputFile = TRUE;
			}

			// Anything else is invalid
			//�������ǲ��Ϸ���
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
/// ��ʼ��������
/// </summary>
void Init()
{
	//! Initialize the script header ��ʼ���ű�ͷ
	g_ScriptHeader.iIsMainFuncPresent = FALSE;
	g_ScriptHeader.iStackSize = 0;
	g_ScriptHeader.iPriorityType = PRIORITY_NONE;

	//! Initialize the main settings
	
	// Mark the assembly file for deletion
	//����ļ�ɾ���ı��
	g_iPreserveOutputFile = FALSE;

	// Generate an .XSE executable
	//����XSEִ�����ļ�
	g_iGenerateXSE = TRUE;

	// Initialize the source code list
	//��ʼ��Դ��������
	InitLinkedList(&g_SourceCode);

	// Initialize the tables
	//��ʼ�����������š��ַ�����
	InitLinkedList(&g_FuncTable);
	InitLinkedList(&g_SymbolTable);
	InitLinkedList(&g_StringTable);
}

/// <summary>
/// Shuts down the compiler.
/// �رձ�����
/// </summary>
void ShutDown()
{
	// Free the source code
	//�ͷ�
	FreeLinkedList(&g_SourceCode);

	// Free the tables

	FreeLinkedList(&g_FuncTable);
	FreeLinkedList(&g_SymbolTable);
	FreeLinkedList(&g_StringTable);
}

/// <summary>
/// Loads the source file into memory.
/// ����Դ�����ļ����ڴ���
/// </summary>
void LoadSourceFile()
{
	//! Open the input file �������ļ�

	FILE* pSourceFile;

	if (!(pSourceFile = fopen(g_pstrSourceFilename, "r")))
		ExitOnError("Could not open source file for input");

	//! Load the source code ����Դ�����ļ�

	// Loop through each line of code in the file
	//ѭ�������ÿһ��
	while (!feof(pSourceFile))
	{
		// Allocate space for the next line
		//Ϊ��һ�з���ռ�
		char* pstrCurrLine = (char*)malloc(MAX_SOURCE_LINE_SIZE + 1);

		// Read the line from the file
		//���ļ��ж�ȡ��һ��
		fgets(pstrCurrLine, MAX_SOURCE_LINE_SIZE, pSourceFile);

		// Add it to the source code linked list
		//��ӵ�Դ����������
		AddNode(&g_SourceCode, pstrCurrLine);
	}

	// ---- Close the file
	//�ر��ļ�
	fclose(pSourceFile);
}

/// <summary>
/// Compiles the high-level source file to its XVM assembly equivelent.
/// ���߼����Ե�ԭ�ļ�����ΪXVM���
/// </summary>
void CompileSourceFile()
{
	/*
		For the purposes of chapter 14, this function will only serve the purpose of encapsulating our hardcoded script.
		�͵� 14 �¶��ԣ��˺���ע�͵Ĳ��ֽ����ڷ�װ���ǵ�Ӳ����ű���

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
/// ��ӡ��������
/// </summary>
void PrintCompileStats()
{
	//! Calculate statistics ͳ��

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

//! ����XASM�����(�ⲿ)�������յĿ�ִ���ļ�.XSE

/// <summary>
/// Invokes the XASM assembler to create an executable .XSE file from the resulting .XASM assembly file.
///����XASM�����(�ⲿ)�������յĿ�ִ���ļ�.XSE
/// </summary>
void AssmblOutputFile()
{
	// Command-line parameters to pass to XASM
	//���ݸ�XASM�������в���
	char* ppstrCmmndLineParams[3];

	// Set the first parameter to "XASM" (not that it really matters)
	//���õ�һ������ΪXASM
	ppstrCmmndLineParams[0] = (char*)malloc(strlen("XASM") + 1);
	strcpy(ppstrCmmndLineParams[0], "XASM");

	// Copy the .XASM filename into the second parameter
	//��XASM�ļ������Ƶ��ڶ�������
	ppstrCmmndLineParams[1] = (char*)malloc(strlen(g_pstrOutputFilename) + 1);
	strcpy(ppstrCmmndLineParams[1], g_pstrOutputFilename);

	// Set the third parameter to NULL
	//���õ���������ΪNULL
	ppstrCmmndLineParams[2] = NULL;

	//! Invoke the assembler
	//���û����
	_spawnv(P_WAIT, "XASM.exe", ppstrCmmndLineParams);//�ú��������½���

	// Free the command-line parameters
	//�ͷŲ���
	free(ppstrCmmndLineParams[0]);
	free(ppstrCmmndLineParams[1]);
}

/// <summary>
/// Exits the program.
/// �˳�����
/// </summary>
void Exit()
{
	// Give allocated resources a chance to be freed

	ShutDown();

	// Exit the program

	exit(0);
}


//! ������

/// <summary>
/// ������
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns></returns>
int main(int argc, char* argv[])
{
	// Print the logo
	//��ӡlogo
	PrintLogo();

	/*
		The following section is commented out to allow our hardcoded script to be added
		to the I-code modules and tables without disturbance from other modules. 
		Of course,this will all be vitally important in chapter 15, when the compiler actually functions as it should.
	*/

	
	// Validate the command line argument count
	//����������
	if ( argc < 2 )
	{
		// If at least one filename isn't present, print the usage info and exit
		//���û�У���ӡʹ��˵��
		PrintUsage ();
		return 0;
	}

	// Verify the filenames
	//У���ļ���
	VerifyFilenames ( argc, argv );

	// Initialize the compiler
	//��ʼ��������
	Init ();

	// Read in the command line parameters
	//��ȡ�����в���
	ReadCmmndLineParams ( argc, argv );

	// ---- Begin the compilation process (front end)
	//! ��ʼ������� ǰ��
	
	// Load the source file into memory
	//����Դ���뵽�ڴ���
	LoadSourceFile ();

	// Preprocess the source file
	//��ԭ�ļ�����Ԥ����
	PreprocessSourceFile ();
	

	//! Compile the source code to I-code ��Դ����תΪ�м����
	printf("Compiling %s...\n\n", g_pstrSourceFilename);
	CompileSourceFile();

	//! Emit XVM assembly from the I-code representation (back end) �����м��������XVM���� ���

	EmitCode();

	// Print out compilation statistics
	//��ӡ����ͳ������
	PrintCompileStats();

	// Free resources and perform general cleanup
	//�ͷ���Դ����������
	ShutDown();

	// Invoke XASM to assemble the output file to create the .XSE, unless the user requests otherwise
	//����XASM�������ļ�����.XSE�ļ�
	if (g_iGenerateXSE)
		AssmblOutputFile();

	// Delete the output (assembly) file unless the user requested it to be preserved
	//ɾ���������ļ�����Ҫ����
	if (!g_iPreserveOutputFile)
		remove(g_pstrOutputFilename);

	return 0;
}
