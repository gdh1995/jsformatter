/* realjsformatter.cpp
2010-12-16

Copyright (c) 2010-2013 SUN Junwen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "realjsformatter.h"

using namespace std;

template<typename T>
inline bool GetStackTop(const stack<T>& stk, T& ret)
{
	if(stk.size() == 0)
		return false;
	ret = stk.top();
	return true;
}

template<typename T>
inline bool StackTopEq(const stack<T>& stk, T eq)
{
	if(stk.size() == 0)
		return false;
	return (eq == stk.top());
}

const RealJSFormatter::StrItemInSet RealJSFormatter::s_specKeywordSet[] = {
	"if",
	"for",
	"while",
	"switch",
	"catch",
	"function",
	"with",
	"return"
};

int RealJSFormatter::findInKeywordSet(const CharString<char> &str) {
	size_t len = str.length();
	int i = sizeof(s_specKeywordSet) / sizeof(s_specKeywordSet[0]);
	while (--i >= 0) {
		if (s_specKeywordSet[i][len] != 0) {
			continue;
		}
		if (str == s_specKeywordSet[i]) {
			return i + 1;
		}
	}
	return 0;
}

// return its start position
const char* RealJSFormatter::Trim(const CharString<char> &str, const char **pend) {
	register const char * start = str.c_str();
	register const char *end = (const char *) str.length();
	if (((long) end) > 0) {
		end = start + ((long) end);
		for(; start < end; start++) {
			if (*start != ' ' && *start != '\t' && *start != '\n' && *start != '\r') {
				break;
			}
		}
		while(--end >= start) {
			if (*end != ' ' && *end != '\t' && *end != '\n' && *end != '\r') {
				break;
			}
		}
		++end;
	}
	*pend = end;
	return start;
}

// return its end position
const char* RealJSFormatter::TrimRightSpace(const CharString<char> &str) {
	register const char *ret = (const char *) str.length();
	if (((long) ret) > 0) {
		register const char *const start = str.c_str();
		ret = start + ((long) ret);
		while(--ret >= start) {
			if (*ret != ' ' && *ret != '\t') {
				break;
			}
		}
		++ret;
	}
	return ret;
}


void RealJSFormatter::Init()
{
	m_initIndent = "";

	m_tokenCount = 0;

	m_lineBuffer.expand(256);
	m_lineBuffer.setLength(0);
	m_lineBuffer.c_str()[0] = 0;

	m_nIndents = 0;
	m_nLineIndents = 0;
	m_bNewLine = false;
	m_bBlockStmt = true;
	m_bAssign = false;
	m_bEmptyBracket = false;
	m_bCommentPut = false;

	//	m_blockMap[string("if")] = JS_IF;
	//	m_blockMap[string("else")] = JS_ELSE;
	//	m_blockMap[string("for")] = JS_FOR;
	//	m_blockMap[string("do")] = JS_DO;
	//	m_blockMap[string("while")] = JS_WHILE;
	//	m_blockMap[string("switch")] = JS_SWITCH;
	//	m_blockMap[string("case")] = JS_CASE;
	//	m_blockMap[string("default")] = JS_CASE;
	//	m_blockMap[string("try")] = JS_TRY;
	//	m_blockMap[string("finally")] = JS_TRY; // ��ͬ�� try
	//	m_blockMap[string("catch")] = JS_CATCH;
	//	m_blockMap[string("=")] = JS_ASSIGN;
	//	m_blockMap[string("function")] = JS_FUNCTION;
	//	m_blockMap[string("{")] = JS_BLOCK;
	//	m_blockMap[string("(")] = JS_BRACKET;
	//	m_blockMap[string("[")] = JS_SQUARE;
}

void RealJSFormatter::PutToken(const char token)
{
	const char str[4 / sizeof(char)] = {token, 0};
	PutString(str, 1);
}

void RealJSFormatter::PutToken(const CharString<char>& token)
{
	PutString(token.c_str(), token.length());
}

void RealJSFormatter::correctCommentFlag()
{
	if(!(m_bCommentPut && m_bNewLine))
		m_bCommentPut = false; // ���һ���ᷢ����ע��֮����κ��������
}

void RealJSFormatter::PutString(const char *str, size_t const length)
{
	for(size_t i = 0; i < length; ++i)
	{
		register char const ch = str[i];
		if ( m_bNewLine && (m_bCommentPut ||
			(ch != ',' && ch != ';' && (m_struOption.eBracNL == NEWLINE_BRAC || ch != '{'))
			) )
		{
			// ���к��治�ǽ����� {,; ��������
			PutLineBuffer(); // ����л���

			m_lineBuffer.setLength(0);
			m_lineBuffer.c_str()[0] = 0;

			m_bNewLine = false;
			m_nIndents = m_nIndents < 0 ? 0 : m_nIndents; // ��������
			m_nLineIndents = m_nIndents;
			if(ch == '{' || ch == ',' || ch == ';') // �н�β��ע�ͣ�ʹ��{,;���ò�����
				--m_nLineIndents;
		}

		if(m_bNewLine && !m_bCommentPut &&
			(ch == ',' || ch == ';' || (m_struOption.eBracNL == NO_NEWLINE_BRAC && ch == '{')))
		{
			m_bNewLine = false;
		}

		if(ch == '\n')
			m_bNewLine = true;
		else
			m_lineBuffer.addOrDouble(ch);
	}
}

void RealJSFormatter::PutLineBuffer(const char *start, const char *const end)
{
	if(start < end || m_struOption.eEmpytIndent == INDENT_IN_EMPTYLINE) // Fix "JSLint unexpect space" bug
	{
		for(size_t i = 0; m_initIndent[i]; ++i)
			PutChar(m_initIndent[i]); // �����Ԥ����
		for(int c = m_nLineIndents * m_struOption.nChPerInd; --c >= 0; )
			PutChar(m_struOption.chIndent); // �������
	}
	// �������
	while (start < end) {
		PutChar(*start++);
	}
	if(m_struOption.eCRPut == PUT_CR)
		PutChar('\r');
	PutChar('\n');
}

void RealJSFormatter::PopMultiBlock(char previousStackTop)
{
	if(m_tokenB == ';') // ��� m_tokenB �� ;����������������������
		return;

	if(!((previousStackTop == JS_IF && m_tokenB == "else") ||
		(previousStackTop == JS_DO && m_tokenB == "while") ||
		(previousStackTop == JS_TRY && m_tokenB == "catch")))
	{
		char topStack;
		// ; �����ܿ��ܽ������ if, do, while, for, try, catch
		while (GetStackTop(m_blockStack, topStack) &&
			(topStack == JS_IF || topStack == JS_FOR || topStack == JS_WHILE ||
			topStack == JS_ELSE || topStack == JS_TRY || topStack == JS_CATCH || topStack == JS_DO))
		{
			if (topStack != JS_DO)
			{
				m_blockStack.pop();
			}
			--m_nIndents;

			if((topStack == JS_IF && m_tokenB == "else") ||
				(topStack == JS_DO && m_tokenB == "while") ||
				(topStack == JS_TRY && m_tokenB == "catch"))
			{
				break; // ֱ���ոս���һ�� if...else, do...while, try...catch
			}
		}
	}
}

void RealJSFormatter::Go()
{
	m_blockStack.push(' ');
	m_brcNeedStack.push(true);

	bool bHaveNewLine;
	char tokenAFirst;
	char tokenBFirst;

	StartParse();

	while(GetToken())
	{
		bHaveNewLine = false; // bHaveNewLine ��ʾ���潫Ҫ���У�m_bNewLine ��ʾ�Ѿ�������
		tokenAFirst = m_tokenA[0];
		tokenBFirst = m_tokenB.size() ? m_tokenB[0] : 0;
		if(tokenBFirst == '\r')
			tokenBFirst = '\n';
		if(tokenBFirst == '\n' || m_tokenB.type == COMMENT_TYPE_1)
			bHaveNewLine = true;

		if(!m_bBlockStmt && m_tokenA.type != COMMENT_TYPE_1 && m_tokenA.type != COMMENT_TYPE_2
			&& m_tokenA != '{' && m_tokenA != '\n')
			m_bBlockStmt = true;

		/*
		* �ο� m_tokenB ������ m_tokenA
		* �������� m_tokenA
		* ��һ��ѭ��ʱ�Զ����� m_tokenB ���� m_tokenA
		*/
		switch(m_tokenA.type)
		{
		case REGULAR_TYPE:
			PutToken(m_tokenA); // ������ʽֱ�������ǰ��û���κ���ʽ
			correctCommentFlag();
			break;
		case COMMENT_TYPE_1:
		case COMMENT_TYPE_2:
			PutToken(m_tokenA);
			if(m_tokenA[1] == '*') {
				// ����ע��
				if(!bHaveNewLine) {
					PutToken('\n'); // ��Ҫ����
				}
			} else {
				// ����ע��
				// �϶��ỻ�е�
			}
			// not need to correctCommentFlag();
			m_bCommentPut = true;
			break;
		case OPER_TYPE:
			ProcessOper(bHaveNewLine, tokenAFirst, tokenBFirst);
			break;
		case STRING_TYPE:
			ProcessString(bHaveNewLine, tokenAFirst, tokenBFirst);
			break;
		}
	}

	const char *end;
	const char *start = Trim(m_lineBuffer, &end);
	if(start < end)
		PutLineBuffer(start, end);

	EndParse();
}

