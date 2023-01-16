//Preprocessor module
//预处理模块

// ---- Include Files -------------------------------------------------------------------------

#include "preprocessor.h"

// ---- Functions -----------------------------------------------------------------------------



/// <summary>
/// Preprocesses the source file to expand preprocessor directives and strip comments.
/// 预处理原文件
/// </summary>
void PreprocessSourceFile()
{
	// Are we inside a block comment?
	//是否在一个注释块里？
	int iInBlockComment = FALSE;

	// Are we inside a string?
	//是否在一个字符串里面
	int iInString = FALSE;

	// Node to traverse list
	//用于遍历的指针
	LinkedListNode* pNode;
	pNode = g_SourceCode.pHead;

	// Traverse the source code
	//遍历源代码
	while (TRUE)
	{
		// Create local copy of the current line
		//对当前行进行局部复制
		char* pstrCurrLine = (char*)pNode->pData;

		//! 扫描注释
		for (int iCurrCharIndex = 0; iCurrCharIndex < (int)strlen(pstrCurrLine); ++iCurrCharIndex)
		{
			// If the current character is a quote, toggle the in string flag
			//如果当前字符是一个引号，就设置字符串标记
			if (pstrCurrLine[iCurrCharIndex] == '"')
			{
				if (iInString)
					iInString = FALSE;
				else
					iInString = TRUE;
			}

			// Check for a single-line comment, and terminate the rest of the line if one is found
			//检查单行注释，如果发现就截断当前行
			if (pstrCurrLine[iCurrCharIndex] == '/' &&
				pstrCurrLine[iCurrCharIndex + 1] == '/' &&
				!iInString && !iInBlockComment)
			{
				pstrCurrLine[iCurrCharIndex] = '\n';
				pstrCurrLine[iCurrCharIndex + 1] = '\0';
				break;
			}

			// Check for a block comment
			//检查注释块
			if (pstrCurrLine[iCurrCharIndex] == '/' &&
				pstrCurrLine[iCurrCharIndex + 1] == '*' &&
				!iInString && !iInBlockComment)
			{
				iInBlockComment = TRUE;
			}

			// Check for the end of a block comment
			//检查注释块尾部
			if (pstrCurrLine[iCurrCharIndex] == '*' &&
				pstrCurrLine[iCurrCharIndex + 1] == '/' &&
				iInBlockComment)
			{
				pstrCurrLine[iCurrCharIndex] = ' ';
				pstrCurrLine[iCurrCharIndex + 1] = ' ';
				iInBlockComment = FALSE;
			}

			// If we're inside a block comment, replace the current character with whitespace
			//如果在注释块内，就用空格替换掉当前字符
			if (iInBlockComment)
			{
				if (pstrCurrLine[iCurrCharIndex] != '\n')
					pstrCurrLine[iCurrCharIndex] = ' ';
			}
		}

		// ---- Move to the next node, and exit the loop if the end of the code is reached
		//移动到下一个节点，如果结束就退出循环
		pNode = pNode->pNext;
		if (!pNode)
			break;
	}

	/*
		Implementation of the #include and #define preprocessor directives could go here
	*/
}