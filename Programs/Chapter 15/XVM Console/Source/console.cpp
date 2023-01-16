//Simple stand-alone XVM that allows scripts to access the console with a simple console output API.
//独立控制台

// ---- Include Files -------------------------------------------------------------------------

#include <stdio.h>
#include <conio.h>

// Include the XVM's header

#include "xvm.h"

// ---- Host API ------------------------------------------------------------------------------

	/******************************************************************************************
	*
	*   HAPI_PrintString ()
	*
	*   Prints a string to the console.
	*/

void HAPI_PrintString(int iThreadIndex)
{
	// Read in the parameters

	char* pstrString = XS_GetParamAsString(iThreadIndex, 0);

	// Print the string

	printf("%s", pstrString);

	// Return to the XVM

	XS_Return(iThreadIndex, 1);
}

/******************************************************************************************
*
*   HAPI_PrintString ()
*
*   Prints a newline to the console.
*/

void HAPI_PrintNewline(int iThreadIndex)
{
	// Print the newline

	printf("\n");

	// Return to the XVM

	XS_Return(iThreadIndex, 0);
}

/******************************************************************************************
*
*   HAPI_PrintTab ()
*
*   Prints a tab to the console.
*/

void HAPI_PrintTab(int iThreadIndex)
{
	// Print the tab

	printf("\t");

	// Return to the XVM

	XS_Return(iThreadIndex, 0);
}

// ---- Main ----------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	// Make sure a filename was passed
	//确保给出了文件信息
	if (argc < 2)
	{
		// Print the logo and usage info
		//打印信息
		printf("XVM Console\n");
		printf("Stand-Alone Console-Based Runtime Environment\n");
		printf("Written by Alex Varanese\n");
		printf("\n");

		printf("Usage:\tXVMCONSOLE Script.XSE\n");
		printf("\n");
		printf("Notes:\n");
		printf("\t- A file extension is required.\n");
		printf("\t- Scripts without a _Main () function will not execute.\n");
		printf("\n");

		// Exit the program

		return 0;
	}

	// Initialize the runtime environment
	//初始化运行环境
	XS_Init();

	// Declare the thread indices
	//声明线程索引
	int iThreadIndex;

	// An error code
	//错误码
	int iErrorCode;

	// Load the specified script
	//装载指定脚本
	iErrorCode = XS_LoadScript(argv[1], iThreadIndex, XS_THREAD_PRIORITY_USER);

	// Check for an error
	//检查错误
	if (iErrorCode != XS_LOAD_OK)
	{
		// Print the error based on the code

		printf("Error: ");

		switch (iErrorCode)
		{
		case XS_LOAD_ERROR_FILE_IO:
			printf("File I/O error");
			break;

		case XS_LOAD_ERROR_INVALID_XSE:
			printf("Invalid .XSE file");
			break;

		case XS_LOAD_ERROR_UNSUPPORTED_VERS:
			printf("Unsupported .XSE version");
			break;

		case XS_LOAD_ERROR_OUT_OF_MEMORY:
			printf("Out of memory");
			break;

		case XS_LOAD_ERROR_OUT_OF_THREADS:
			printf("Out of threads");
			break;
		}

		printf(".\n");
		return 0;
	}

	// Register the console output API
	//注册host api打印输出
	XS_RegisterHostAPIFunc(XS_GLOBAL_FUNC, "PrintString", HAPI_PrintString);
	XS_RegisterHostAPIFunc(XS_GLOBAL_FUNC, "PrintNewline", HAPI_PrintNewline);
	XS_RegisterHostAPIFunc(XS_GLOBAL_FUNC, "PrintTab", HAPI_PrintTab);

	// Start up the script

	XS_StartScript(iThreadIndex);

	// Run the script until a key is pressed

	while (!kbhit())
		XS_RunScripts(200);

	// Free resources and perform general cleanup

	XS_ShutDown();

	return 0;
}