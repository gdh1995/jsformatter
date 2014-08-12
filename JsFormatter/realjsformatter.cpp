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

template<typename T> inline T GetStackTop(const std::stack<T>& stk)
{
	return stk.top();
}

template<typename T> inline bool StackTopEq(const std::stack<T>& stk, T eq)
{
	// return stk.size() && eq == stk.top();
	return eq == stk.top(); // outside will ensure that it's not empty
}

const RealJSFormatter::StrItemInSet RealJSFormatter::s_specKeywordSet[] = {
	"with", "switch", "catch", "while", "function", "return", "for", "if"
};
const size_t RealJSFormatter::s_len_specKeywordSet[] = {
	4, 6, 5, 5, 8, 6, 3, 2
};

RealJSFormatter::KEY_INDEX RealJSFormatter::findInKeywordSet(const CharString<char> &str) {
	const size_t len = str.length();
	int i = sizeof(s_specKeywordSet) / sizeof(s_specKeywordSet[0]);
	while (--i >= 0) {
		if (len != s_len_specKeywordSet[i]) {
			continue;
		}
		if (str == s_specKeywordSet[i]) {
			return (KEY_INDEX)(i + 1);
		}
	}
	return (KEY_INDEX)0;
}

// return its start position
const char* RealJSFormatter::Trim(const CharString<char> &str, const char ** const pend) {
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
	m_initIndent = NULL;
	m_len_initIndent = 0;

	//m_tokenCount = 0;

	m_lineBuffer.expand(256);

	m_nIndents = 0;
	m_nLineIndents = 0;
	m_bNewLine = false;
	m_bBlockStmt = true;
	m_bAssign = false;
	m_bEmptyBracket = false;
	m_bCommentPut = false;

	m_blockStack.push(' ');
	m_brcNeedStack.push(true);


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
			PutLine(m_lineBuffer.c_str(), TrimRightSpace(m_lineBuffer)); // ����л���
			m_lineBuffer.setLength(0);
			m_lineBuffer.c_str()[0] = 0;

			m_bNewLine = false;
			m_nIndents = m_nIndents < 0 ? 0 : m_nIndents; // ��������
			m_nLineIndents = m_nIndents;
			if(ch == '{' || ch == ',' || ch == ';') // �н�β��ע�ͣ�ʹ��{,;���ò�����
				--m_nLineIndents;
		}

		if(ch == '\n') {
			m_bNewLine = true;
		} else {
			if(m_bNewLine && !m_bCommentPut && ( ch == ',' || ch == ';' ||
				(m_struOption.eBracNL == NO_NEWLINE_BRAC && ch == '{') ) )
			{
				m_bNewLine = false;
			}
			m_lineBuffer.addOrDouble(ch);
		}

	}
}

void RealJSFormatter::PutLine(const char *start, const char *const end)
{
	size_t len2 = 4;
	bool indent = false;
	if(start < end || m_struOption.eEmpytIndent == INDENT_IN_EMPTYLINE) { // Fix "JSLint unexpect space" bug
		len2 += m_len_initIndent + m_nLineIndents * m_struOption.nChPerInd;
		indent = true;
	}
	if (end - start > 0) {
		len2 += end - start;
	}
	out.expand(out.length() + len2);

	char *str = out.c_str() + out.length();

	if (indent) {
		for(register size_t i = -1; ++i < m_len_initIndent; )
			*str++ = m_initIndent[i]; // �����Ԥ����
		for(register int i = m_nLineIndents * m_struOption.nChPerInd; --i >= 0; )
			*str++ = m_struOption.chIndent; // �������
	}
	// �������
	while (start < end) {
		*str++ = *start++;
	}
	if(m_struOption.eCRPut == PUT_CR)
		*str++ = '\r';
	*str++ = '\n';
	out.setLength(str - out.c_str());
}

