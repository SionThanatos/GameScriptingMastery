//Error-handling routines
//����������

// ---- Include Files -------------------------------------------------------------------------

#include "error.h"
#include "lexer.h"

// ---- Functions -----------------------------------------------------------------------------


/// <summary>
/// Prints a general error message and exits.
/// ��ӡ�����˳�
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
/// ��ӡ���������Ϣ���˳�
/// </summary>
/// <param name="pstrErrorMssg"></param>
void ExitOnCodeError(char* pstrErrorMssg)
{
	// Print the message

	printf("Error: %s.\n\n", pstrErrorMssg);
	printf("Line %d\n", GetCurrSourceLineIndex());

	// Reduce all of the source line's spaces to tabs so it takes less space and so the karet lines up with the current token properly
	//��Դ�������пհ�תΪtab������ռ�ÿռ䡣^�ַ�Ҳ����ȷ��ʾ
	char pstrSourceLine[MAX_SOURCE_LINE_SIZE];

	// If the current line is a valid string, copy it into the local source line buffer
	//��ǰ����һ���Ϸ����ַ��������Ƶ��ֲ�������
	char* pstrCurrSourceLine = GetCurrSourceLine();
	if (pstrCurrSourceLine)
		strcpy(pstrSourceLine, pstrCurrSourceLine);
	else
		pstrSourceLine[0] = '\0';

	// If the last character of the line is a line break, clip it
	//����������һ���ַ��Ƕ��б�ǣ�ȥ����
	int iLastCharIndex = strlen(pstrSourceLine) - 1;
	if (pstrSourceLine[iLastCharIndex] == '\n')
		pstrSourceLine[iLastCharIndex] = '\0';

	// Loop through each character and replace tabs with spaces
	//ѭ����飬���ÿո��滻��tab
	for (unsigned int iCurrCharIndex = 0; iCurrCharIndex < strlen(pstrSourceLine); ++iCurrCharIndex)
		if (pstrSourceLine[iCurrCharIndex] == '\t')
			pstrSourceLine[iCurrCharIndex] = ' ';

	// Print the offending source line
	//��ӡ����Ĵ�����
	printf("%s\n", pstrSourceLine);

	// Print a karet at the start of the (presumably) offending lexeme
	//�ڳ���Ĵ�ǰ��ӡһ��^
	for (int iCurrSpace = 0; iCurrSpace < GetLexemeStartIndex(); ++iCurrSpace)
		printf(" ");
	printf("^\n");

	// Print message indicating that the script could not be assembled
	//��ӡ��ϢԴ�벻��ת��Ϊ���
	printf("Could not compile %s.", g_pstrSourceFilename);

	// Exit the program
	//�˳�
	Exit();
}