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
#include "alldefs.h"
#include "realjsformatter.h"

static inline RealJSFormatter::Char GetStackTop(const RealJSFormatter::CharStack& stk) {
	return stk.top(); // outside will ensure that it's not empty
}

static inline bool StackTopEq(const RealJSFormatter::CharStack& stk, RealJSFormatter::Char eq) {
	return eq == stk.top(); // outside will ensure that it's not empty
}

// ����Ҫ�������ŵĹؼ��ֵļ���
static const RealJSFormatter::StrItemInSet s_specKeywordSet[] = {
	_T("with"), _T("switch"), _T("catch"), _T("while"), _T("function"), _T("return"), _T("for"), _T("if")
};
static const size_t s_len_specKeywordSet[] = {
	4, 6, 5, 5, 8, 6, 3, 2
};
RealJSFormatter::KEY_INDEX findInKeywordSet(const RealJSFormatter::CharString &str) {
	const size_t len = str.length();
	int i = sizeof(s_specKeywordSet) / sizeof(s_specKeywordSet[0]);
	while (--i >= 0) {
		if (len != s_len_specKeywordSet[i]) {
			continue;
		}
		if (str == s_specKeywordSet[i]) {
			return (RealJSFormatter::KEY_INDEX)(i + 1);
		}
	}
	return (RealJSFormatter::KEY_INDEX)0;
}

// return its start position
const RealJSFormatter::Char* RealJSFormatter::Trim(const CharString &str, const Char ** const pend) {
	register const Char * start = str.c_str();
	register const Char *end = (const Char *) str.length();
	if (((long) end) > 0) {
		end = start + ((long) end);
		for(; start < end; start++) {
			if (*start != _T(' ') && *start != _T('\t') && *start != _T('\n') && *start != _T('\r')) {
				break;
			}
		}
		while(--end >= start) {
			if (*end != _T(' ') && *end != _T('\t') && *end != _T('\n') && *end != _T('\r')) {
				break;
			}
		}
		++end;
	}
	*pend = end;
	return start;
}