void RealJSFormatter::ProcessOper(bool bHaveNewLine, char tokenAFirst, char tokenBFirst)
{
	char topStack;// = m_blockStack.top();
	GetStackTop(m_blockStack, topStack);

	if(m_tokenA.findIn("()[]!~^.") || m_tokenA == "!!")
	{
		// ()[]!. ����ǰ��û����ʽ�������
		if((topStack == JS_ASSIGN || topStack == JS_HELPER) && (m_tokenA == ')' || m_tokenA == ']'))
		{
			if(topStack == JS_ASSIGN)
				--m_nIndents;
			m_blockStack.pop();
		}
		GetStackTop(m_blockStack, topStack);
		if ((topStack == JS_BRACKET && m_tokenA == ')') ||
			(topStack == JS_SQUARE && m_tokenA == ']'))
		{
			// )] ��Ҫ��ջ����������
			m_blockStack.pop();
			--m_nIndents;
			GetStackTop(m_blockStack, topStack);
			//topStack = m_blockStack.top();
			if(topStack == JS_ASSIGN || topStack == JS_HELPER)
				m_blockStack.pop();
		}

		GetStackTop(m_blockStack, topStack);
		if(m_tokenA == ')' && !m_brcNeedStack.top() &&
			(topStack == JS_IF || topStack == JS_FOR || topStack == JS_WHILE ||
			topStack == JS_SWITCH || topStack == JS_CATCH))
		{
			// ջ���� if, for, while, switch, catch ���ڵȴ� )��֮������������
			// ����Ŀո������Ŀո������� { �ģ�m_bNLBracket Ϊ true ����Ҫ�ո���
			PutToken(m_tokenA);
			if (m_tokenB != ';')
				PutToken(' ');
			if (!bHaveNewLine)
				PutToken('\n');
			correctCommentFlag();

			//bBracket = true;
			m_brcNeedStack.pop();
			m_bBlockStmt = false; // �ȴ� statment
			if(StackTopEq(m_blockStack, JS_WHILE)) //m_blockStack.top() == JS_WHILE
			{
				m_blockStack.pop();
				if(StackTopEq(m_blockStack, JS_DO)) // m_blockStack.top() == JS_DO
				{
					m_blockStack.pop();
					PopMultiBlock(JS_WHILE); // ���� do...while
				}
				else
				{
					m_blockStack.push(JS_WHILE);
					++m_nIndents;
				}
			}
			else
				++m_nIndents;
		}
		else {
			PutToken(m_tokenA); // �������
			if(m_tokenA == ')' && (m_tokenB == '{' || bHaveNewLine)) {
				PutToken(' '); // { ���߻���֮ǰ�����ո�
			}
			correctCommentFlag();
		}

		if(m_tokenA == '(' || m_tokenA == '[')
		{
			// ([ ��ջ����������
			GetStackTop(m_blockStack, topStack);
			if(topStack == JS_ASSIGN)
			{
				if(!m_bAssign)
					--m_nIndents;
				else
					m_blockStack.push(JS_HELPER);
			}
			m_blockStack.push(m_tokenA.c_str()[0]);
			++m_nIndents;
		}

		return;
	}

	if(m_tokenA == ';')
	{
		if(StackTopEq(m_blockStack, JS_ASSIGN))
		{
			m_blockStack.pop();
			--m_nIndents;
		}

		GetStackTop(m_blockStack, topStack);

		// ; ���� if, else, while, for, try, catch
		if(topStack == JS_IF || topStack == JS_FOR ||
			topStack == JS_WHILE || topStack == JS_CATCH)
		{
			m_blockStack.pop();
			--m_nIndents;
			// ����� } ��ͬ���Ĵ���
			PopMultiBlock(topStack);
		}
		else if(topStack == JS_ELSE || topStack == JS_TRY)
		{
			m_blockStack.pop();
			--m_nIndents;
			PopMultiBlock(topStack);
		}
		else if(topStack == JS_DO)
		{
			--m_nIndents;
			PopMultiBlock(topStack);
		}
		// do �ڶ�ȡ�� while ����޸ļ���
		// ���� do{} Ҳһ��

		//if(m_blockStack.top() == 't')
		//m_blockStack.pop(); // ; Ҳ����������������ʱ��ѹ�� t
		
		PutToken(m_tokenA);

		GetStackTop(m_blockStack, topStack);
		//topStack = m_blockStack.top();
		if(topStack != JS_BRACKET && !bHaveNewLine) {
			PutToken(' ');
			PutToken('\n'); // ������� () ��� ; �ͻ���
		}
		else if(topStack == JS_BRACKET || m_tokenB.type == COMMENT_TYPE_1) {
			PutToken(' '); // (; ) �ո�
		}
		correctCommentFlag();

		return; // ;
	}

	if(m_tokenA == ',')
	{
		if(StackTopEq(m_blockStack, JS_ASSIGN)) // m_blockStack.top() == JS_ASSIGN
		{
			--m_nIndents;
			m_blockStack.pop();
		}
		PutToken(m_tokenA);
		PutToken(' ');
		if(StackTopEq(m_blockStack, JS_BLOCK) && !bHaveNewLine) {
			PutToken('\n'); // ����� {} ���
		}
		correctCommentFlag();

		return; // ,
	}

	if(m_tokenA == '{')
	{
		GetStackTop(m_blockStack, topStack);
		if(topStack == JS_IF || topStack == JS_FOR ||
			topStack == JS_WHILE || topStack == JS_DO ||
			topStack == JS_ELSE || topStack == JS_SWITCH ||
			topStack == JS_TRY || topStack == JS_CATCH ||
			topStack == JS_ASSIGN)
		{
			if(!m_bBlockStmt || topStack == JS_ASSIGN) //(topStack == JS_ASSIGN && !m_bAssign))
			{
				//m_blockStack.pop(); // �����Ǹ������������� } ʱһ��
				--m_nIndents;
				m_bBlockStmt = true;
			}
			else
			{
				m_blockStack.push(JS_HELPER); // ѹ��һ�� JS_HELPER ͳһ״̬
			}
		}

		// ����({...}) �ж�һ������
		bool bPrevFunc = (topStack == JS_FUNCTION);
		char fixTopStack = topStack;
		if(bPrevFunc)
		{
			m_blockStack.pop(); // ���� JS_FUNCTION
			GetStackTop(m_blockStack, fixTopStack);
		}

		if(fixTopStack == JS_BRACKET)
		{
			--m_nIndents; // ({ ����һ������
		}

		if(bPrevFunc)
		{
			m_blockStack.push(JS_FUNCTION); // ѹ�� JS_FUNCTION
		}

		m_blockStack.push(JS_BLOCK); // ��ջ����������
		++m_nIndents;

		/*
		* { ֮��Ŀո�����֮ǰ�ķ���׼���õ�
		* ����Ϊ�˽�� { ������ʱ��ǰ����һ���ո������
		* ��Ϊ�㷨ֻ����󣬲�����ǰ��
		*/
		if(m_tokenB == '}')
		{
			// �� {}
			m_bEmptyBracket = true;
			if(m_bNewLine == false && m_struOption.eBracNL == NEWLINE_BRAC &&
				(topStack == JS_IF || topStack == JS_FOR ||
				topStack == JS_WHILE || topStack == JS_SWITCH ||
				topStack == JS_CATCH || topStack == JS_FUNCTION))
			{
				PutToken(' '); // ��Щ����£�ǰ�油һ���ո�
			}
			PutToken(m_tokenA);
			correctCommentFlag();
		}
		else
		{
			if (m_struOption.eBracNL == NEWLINE_BRAC && !m_bNewLine) {
				PutToken('\n'); // ��Ҫ����
			}
			PutToken(m_tokenA);
			PutToken(' ');
			if(!bHaveNewLine)
				PutToken('\n');
			correctCommentFlag();
		}

		return; // {
	}

	if(m_tokenA == '}')
	{
		// �����Ĳ��ԣ�} һֱ���� {
		// ���������ٿ���ʹ�� {} ֮������ȷ��
		while(GetStackTop(m_blockStack, topStack))
		{
			if(topStack == JS_BLOCK)
				break;

			m_blockStack.pop();

			switch(topStack)
			{
			case JS_IF:
			case JS_FOR:
			case JS_WHILE:
			case JS_CATCH:
			case JS_DO:
			case JS_ELSE:
			case JS_TRY:
			case JS_SWITCH:
			case JS_ASSIGN:
			case JS_FUNCTION:
			case JS_HELPER:
				--m_nIndents;
				break;
			}
		}

		if(topStack == JS_BLOCK)
		{
			// ��ջ����С����
			m_blockStack.pop();
			--m_nIndents;

			if(GetStackTop(m_blockStack, topStack))
			{
				switch(topStack)
				{
				case JS_IF:
				case JS_FOR:
				case JS_WHILE:
				case JS_CATCH:
				case JS_ELSE:
				case JS_TRY:
				case JS_SWITCH:
				case JS_ASSIGN:
				case JS_FUNCTION:
				case JS_HELPER:
					m_blockStack.pop();
					break;
				case JS_DO:
					// �����Ѿ�����do ���� while
					break;
				}
			}
			//topStack = m_blockStack.top();
		}
		
		//bool bb = m_bEmptyBracket;
		if(m_bEmptyBracket) {
			m_bEmptyBracket = false;
		}
		else if(!m_bNewLine) {
			PutToken('\n');
		}
		PutToken(m_tokenA);
		if((!bHaveNewLine && m_tokenB != ';' && m_tokenB != ',')
			&& (m_struOption.eBracNL == NEWLINE_BRAC || 
			!((topStack == JS_DO && m_tokenB == "while") ||
			(topStack == JS_IF && m_tokenB == "else") ||
			(topStack == JS_TRY && m_tokenB == "catch") ||
			m_tokenB == ')') ))
		{
			PutToken(' ');
			// if (bb) { PutToken('\n'); }
			PutToken('\n');
		}
		else if(m_tokenB.type == STRING_TYPE || m_tokenB.type == COMMENT_TYPE_1) {
			PutToken(' '); // Ϊ else ׼���Ŀո�
			// if (bb) { PutToken('\n'); }
		}
		else { // }, }; })
		}
		correctCommentFlag();
		// ע�� ) ��Ҫ�����ʱ���� ,; ȡ��ǰ��Ļ���

		if(topStack != JS_ASSIGN && StackTopEq(m_blockStack, JS_BRACKET))
			++m_nIndents;

		PopMultiBlock(topStack);

		return; // }
	}

	if(m_tokenA == "++" || m_tokenA == "--" ||
		m_tokenA == '\n' || m_tokenA == "\r\n")
	{
		PutToken(m_tokenA);
		correctCommentFlag();
		return;
	}

	if(m_tokenA == ':' && StackTopEq(m_blockStack, JS_CASE)) //m_blockStack.top() == JS_CASE
	{
		PutToken(m_tokenA);
		PutToken(' ');
		// case, default
		if(!bHaveNewLine) {
			PutToken('\n');
		}
		correctCommentFlag();
		m_blockStack.pop();
		return;
	}

	if(m_tokenA == "::" || m_tokenA == "->")
	{
		PutToken(m_tokenA);
		correctCommentFlag();
		return;
	}

	if(StackTopEq(m_blockStack, JS_ASSIGN)) // m_blockStack.top() == JS_ASSIGN
		m_bAssign = true;

	if(m_tokenA == '=' && !StackTopEq(m_blockStack, JS_ASSIGN)) // m_blockStack.top() != JS_ASSIGN)
	{
		m_blockStack.push(m_tokenA.c_str()[0]);
		++m_nIndents;
		m_bAssign = false;
	}

	PutToken(' ');
	PutToken(m_tokenA); // ʣ��Ĳ��������� �ո�oper�ո�
	PutToken(' ');
	correctCommentFlag();
}

