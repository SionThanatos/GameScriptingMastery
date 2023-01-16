//Preprocessor module
//Ԥ����ģ��

// ---- Include Files -------------------------------------------------------------------------

#include "preprocessor.h"

// ---- Functions -----------------------------------------------------------------------------



/// <summary>
/// Preprocesses the source file to expand preprocessor directives and strip comments.
/// Ԥ����ԭ�ļ�
/// </summary>
void PreprocessSourceFile()
{
	// Are we inside a block comment?
	//�Ƿ���һ��ע�Ϳ��
	int iInBlockComment = FALSE;

	// Are we inside a string?
	//�Ƿ���һ���ַ�������
	int iInString = FALSE;

	// Node to traverse list
	//���ڱ�����ָ��
	LinkedListNode* pNode;
	pNode = g_SourceCode.pHead;

	// Traverse the source code
	//����Դ����
	while (TRUE)
	{
		// Create local copy of the current line
		//�Ե�ǰ�н��оֲ�����
		char* pstrCurrLine = (char*)pNode->pData;

		//! ɨ��ע��
		for (int iCurrCharIndex = 0; iCurrCharIndex < (int)strlen(pstrCurrLine); ++iCurrCharIndex)
		{
			// If the current character is a quote, toggle the in string flag
			//�����ǰ�ַ���һ�����ţ��������ַ������
			if (pstrCurrLine[iCurrCharIndex] == '"')
			{
				if (iInString)
					iInString = FALSE;
				else
					iInString = TRUE;
			}

			// Check for a single-line comment, and terminate the rest of the line if one is found
			//��鵥��ע�ͣ�������־ͽضϵ�ǰ��
			if (pstrCurrLine[iCurrCharIndex] == '/' &&
				pstrCurrLine[iCurrCharIndex + 1] == '/' &&
				!iInString && !iInBlockComment)
			{
				pstrCurrLine[iCurrCharIndex] = '\n';
				pstrCurrLine[iCurrCharIndex + 1] = '\0';
				break;
			}

			// Check for a block comment
			//���ע�Ϳ�
			if (pstrCurrLine[iCurrCharIndex] == '/' &&
				pstrCurrLine[iCurrCharIndex + 1] == '*' &&
				!iInString && !iInBlockComment)
			{
				iInBlockComment = TRUE;
			}

			// Check for the end of a block comment
			//���ע�Ϳ�β��
			if (pstrCurrLine[iCurrCharIndex] == '*' &&
				pstrCurrLine[iCurrCharIndex + 1] == '/' &&
				iInBlockComment)
			{
				pstrCurrLine[iCurrCharIndex] = ' ';
				pstrCurrLine[iCurrCharIndex + 1] = ' ';
				iInBlockComment = FALSE;
			}

			// If we're inside a block comment, replace the current character with whitespace
			//�����ע�Ϳ��ڣ����ÿո��滻����ǰ�ַ�
			if (iInBlockComment)
			{
				if (pstrCurrLine[iCurrCharIndex] != '\n')
					pstrCurrLine[iCurrCharIndex] = ' ';
			}
		}

		// ---- Move to the next node, and exit the loop if the end of the code is reached
		//�ƶ�����һ���ڵ㣬����������˳�ѭ��
		pNode = pNode->pNext;
		if (!pNode)
			break;
	}

	/*
		Implementation of the #include and #define preprocessor directives could go here
	*/
}