// return its end position
const RealJSFormatter::Char* RealJSFormatter::TrimRightSpace(const CharString &str) {
	register const Char *ret = (const Char *) str.length();
	if (((long) ret) > 0) {
		register const Char *const start = str.c_str();
		ret = start + ((long) ret);
		while(--ret >= start) {
			if (*ret != _T(' ') && *ret != _T('\t')) {
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

	m_blockStack.push(_T(' '));
	m_brcNeedStack.push(true);


	//	m_blockMap[string(_T("if"))] = JS_IF;
	//	m_blockMap[string(_T("else"))] = JS_ELSE;
	//	m_blockMap[string(_T("for"))] = JS_FOR;
	//	m_blockMap[string(_T("do"))] = JS_DO;
	//	m_blockMap[string(_T("while"))] = JS_WHILE;
	//	m_blockMap[string(_T("switch"))] = JS_SWITCH;
	//	m_blockMap[string(_T("case"))] = JS_CASE;
	//	m_blockMap[string(_T("default"))] = JS_CASE;
	//	m_blockMap[string(_T("try"))] = JS_TRY;
	//	m_blockMap[string(_T("finally"))] = JS_TRY; // ��ͬ�� try
	//	m_blockMap[string(_T("catch"))] = JS_CATCH;
	//	m_blockMap[string(_T("="))] = JS_ASSIGN;
	//	m_blockMap[string(_T("function"))] = JS_FUNCTION;
	//	m_blockMap[string("{")] = JS_BLOCK;
	//	m_blockMap[string("(")] = JS_BRACKET;
	//	m_blockMap[string("[")] = JS_SQUARE;
}

void RealJSFormatter::PutString(const Char *str, size_t const length)
{
	for(size_t i = 0; i < length; ++i)
	{
		register const Char ch = str[i];
		if ( m_bNewLine && (m_bCommentPut ||
			(ch != _T(',') && ch != _T(';') && (m_struOption.eBracNL == NEWLINE_BRAC || ch != _T('{')))
			) )
		{
			// ���к��治�ǽ����� {,; ��������
			PutLine(m_lineBuffer.c_str(), TrimRightSpace(m_lineBuffer)); // ����л���
			m_lineBuffer.setLength(0);
			m_lineBuffer.c_str()[0] = 0;

			m_bNewLine = false;
			m_nIndents = m_nIndents < 0 ? 0 : m_nIndents; // ��������
			m_nLineIndents = m_nIndents;
			if(ch == _T('{') || ch == _T(',') || ch == _T(';')) // �н�β��ע�ͣ�ʹ��{,;���ò�����
				--m_nLineIndents;
		}

		if(ch == _T('\n')) {
			m_bNewLine = true;
		} else {
			if(m_bNewLine && !m_bCommentPut && ( ch == _T(',') || ch == _T(';') ||
				(m_struOption.eBracNL == NO_NEWLINE_BRAC && ch == _T('{')) ) )
			{
				m_bNewLine = false;
			}
			m_lineBuffer.addOrDouble(ch);
		}

	}
}

void RealJSFormatter::PutLine(const Char *start, const Char *const end)
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

	Char *str = out.c_str() + out.length();

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
		*str++ = _T('\r');
	*str++ = _T('\n');
	out.setLength(str - out.c_str());
}

void RealJSFormatter::PopMultiBlock(const Char previousStackTop)
{
	if(m_tokenB == _T(';')) // ��� m_tokenB �� ;����������������������
		return;

	if ((previousStackTop != JS_IF  || m_tokenB.strncmp(_T("else"),  4)) &&
		(previousStackTop != JS_DO  || m_tokenB.strncmp(_T("while"), 5)) &&
		(previousStackTop != JS_TRY || m_tokenB.strncmp(_T("catch"), 5)) )
	{
		// ; �����ܿ��ܽ������ if, do, while, for, try, catch
		bool next = true;
		do {
			register Char topStack = GetStackTop(m_blockStack);
			if (topStack == JS_IF) {
				if (m_tokenB.equals(_T("else"), 4)) { next = false; }
			} else if (topStack == JS_FOR) {
			} else if (topStack == JS_ELSE) {
			} else if (topStack == JS_WHILE) {
			} else if (topStack == JS_TRY) {
				if (m_tokenB.equals(_T("catch"), 5)) { next = false; }
			} else if (topStack == JS_CATCH) {
			} else if (topStack == JS_DO) {
				--m_nIndents;
				if (m_tokenB.equals(_T("while"), 5)) { break; }
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
		Char tokenAFirst = m_tokenA[0];
		Char tokenBFirst = m_tokenB[0]; // tokenBFirst = m_tokenB.size() ? m_tokenB[0] : 0;
		if(tokenBFirst == _T('\r'))
			tokenBFirst = _T('\n');
		if(tokenBFirst == _T('\n') || m_tokenB.more == TYPE_COMMENT_1)
			m_bHaveNewLine = true;

		if(!m_bBlockStmt && m_tokenA.more != TYPE_COMMENT_1 && m_tokenA.more != TYPE_COMMENT_2
			&& m_tokenA != _T('{') && m_tokenA != _T('\n'))
			m_bBlockStmt = true;

		/*
		* �ο� m_tokenB ������ m_tokenA
		* �������� m_tokenA
		* ��һ��ѭ��ʱ�Զ����� m_tokenB ���� m_tokenA
		*/
		switch(m_tokenA.more)
		{
		case TYPE_REGULAR:
			PutTokenA(); // ������ʽֱ�������ǰ��û���κ���ʽ
			correctCommentFlag();
			break;
		case TYPE_COMMENT_1: case TYPE_COMMENT_2:
			PutTokenA();
			if(m_tokenA[1] == _T('*')) {
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
		case TYPE_OPER:
			ProcessOper(tokenAFirst);
			break;
		case TYPE_STRING:
			ProcessString();
			break;
		}
	}
	EndParse();
}

void RealJSFormatter::ProcessOper(const Char ach0)
{
	Char topStack = GetStackTop(m_blockStack);

	const int alen = m_tokenA.length();

	if (alen <= 2) {
		if(m_tokenA.findIn(_T("()[]!~^.")) || m_tokenA.equals(_T('!'), _T('!'))) {
			// ()[]!. ����ǰ��û����ʽ�������
			if((topStack == JS_ASSIGN || topStack == JS_HELPER) && (m_tokenA == _T(')') || m_tokenA == _T(']'))) {
				if(topStack == JS_ASSIGN)
					--m_nIndents;
				m_blockStack.pop();
				topStack = GetStackTop(m_blockStack);
			}
			
			if ((topStack == JS_BRACKET && m_tokenA == _T(')')) ||
				(topStack == JS_SQUARE && m_tokenA == _T(']')))
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

			if(m_tokenA == _T(')') && !m_brcNeedStack.top() &&
				(topStack == JS_IF || topStack == JS_FOR || topStack == JS_WHILE ||
				topStack == JS_SWITCH || topStack == JS_CATCH))
			{
				// ջ���� if, for, while, switch, catch ���ڵȴ� )��֮������������
				// ����Ŀո������Ŀո������� { �ģ�m_bNLBracket Ϊ true ����Ҫ�ո���
				PutTokenA();
				if (m_tokenB != _T(';'))
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
				if(m_tokenA == _T(')') && (m_tokenB == _T('{') || m_bHaveNewLine)) {
					PutSpace(); // { ���߻���֮ǰ�����ո�
				}
				correctCommentFlag();
			}

			if(m_tokenA == _T('(') || m_tokenA == _T('['))
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
		else if (alen == 1 && ach0 != _T('\n')) {
			if(ach0 == _T(';')) {
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

				//	if(m_blockStack.top() == _T('t'))
				//		m_blockStack.pop(); // ; Ҳ����������������ʱ��ѹ�� t

				PutTokenA();
				topStack = GetStackTop(m_blockStack);
				//topStack = m_blockStack.top();
				if(topStack != JS_BRACKET && !m_bHaveNewLine) {
					PutSpace(); // ������� () ��� ; �ͻ���
					PutLF();
				}
				else if(topStack == JS_BRACKET || m_tokenB.more == TYPE_COMMENT_1)
					PutSpace(); // (; ) �ո�
				correctCommentFlag();

				return; // ;
			}
			else if(ach0 == _T(',')) {
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
			else if(ach0 == _T('{')) {
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
				if(m_tokenB == _T('}')) {
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
			else if(ach0 == _T('}')) {
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
				if (!m_bHaveNewLine && m_tokenB != _T(';') && m_tokenB != _T(',')
					&& (m_struOption.eBracNL == NEWLINE_BRAC || 
					! ( (topStack == JS_IF  && m_tokenB.equals(_T("else"),  4)) ||
						(topStack == JS_DO  && m_tokenB.equals(_T("while"), 5)) ||
						(topStack == JS_TRY && m_tokenB.equals(_T("catch"), 5)) ||
						m_tokenB == _T(')') ) ))
				{
					PutSpace();
					PutLF();
					if (bb) { PutLF(); }
				}
				else if(m_tokenB.more == TYPE_STRING || m_tokenB.more == TYPE_COMMENT_1) {
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
			else if(ach0 == _T(':') && topStack == JS_CASE) {
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
			const Char ach1 = m_tokenA[1];
			if ((ach0 ==  _T('+') && ach1 ==  _T('+')) ||
				(ach0 ==  _T('-') && ach1 ==  _T('-')) ||
				(ach0 == _T('\r') && ach1 == _T('\n')) ||
				(ach0 == _T('\n') && alen ==   1 ) ||
				(ach0 ==  _T(':') && ach1 ==  _T(':')) ||
				(ach0 ==  _T('-') && ach1 ==  _T('>')))
			{
				PutTokenA();
				correctCommentFlag();
				return;
			}
		}
	}

	if(topStack == JS_ASSIGN) // m_blockStack.top() == JS_ASSIGN
		m_bAssign = true;
	else if(m_tokenA == _T('=')) {
		m_blockStack.push(_T('='));
		++m_nIndents;
		m_bAssign = false;
	}

	PutSpace();
	PutTokenA(); // ʣ��Ĳ��������� �ո�oper�ո�
	PutSpace();
	correctCommentFlag();
}

void RealJSFormatter::ProcessString() {
	if (m_tokenA.equals(_T("case"), 4) || m_tokenA.equals(_T("default"), 7)) {
		// case, default ��������һ��
		--m_nIndents;
		PutTokenA();
		if (m_tokenA[0] != _T('d')) { PutSpace(); }
		correctCommentFlag();
		++m_nIndents;
		m_blockStack.push(JS_CASE);
		return;
	}
	else if ( (m_tokenA.equals(_T("else"), 4) && m_tokenB.strncmp(_T("if"), 2)) ||
		m_tokenA.equals(_T("do"), 2) || m_tokenA.equals(_T("try"), 3) ||
		m_tokenA.equals(_T("finally"), 7))
	{
		// do, else (NOT else if), try
		PutTokenA();
		correctCommentFlag();

		m_blockStack.push(m_tokenA[0] != _T('f') ? m_tokenA[0] : JS_TRY);
		++m_nIndents; // ���� ()��ֱ������
		m_bBlockStmt = false; // �ȴ� block �ڲ��� statment
		
		PutSpace();
		if(!m_bHaveNewLine && (m_tokenB.more == TYPE_STRING || m_struOption.eBracNL == NEWLINE_BRAC))
			PutLF();
		return;
	}

	KEY_INDEX ki = KEY_NONE;
	if (m_tokenA.equals(_T("function"), 8)) {
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

	if (m_tokenB == _T('{') || m_tokenB.more == TYPE_STRING || 
		m_tokenB.more == TYPE_COMMENT_1 || m_tokenB.more == TYPE_COMMENT_2)
	{
		PutTokenA();
		PutSpace();
		correctCommentFlag();

		//if(m_blockStack.top() != _T('v'))
		//m_blockStack.push(_T('v')); // ��������
		return;
	}

	PutTokenA();
	if (ki == KEY_NONE) { ki = findInKeywordSet(m_tokenA); }
	if (ki != KEY_NONE && m_tokenB != _T(';')) { PutSpace(); }
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