void RealJSFormatter::ProcessString(bool bHaveNewLine, char tokenAFirst, char tokenBFirst)
{
	if(m_tokenA == "case" || m_tokenA == "default")
	{
		// case, default ��������һ��
		--m_nIndents;
		PutToken(m_tokenA);
		if (m_tokenA == "case") {
			PutToken(' ');
		}
		correctCommentFlag();
		++m_nIndents;
		m_blockStack.push(JS_CASE);
		return;
	}

	if (m_tokenA == "do" ||
		(m_tokenA == "else" && m_tokenB != "if") ||
		m_tokenA == "try" ||
		m_tokenA == "finally")
	{
		// do, else (NOT else if), try
		PutToken(m_tokenA);
		correctCommentFlag();

		m_blockStack.push(m_tokenA != "finally" ? m_tokenA.c_str()[0] : JS_TRY);
		++m_nIndents; // ���� ()��ֱ������
		m_bBlockStmt = false; // �ȴ� block �ڲ��� statment

		PutString(" ", 1);
		if(!bHaveNewLine && (m_tokenB.type == STRING_TYPE || m_struOption.eBracNL == NEWLINE_BRAC))
			PutString("\n", 1);

		return;
	}

	if(m_tokenA == "function")
	{
		if(StackTopEq(m_blockStack, JS_ASSIGN)) // m_blockStack.top() == JS_ASSIGN)
		{
			--m_nIndents;
			m_blockStack.pop();
		}
		m_blockStack.push(JS_FUNCTION); // �� function Ҳѹ��ջ������ } ����
	}

	if(StackTopEq(m_blockStack, JS_ASSIGN)) //m_blockStack.top() == JS_ASSIGN
		m_bAssign = true;

	if(m_tokenB.type == STRING_TYPE || 
		m_tokenB.type == COMMENT_TYPE_1 ||
		m_tokenB.type == COMMENT_TYPE_2 ||
		m_tokenB == '{')
	{
		PutToken(m_tokenA);
		PutToken(' ');
		correctCommentFlag();

		//if(m_blockStack.top() != 'v')
		//m_blockStack.push('v'); // ��������
		return;
	}
	
	PutToken(m_tokenA);
	if(m_tokenB != ';' && findInKeywordSet(m_tokenA))
	{
		PutToken(' ');
	}
	correctCommentFlag();

	if(m_tokenA == "if" || m_tokenA == "for" ||
		m_tokenA == "while" || m_tokenA == "catch")
	{
		// �ȴ� ()��() ��������ܼ�����
		m_brcNeedStack.push(false);
		m_blockStack.push(m_tokenA != "catch" ? m_tokenA.c_str()[0] : JS_CATCH);

	}

	if(m_tokenA == "switch")
	{
		//bBracket = false;
		m_brcNeedStack.push(false);
		m_blockStack.push(JS_SWITCH);
	}
}