void RealJSFormatter::PopMultiBlock(char previousStackTop)
{
	if(m_tokenB == ';') // ��� m_tokenB �� ;����������������������
		return;

	if ((previousStackTop != JS_IF  || m_tokenB.strncmp("else",  4)) &&
		(previousStackTop != JS_DO  || m_tokenB.strncmp("while", 5)) &&
		(previousStackTop != JS_TRY || m_tokenB.strncmp("catch", 5)) )
	{
		// ; �����ܿ��ܽ������ if, do, while, for, try, catch
		bool next = true;
		do {
			register char topStack = GetStackTop(m_blockStack);
			if (topStack == JS_IF) {
				if (m_tokenB.equals("else", 4)) { next = false; }
			} else if (topStack == JS_FOR) {
			} else if (topStack == JS_ELSE) {
			} else if (topStack == JS_WHILE) {
			} else if (topStack == JS_TRY) {
				if (m_tokenB.equals("catch", 5)) { next = false; }
			} else if (topStack == JS_CATCH) {
			} else if (topStack == JS_DO) {
				--m_nIndents;
				if (m_tokenB.equals("while", 5)) { break; }
				continue;
			} else {
				break;
			}
			--m_nIndents;
			m_blockStack.pop();
		} while(next);
	}
}

void RealJSFormatter::Go()
{
	StartParse();
	while(GetToken()) {
		m_bHaveNewLine = false;
		char tokenAFirst = m_tokenA[0];
		char tokenBFirst = m_tokenB[0]; // tokenBFirst = m_tokenB.size() ? m_tokenB[0] : 0;
		if(tokenBFirst == '\r')
			tokenBFirst = '\n';
		if(tokenBFirst == '\n' || m_tokenB.more == COMMENT_TYPE_1)
			m_bHaveNewLine = true;

		if(!m_bBlockStmt && m_tokenA.more != COMMENT_TYPE_1 && m_tokenA.more != COMMENT_TYPE_2
			&& m_tokenA != '{' && m_tokenA != '\n')
			m_bBlockStmt = true;

		/*
		* �ο� m_tokenB ������ m_tokenA
		* �������� m_tokenA
		* ��һ��ѭ��ʱ�Զ����� m_tokenB ���� m_tokenA
		*/
		switch(m_tokenA.more)
		{
		case REGULAR_TYPE:
			PutTokenA(); // ������ʽֱ�������ǰ��û���κ���ʽ
			correctCommentFlag();
			break;
		case COMMENT_TYPE_1: case COMMENT_TYPE_2:
			PutTokenA();
			if(m_tokenA[1] == '*') {
				// ����ע��
				if(!m_bHaveNewLine)
					PutLF(); // ��Ҫ����
			} else {
				// ����ע��
				// �϶��ỻ�е�
			}
			// not need to correctCommentFlag();
			m_bCommentPut = true;
			break;
		case OPER_TYPE:
			ProcessOper(tokenAFirst);
			break;
		case STRING_TYPE:
			ProcessString();
			break;
		}
	}
	EndParse();
}

