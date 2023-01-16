//Error-handling routines
//错误处理例程

// ---- Include Files -------------------------------------------------------------------------

#include "error.h"
#include "lexer.h"

// ---- Functions -----------------------------------------------------------------------------


/// <summary>
/// Prints a general error message and exits.
/// 打印错误并退出
/// </summary>
/// <param name="pstrErrorMssg"></param>
void ExitOnError(char* pstrErrorMssg)
{
	// Print the message

	printf("Fatal Error: %s.\n", pstrErrorMssg);

	// Exit the program

	Exit();
}

/// <summary>
/// Prints an code-related error message and exits.
/// 打印错误相关信息并退出
/// </summary>
/// <param name="pstrErrorMssg"></param>
void ExitOnCodeError(char* pstrErrorMssg)
{
	// Print the message

	printf("Error: %s.\n\n", pstrErrorMssg);
	printf("Line %d\n", GetCurrSourceLineIndex());

	// Reduce all of the source line's spaces to tabs so it takes less space and so the karet lines up with the current token properly
	//将源代码所有空白转为tab，减少占用空间。^字符也能正确表示
	char pstrSourceLine[MAX_SOURCE_LINE_SIZE];

	// If the current line is a valid string, copy it into the local source line buffer
	//当前行是一个合法的字符串，复制到局部缓冲区
	char* pstrCurrSourceLine = GetCurrSourceLine();
	if (pstrCurrSourceLine)
		strcpy(pstrSourceLine, pstrCurrSourceLine);
	else
		pstrSourceLine[0] = '\0';

	// If the last character of the line is a line break, clip it
	//如果该行最后一个字符是断行标记，去掉它
	int iLastCharIndex = strlen(pstrSourceLine) - 1;
	if (pstrSourceLine[iLastCharIndex] == '\n')
		pstrSourceLine[iLastCharIndex] = '\0';

	// Loop through each character and replace tabs with spaces
	//循环检查，并用空格替换掉tab
	for (unsigned int iCurrCharIndex = 0; iCurrCharIndex < strlen(pstrSourceLine); ++iCurrCharIndex)
		if (pstrSourceLine[iCurrCharIndex] == '\t')
			pstrSourceLine[iCurrCharIndex] = ' ';

	// Print the offending source line
	//打印出错的代码行
	printf("%s\n", pstrSourceLine);

	// Print a karet at the start of the (presumably) offending lexeme
	//在出错的词前打印一个^
	for (int iCurrSpace = 0; iCurrSpace < GetLexemeStartIndex(); ++iCurrSpace)
		printf(" ");
	printf("^\n");

	// Print message indicating that the script could not be assembled
	//打印信息源码不能转换为汇编
	printf("Could not compile %s.", g_pstrSourceFilename);

	// Exit the program
	//退出
	Exit();
}