void RealJSFormatter::ProcessOper(const char ach0)
{
	char topStack = GetStackTop(m_blockStack);

	const int alen = m_tokenA.length();

	if (alen <= 2) {
		if(m_tokenA.findIn("()[]!~^.") || m_tokenA.equals('!', '!')) {
			// ()[]!. ����ǰ��û����ʽ�������
			if((topStack == JS_ASSIGN || topStack == JS_HELPER) && (m_tokenA == ')' || m_tokenA == ']')) {
				if(topStack == JS_ASSIGN)
					--m_nIndents;
				m_blockStack.pop();
				topStack = GetStackTop(m_blockStack);
			}
			
			if ((topStack == JS_BRACKET && m_tokenA == ')') ||
				(topStack == JS_SQUARE && m_tokenA == ']'))
			{
				// )] ��Ҫ��ջ����������
				--m_nIndents;
				m_blockStack.pop();
				topStack = GetStackTop(m_blockStack);
				if(topStack == JS_ASSIGN || topStack == JS_HELPER) {
					m_blockStack.pop();
					topStack = GetStackTop(m_blockStack);
				}
			}

			if(m_tokenA == ')' && !m_brcNeedStack.top() &&
				(topStack == JS_IF || topStack == JS_FOR || topStack == JS_WHILE ||
				topStack == JS_SWITCH || topStack == JS_CATCH))
			{
				// ջ���� if, for, while, switch, catch ���ڵȴ� )��֮������������
				// ����Ŀո������Ŀո������� { �ģ�m_bNLBracket Ϊ true ����Ҫ�ո���
				PutTokenA();
				if (m_tokenB != ';')
					PutSpace();
				if (!m_bHaveNewLine)
					PutLF();
				correctCommentFlag();

				//bBracket = true;
				m_brcNeedStack.pop();
				m_bBlockStmt = false; // �ȴ� statment
				if(topStack == JS_WHILE) {
					m_blockStack.pop();
					if(StackTopEq(m_blockStack, JS_DO)) { // m_blockStack.top() == JS_DO
						m_blockStack.pop();
						PopMultiBlock(JS_WHILE); // ���� do...while
						topStack = GetStackTop(m_blockStack);
					} else {
						topStack = JS_WHILE;
						m_blockStack.push(JS_WHILE);
						++m_nIndents;
					}
				} else
					++m_nIndents;
			}
			else {
				PutTokenA(); // �������
				if(m_tokenA == ')' && (m_tokenB == '{' || m_bHaveNewLine)) {
					PutSpace(); // { ���߻���֮ǰ�����ո�
				}
				correctCommentFlag();
			}

			if(m_tokenA == '(' || m_tokenA == '[')
			{
				// ([ ��ջ����������
				if(topStack == JS_ASSIGN)
				{
					if(!m_bAssign)
						--m_nIndents;
					else
						m_blockStack.push(JS_HELPER);
				}
				m_blockStack.push(m_tokenA[0]);
				++m_nIndents;
			}

			return;
		}
		else if (alen == 1 && ach0 != '\n') {
			if(ach0 == ';') {
				if(topStack == JS_ASSIGN) {
					m_blockStack.pop();
					topStack = GetStackTop(m_blockStack);
					--m_nIndents;
				}

				// ; ���� if, else, while, for, try, catch
				if (topStack == JS_IF || topStack == JS_FOR ||
					topStack == JS_WHILE || topStack == JS_CATCH ||
					topStack == JS_ELSE || topStack == JS_TRY)
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

				//	if(m_blockStack.top() == 't')
				//		m_blockStack.pop(); // ; Ҳ����������������ʱ��ѹ�� t

				PutTokenA();
				topStack = GetStackTop(m_blockStack);
				//topStack = m_blockStack.top();
				if(topStack != JS_BRACKET && !m_bHaveNewLine) {
					PutSpace(); // ������� () ��� ; �ͻ���
					PutLF();
				}
				else if(topStack == JS_BRACKET || m_tokenB.more == COMMENT_TYPE_1)
					PutSpace(); // (; ) �ո�
				correctCommentFlag();

				return; // ;
			}
			else if(ach0 == ',') {
				if(topStack == JS_ASSIGN)
				{
					--m_nIndents;
					m_blockStack.pop();
				}
				PutTokenA();
				PutSpace();
				if(StackTopEq(m_blockStack, JS_BLOCK) && !m_bHaveNewLine)
					PutLF(); // ����� {} ���
				correctCommentFlag();

				return; // ,
			}
			else if(ach0 == '{') {
				if (topStack == JS_IF || topStack == JS_FOR ||
					topStack == JS_WHILE || topStack == JS_DO ||
					topStack == JS_ELSE || topStack == JS_SWITCH ||
					topStack == JS_TRY || topStack == JS_CATCH ||
					topStack == JS_ASSIGN)
				{
					if(!m_bBlockStmt || topStack == JS_ASSIGN) { //(topStack == JS_ASSIGN && !m_bAssign))
						//m_blockStack.pop(); // �����Ǹ������������� } ʱһ��
						--m_nIndents;
						m_bBlockStmt = true;
					} else {
						m_blockStack.push(JS_HELPER); // ѹ��һ�� JS_HELPER ͳһ״̬
					}
				}
				else if (topStack == JS_FUNCTION) {
					// ����({...}) �ж�һ������
					m_blockStack.pop(); // ���� JS_FUNCTION
					if(StackTopEq(m_blockStack, JS_BRACKET)) {
						--m_nIndents; // ({ ����һ������
					}
					m_blockStack.push(JS_FUNCTION); // ѹ�� JS_FUNCTION
				}
				else if (topStack == JS_BRACKET) {
					--m_nIndents; // ({ ����һ������
				}

				m_blockStack.push(JS_BLOCK); // ��ջ����������
				++m_nIndents;

				/*
				* { ֮��Ŀո�����֮ǰ�ķ���׼���õ�
				* ����Ϊ�˽�� { ������ʱ��ǰ����һ���ո������
				* ��Ϊ�㷨ֻ����󣬲�����ǰ��
				*/
				if(m_tokenB == '}') {
					// �� {}
					m_bEmptyBracket = true;
					if(m_bNewLine == false && m_struOption.eBracNL == NEWLINE_BRAC &&
						(topStack == JS_IF || topStack == JS_FOR ||
						topStack == JS_WHILE || topStack == JS_SWITCH ||
						topStack == JS_CATCH || topStack == JS_FUNCTION))
					{
						PutSpace(); // ��Щ����£�ǰ�油һ���ո�
					}
					PutTokenA();
				}
				else {
					if (m_struOption.eBracNL == NEWLINE_BRAC && !m_bNewLine)
						PutLF(); // ��Ҫ����
					PutTokenA();
					PutSpace();
					if(!m_bHaveNewLine)
						PutLF();
				}
				correctCommentFlag();

				return; // {
			}
			else if(ach0 == '}') {
				// �����Ĳ��ԣ�} һֱ���� {
				// ���������ٿ���ʹ�� {} ֮������ȷ��
				for (; topStack != JS_BLOCK; topStack = GetStackTop(m_blockStack)) {
					m_blockStack.pop();
					switch(topStack) {
					case JS_IF:		case JS_FOR:	case JS_WHILE:	case JS_ELSE:	case JS_DO:
					case JS_TRY:	case JS_CATCH:	case JS_SWITCH:	case JS_ASSIGN:	case JS_FUNCTION:
					case JS_HELPER:
						--m_nIndents;
						break;
					}
				}

				if(topStack == JS_BLOCK) {
					// ��ջ����С����
					--m_nIndents;
					m_blockStack.pop();
					topStack = GetStackTop(m_blockStack);
					switch(topStack) {
					case JS_IF:		case JS_FOR:	case JS_WHILE:	case JS_ELSE:
					case JS_TRY:	case JS_CATCH:	case JS_SWITCH:	case JS_ASSIGN:
					case JS_FUNCTION:	case JS_HELPER:
						m_blockStack.pop();
						break;
					case JS_DO:
						// �����Ѿ�����do ���� while
						break;
					}
					// // add new lines before 'else' if uncommenting the below
					// topStack = GetStackTop(m_blockStack);
				}

				const bool bb = /**/ false /*/ m_bEmptyBracket /**/;
				if(m_bEmptyBracket) { m_bEmptyBracket = false; }
				else if(!m_bNewLine) { PutLF(); }
				PutTokenA();
				if (!m_bHaveNewLine && m_tokenB != ';' && m_tokenB != ','
					&& (m_struOption.eBracNL == NEWLINE_BRAC || 
					! ( (topStack == JS_IF  && m_tokenB.equals("else",  4)) ||
						(topStack == JS_DO  && m_tokenB.equals("while", 5)) ||
						(topStack == JS_TRY && m_tokenB.equals("catch", 5)) ||
						m_tokenB == ')' ) ))
				{
					PutSpace();
					PutLF();
					if (bb) { PutLF(); }
				}
				else if(m_tokenB.more == STRING_TYPE || m_tokenB.more == COMMENT_TYPE_1) {
					PutSpace(); // Ϊ else ׼���Ŀո�
					if (bb) { PutLF(); }
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
			else if(ach0 == ':' && topStack == JS_CASE) {
				// case, default
				PutTokenA();
				PutSpace();
				if(!m_bHaveNewLine)
					PutLF();
				correctCommentFlag();
				m_blockStack.pop();
				return;
			}
		}
		else {
			const char ach1 = m_tokenA[1];
			if ((ach0 ==  '+' && ach1 ==  '+') ||
				(ach0 ==  '-' && ach1 ==  '-') ||
				(ach0 == '\r' && ach1 == '\n') ||
				(ach0 == '\n' && alen ==   1 ) ||
				(ach0 ==  ':' && ach1 ==  ':') ||
				(ach0 ==  '-' && ach1 ==  '>'))
			{
				PutTokenA();
				correctCommentFlag();
				return;
			}
		}
	}

	if(topStack == JS_ASSIGN) // m_blockStack.top() == JS_ASSIGN
		m_bAssign = true;
	else if(m_tokenA == '=') {
		m_blockStack.push('=');
		++m_nIndents;
		m_bAssign = false;
	}

	PutSpace();
	PutTokenA(); // ʣ��Ĳ��������� �ո�oper�ո�
	PutSpace();
	correctCommentFlag();
}

void RealJSFormatter::ProcessString() {
	if (m_tokenA.equals("case", 4) || m_tokenA.equals("default", 7)) {
		// case, default ��������һ��
		--m_nIndents;
		PutTokenA();
		if (m_tokenA[0] != 'd') { PutSpace(); }
		correctCommentFlag();
		++m_nIndents;
		m_blockStack.push(JS_CASE);
		return;
	}
	else if ( (m_tokenA.equals("else", 4) && m_tokenB.strncmp("if", 2)) ||
		m_tokenA.equals("do", 2) || m_tokenA.equals("try", 3) ||
		m_tokenA.equals("finally", 7))
	{
		// do, else (NOT else if), try
		PutTokenA();
		correctCommentFlag();

		m_blockStack.push(m_tokenA[0] != 'f' ? m_tokenA[0] : JS_TRY);
		++m_nIndents; // ���� ()��ֱ������
		m_bBlockStmt = false; // �ȴ� block �ڲ��� statment
		
		PutSpace();
		if(!m_bHaveNewLine && (m_tokenB.more == STRING_TYPE || m_struOption.eBracNL == NEWLINE_BRAC))
			PutLF();
		return;
	}

	KEY_INDEX ki = KEY_NONE;
	if (m_tokenA.equals("function", 8)) {
		ki = KEY_FUNCTION;
		if(StackTopEq(m_blockStack, JS_ASSIGN)) {
			--m_nIndents;
			m_blockStack.top() = JS_FUNCTION;
		} else {
			m_blockStack.push(JS_FUNCTION); // �� function Ҳѹ��ջ������ } ����
		}
	}
	else if(StackTopEq(m_blockStack, JS_ASSIGN))
		m_bAssign = true;

	if (m_tokenB == '{' || m_tokenB.more == STRING_TYPE || 
		m_tokenB.more == COMMENT_TYPE_1 || m_tokenB.more == COMMENT_TYPE_2)
	{
		PutTokenA();
		PutSpace();
		correctCommentFlag();

		//if(m_blockStack.top() != 'v')
		//m_blockStack.push('v'); // ��������
		return;
	}

	PutTokenA();
	if (ki == KEY_NONE) { ki = findInKeywordSet(m_tokenA); }
	if (ki != KEY_NONE && m_tokenB != ';') { PutSpace(); }
	correctCommentFlag();

	switch (ki) {
	case KEY_IF: case KEY_FOR: case KEY_WHILE: case KEY_SWITCH:
		m_blockStack.push(m_tokenA[0]);
		m_brcNeedStack.push(false);
		break;
	case KEY_CATCH:
		m_blockStack.push(JS_CATCH);
		m_brcNeedStack.push(false);
		break;
	}
}
