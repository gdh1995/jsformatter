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

static inline RealJSFormatter::Char GetStackTop(const RealJSFormatter::ByteStack& stk) {
	return stk.top(); // outside will ensure that it's not empty
}

static inline bool StackTopEq(const RealJSFormatter::ByteStack& stk, const RealJSFormatter::Char eq) {
	return eq == stk.top(); // outside will ensure that it's not empty
}

// ����Ҫ�������ŵĹؼ��ֵļ���
RealJSFormatter::JS_TOKEN_TYPE RealJSFormatter::findSomeKeyword(const Token &str) {
	if (str.equals(_T("if"), 2))
		return JS_IF;
	else if (str.equals(_T("for"), 3))
		return JS_FOR;
	else if (str.equals(_T("return"), 6))
		return JS_HELPER;
	else if (str.equals(_T("function"), 8))
		return JS_FUNCTION;
	else if (str.equals(_T("while"), 5))
		return JS_WHILE;
	else if (str.equals(_T("catch"), 5))
		return JS_CATCH;
	else if (str.equals(_T("switch"), 6))
		return JS_SWITCH;
	else if (str.equals(_T("with"), 4))
		return JS_HELPER;
	return JS_NULL;
}

void RealJSFormatter::Init() const {
	m_line.expand(sc_lineBufferReservedSize);

	m_nIndents = 0;
	m_uhLineIndents = 0;
	m_line.more = INSERT_NULL;
	m_bBlockStmt = true;
	m_bAssign = false;
	m_blockStack.push(JS_NULL); // stay having 1 element when finished
	m_brcNeedStack.push(true); // stay having 1 element when finished
}

// ����/mod: �޸�';'��'{'������; "if"�Ƚ���';'ʱ����ո�
// ����/mod: �����������еĴ����߼�
// TODO: ����"var"���ж�, ����','�����߼�, �ĳ�var�ڿ��Բ�����

//void RealJSFormatter::PutNewLine() {
//	if (!m_struOption.bNotPutCR)
//		m_line.addOrDouble(_T('\r'));
//	m_line.addOrDouble(_T('\n'));
//}

void RealJSFormatter::PutTokenRaw(const Char*const str, int len) const {
	if (m_line.more >= INSERT_NEWLINE) {
		if (m_line.more < INSERT_NULL)
			PutLineBuffer();
		if (m_nIndents < 0)
			m_nIndents = 0; // ��������
		m_uhLineIndents = m_nIndents;
	}
	else if (m_line.more >= INSERT_SPACE) {
		// ����Ҫ�ж�m_line.nempty(), ��ΪҪ��PutTokenA()֮ǰ�жϺ�
		m_line.addOrDouble(' ');
	}
	if (len == 1)
		m_line.addOrDouble(*str);
	else
		m_line.addLine(str, str + len, 0);
}

void RealJSFormatter::PutLine(const Char *start, int len, bool insertBlank) const {
	size_t len2 = 4;
	bool indent = false;
	if (len > 0) {
		len2 += len;
	}
	if (len > 0 || m_struOption.bEmpytLineIndent) { // Fix "JSLint unexpect space" bug
		len2 += m_uhLineIndents * m_struOption.uhChPerInd;
		indent = true;
	}
	m_out->expand(m_out->size() + len2);

	Char *str = m_out->c_str() + m_out->size();

	// TODO: asm it
	if (indent) {
		for (register int i = m_uhLineIndents * m_struOption.uhChPerInd; --i >= 0; )
			*str++ = m_struOption.chIndent; // �������
	}
	if (insertBlank) {
		*str++ = _T(' ');
	}
	// �������
	const Char* const end = start + len;
	while (start < end) {
		*str++ = *start++;
	}
	if (!m_struOption.bNotPutCR)
		*str++ = _T('\r');
	*str++ = _T('\n');
	*str = 0;
	m_out->setLength(str - m_out->c_str());
}

void RealJSFormatter::PopMultiBlock(const Char previousStackTop) const {
	if (m_tokenB == _T(';')) // ��� m_tokenB �� ;����������������������
		return;

	if ((previousStackTop != JS_IF  || m_tokenB.nequals(_T("else"),  4)) &&
		(previousStackTop != JS_DO  || m_tokenB.nequals(_T("while"), 5)) &&
		(previousStackTop != JS_TRY || m_tokenB.nequals(_T("catch"), 5)) )
	{
		// ; �����ܽ������ if, do, while, for, try, catch
		bool next = true;
		do {
			register Char topStack = GetStackTop(m_blockStack);
			if (topStack == JS_IF) {
				if (m_tokenB.equals(_T("else"), 4)) { next = false; }
			} else if (topStack <= JS_ELSE) {
				if (topStack == JS_SWITCH) {
					break; // TODO: check whether it's useful
				}
			} else if (topStack == JS_TRY) {
				if (m_tokenB.equals(_T("catch"), 5)) { next = false; }
			} else if (topStack == JS_DO) {
				--m_nIndents;
				if (m_tokenB.equals(_T("while"), 5)) { break; }
				continue;
			} else {
				break;
			}
			--m_nIndents;
			m_blockStack.pop();
		} while (next);
	}
}

void RealJSFormatter::Go() const {
	StartParse();
	while (GetToken()) {
		switch (m_tokenA.more) {
		case TOKEN_REGULAR:
			PutTokenA(); // ������ʽֱ�������ǰ��û���κ���ʽ
			m_line.more = INSERT_UNKNOWN;
			break;
		case TOKEN_OPER: ProcessOper(); break;
		case TOKEN_ID: ProcessID(); break;
		case TOKEN_STRING: ProcessAndPutString(); break;
		case TOKEN_COMMENT_LINE:
			// ����/mod: �ֶ�����; �˴����ж�m_tokenA.nempty(), ������
			if (m_line.more < INSERT_NULL)
				m_line.more = INSERT_SPACE;
			PutTokenA();
			PutLineBuffer();
			m_line.more = INSERT_NULL;
			break;
		case TOKEN_COMMENT_BLOCK: ProcessAndPutBlockComment(); break;
		case TOKEN_BLANK_LINE:
			if (m_line.more < INSERT_NULL)
				PutLineBuffer();
			PutLine(NULL, 0, 0); // �������
			m_line.more = INSERT_NULL;
			break;
		}
	}
	EndParse();
}

void RealJSFormatter::ProcessOper() const {
	const int alen = m_tokenA.size();
	Char topStack = GetStackTop(m_blockStack);

	if (alen <= 2) {
		if (m_tokenA.findIn(_T("()[]!~^.")) || m_tokenA.equals(_T('!'), _T('!'))) {
			// ()[]!. ����ǰ��û����ʽ�������
			const Char ach0 = m_tokenA[0];
			if ((topStack == JS_ASSIGN || topStack == JS_HELPER) &&
				(ach0 == _T(')') || ach0 == _T(']')))
			{
				if (topStack == JS_ASSIGN)
					--m_nIndents;
				m_blockStack.pop();
				topStack = GetStackTop(m_blockStack);
			}
			if ((topStack == JS_BRACKET && ach0 == _T(')')) ||
				(topStack == JS_SQUARE && ach0 == _T(']')))
			{
				// )] ��Ҫ��ջ����������
				--m_nIndents;
				PutTokenA();

				m_blockStack.pop();
				topStack = GetStackTop(m_blockStack);
				if (topStack == JS_BRACKET)
					++m_nIndents;
				else if (topStack == JS_ASSIGN || topStack == JS_HELPER) {
					m_blockStack.pop();
					topStack = GetStackTop(m_blockStack);
				}	
			}
			else
				PutTokenA();

			if (ach0 == _T(')') && !m_brcNeedStack.top() && topStack <= JS_SWITCH) {
				// ջ���� if, for, while, switch, catch ���ڵȴ� )��֮������������
				m_line.more = INSERT_NEWLINE;
				m_brcNeedStack.pop();
				m_bBlockStmt = false; // �ȴ� statment
				if (topStack == JS_WHILE && m_blockStack._Get_container().end()[-2] == JS_DO) {
					m_blockStack.pop();
					m_blockStack.pop();
					PopMultiBlock(JS_WHILE); // ���� do...while
					topStack = GetStackTop(m_blockStack);
				}
				else
					++m_nIndents;
			}
			else if (ach0 == _T(')') && (m_tokenB == _T('{')))
				m_line.more = INSERT_SPACE; // { ���߻���֮ǰ�����ո�
			else if (ach0 == _T('(') || ach0 == _T('[')) {
				// ([ ��ջ����������
				if (topStack == JS_ASSIGN) {
					if (m_bAssign) {
						m_blockStack.push(JS_HELPER);
						++m_nIndents;
					}
				}
				else if (topStack != JS_BRACKET) // ����/mod: "([...])"�м���һ������
					++m_nIndents;
				m_blockStack.push(ach0 == _T('(') ? JS_BRACKET : JS_SQUARE);
				m_line.more = INSERT_UNKNOWN;
			}
			else
				m_line.more = INSERT_UNKNOWN;
			return;
		}
		if (alen == 1) {
			switch (m_tokenA[0]) {
			case _T(';'):
				if (topStack == JS_ASSIGN) {             
					--m_nIndents;
					m_blockStack.pop();
					topStack = GetStackTop(m_blockStack);
				}
				// ; ���� if, else, while, for, try, catch
				if ( (topStack <= JS_TRY && topStack != JS_SWITCH &&
					(m_blockStack.pop(), true)) ||
					topStack == JS_DO ) // do �ڶ�ȡ�� while ����޸ļ���, ���� do{} Ҳһ��
				{
					if (m_line.more == INSERT_NEWLINE)
						m_line.more = INSERT_SPACE;
					--m_nIndents;
					PopMultiBlock(topStack);
					topStack = GetStackTop(m_blockStack);
				}
				//	if (topStack == _T('t'))
				//		m_blockStack.pop(); // ; Ҳ����������������ʱ��ѹ�� t

				PutTokenA();
				// �������"(...)"��� ';' �ͻ���, ʹ��FORCE_NEWLINE��֤֮��� '{' ���ỹ��ͬһ��
				m_line.more = (topStack != JS_BRACKET) ? INSERT_NEWLINE_SHOULD
					: INSERT_SPACE; // "(...; ...)" �ո�
				return; // ;
			case _T(','):
				if (topStack == JS_ASSIGN) {
					--m_nIndents;
					m_blockStack.pop();
					topStack = GetStackTop(m_blockStack);
				}
				PutTokenA();
				// ����� "{...}" ��ľͻ���
				m_line.more = (topStack == JS_BLOCK) ? INSERT_NEWLINE : INSERT_SPACE;
				return; // ,
			case _T('{'): {
					INSERT_MODE eNextInsert = (m_tokenB == _T('}')) // �� {}
						? INSERT_NONE : INSERT_NEWLINE_SHOULD;
					if (m_line.more == INSERT_NEWLINE) {
						if (m_struOption.bBracketAtNewLine)
							eNextInsert = INSERT_NEWLINE_SHOULD;
						else
							m_line.more = INSERT_SPACE;
					}
					if (topStack <= JS_TRY || topStack == JS_DO) {
						if (m_bBlockStmt)
							m_blockStack.push(JS_HELPER); // ѹ��һ�� JS_HELPER ͳһ״̬
						else {
							m_bBlockStmt = true;
							--m_nIndents;
						}
					}
					else if (topStack == JS_ASSIGN) {
						m_bBlockStmt = true;
						--m_nIndents;
					}
					else if (topStack < JS_ASSIGN) { // "(function ... {"����һ������
						if (m_blockStack._Get_container().end()[-2] == JS_BRACKET)
							--m_nIndents;
					}
					else if (topStack == JS_BRACKET) // ����"({...})"�ж�һ������
						--m_nIndents;
					m_blockStack.push(JS_BLOCK); // ��ջ����������
					PutTokenA();
					++m_nIndents;
					m_line.more = eNextInsert;
				}
				return; // {
			case _T('}'):
				// �����Ĳ��ԣ�} һֱ���� {
				// ���������ٿ���ʹ�� {} ֮������ȷ��
				do {
					m_blockStack.pop();
					if (topStack == JS_BLOCK)
						break;
					else if (topStack <= JS_DO)
						--m_nIndents;
					topStack = GetStackTop(m_blockStack);
				} while (topStack != JS_NULL);
				if (topStack == JS_BLOCK) {
					topStack = GetStackTop(m_blockStack);
					if (topStack <= JS_HELPER)
						m_blockStack.pop();
					// JS_DO: (do nothing); // �����Ѿ�����do ���� while
					--m_nIndents;
				}
				{
					const bool bSpace = (m_line.more > INSERT_NONE && m_line.more < INSERT_NULL)
						&& (m_line.more = INSERT_NEWLINE, !m_struOption.bBracketAtNewLine);
					PutTokenA();
					if (m_tokenB.more == TOKEN_OPER) {
						const Char chB = m_tokenB[0];
						if (m_tokenB.size() == 1 && (chB == _T(';') || chB == _T(',') || chB == _T(')')))
						{ // ����/mod: �Ѿ�ͳһʹ(...{...})��{}ǰ�󲻻���
							m_line.more = INSERT_UNKNOWN; // }, }; })
						}
						else
							m_line.more = INSERT_NEWLINE;
					}
					else if (m_tokenB.more == TOKEN_ID && bSpace && (
						(topStack == JS_IF  && m_tokenB.equals(_T("else"),  4)) ||
						(topStack == JS_DO  && m_tokenB.equals(_T("while"), 5)) ||
						(topStack == JS_TRY && m_tokenB.equals(_T("catch"), 5)) ))
					{
						m_line.more = INSERT_SPACE;
					}
					// ���Է���: JS�﷨����"{xxx}.xxx"���ǲ��Ϸ���, ����"{xxx}"�Ǵʵ仹������
					//else if ( m_tokenB.length() >= 2 && m_tokenB[0] == _T('.') &&
					//	(m_tokenB[1] < _T('0') || m_tokenB[1] > _T('9')) )
					//{ // "{xxx}.xxx"Ҫ��'.'������normal_char
					//	m_line.more = INSERT_UNKNOWN;
					//}
					else
						m_line.more = INSERT_NEWLINE;
				}
				if (topStack != JS_ASSIGN && StackTopEq(m_blockStack, JS_BRACKET))
					++m_nIndents;
				PopMultiBlock(topStack);
				return; // }
			case _T(':'):
				PutTokenA();
				if (topStack == JS_CASE) { // case, default
					m_line.more = INSERT_NEWLINE;
					m_blockStack.pop();
				}
				else
					m_line.more = INSERT_SPACE;
				return;
			}
		}
		else {
			const Char ach0 = m_tokenA[0];
			const Char ach1 = m_tokenA[1];
			if ((ach0 == _T('+') && ach1 == _T('+')) || (ach0 == _T('-') && ach1 == _T('-'))) {
			// || (ach0 ==  _T(':') && ach1 ==  _T(':')) || (ach0 ==  _T('-') && ach1 ==  _T('>'))
				PutTokenA();
				m_line.more = INSERT_UNKNOWN;
				return;
			}
		}
	}

	if (topStack == JS_ASSIGN)
		m_bAssign = true;
	else if (m_tokenA == _T('=')) {
		m_blockStack.push(JS_ASSIGN);
		++m_nIndents;
		m_bAssign = false;
	}

	if (m_line.more < INSERT_SPACE)
		m_line.more = INSERT_SPACE;
	PutTokenA(); // ʣ��Ĳ��������� �ո�oper�ո�
	m_line.more = INSERT_SPACE;
}

void RealJSFormatter::ProcessID() const {
	JS_TOKEN_TYPE token_type1 = JS_NULL;
	if (m_tokenA.equals(_T("else"), 4) && m_tokenB.nequals(_T("if"), 2)) {
		token_type1 = JS_ELSE;
	}
	else if (m_tokenA.equals(_T("do"), 2)) {
		token_type1 = JS_DO;
	}
	else if (m_tokenA.equals(_T("try"), 3)) {
		token_type1 = JS_TRY;
	}
	else if (m_tokenA.equals(_T("finally"), 7)) {
		token_type1 = JS_TRY;
	}
	else {
		if (m_tokenA.equals(_T("function"), 8)) {
			token_type1 = JS_FUNCTION;
			if (StackTopEq(m_blockStack, JS_ASSIGN)) {
				--m_nIndents;
				m_blockStack.top() = JS_FUNCTION;
			} else {
				m_blockStack.push(JS_FUNCTION); // �� function Ҳѹ��ջ������ } ����
			}
		}
		else if (m_tokenA.equals(_T("case"), 4) || m_tokenA.equals(_T("default"), 7)) {
			// case, default ��������һ��
			if (m_line.more == INSERT_NEWLINE)
				m_line.more = INSERT_SPACE;
			--m_nIndents;
			PutTokenA();
			++m_nIndents;
			m_line.more = (m_tokenA[0] != _T('d')) ? INSERT_SPACE : INSERT_UNKNOWN;
			m_blockStack.push(JS_CASE);
			return;
		}
		else if (StackTopEq(m_blockStack, JS_ASSIGN))
			m_bAssign = true;

		PutTokenA();
		if (m_tokenB.more >= TOKEN_ID || m_tokenB == _T('{')) {
			m_line.more = INSERT_SPACE; // '{': such as "return {'a':1};"
			//	if (m_blockStack.top() != _T('v'))
			//		m_blockStack.push(_T('v')); // ��������
			return;
		}
		if (token_type1 == JS_NULL) { token_type1 = findSomeKeyword(m_tokenA); }
		if (token_type1 <= JS_SWITCH) {
			m_blockStack.push(token_type1);
			m_brcNeedStack.push(false);
		}
		m_line.more = (token_type1 != JS_NULL && m_tokenB != _T(';'))
			? INSERT_SPACE : INSERT_UNKNOWN;
		return;
	}
	// do, else (EXCEPT else if), try
	PutTokenA();

	m_blockStack.push(token_type1);
	++m_nIndents; // ���� ()��ֱ������
	m_bBlockStmt = false; // �ȴ� block �ڲ��� statment
	m_line.more = (m_tokenB.more >= TOKEN_ID || m_struOption.bBracketAtNewLine)
		? INSERT_NEWLINE : INSERT_SPACE;
	return;
}

void RealJSFormatter::ProcessAndPutString() const {
	if (StackTopEq(m_blockStack, JS_ASSIGN))
		m_bAssign = true;

	bool bNewLineNotNeeded = true;
	const Char *const end = m_tokenA.c_end();
	const Char *str0;
	for (register const Char *str = m_tokenA.c_str() - 1; end > ++str; ) {
		if (*str == '\r' || *str == '\n') {
			bNewLineNotNeeded = false;
			str0 = str;
			break;
		}
	}
	if (bNewLineNotNeeded)
		PutTokenA();
	else {
		m_out->expand(m_out->size() + m_tokenA.size() + 8);
		PutTokenRaw(m_tokenA.c_str(), str0 - m_tokenA.c_str());
		PutLineBuffer();
		if (*str0 == _T('\r'))
			++str0;
		if (*str0 == _T('\n'))
			++str0;
		for (register const Char *str = str0; ; ) {
			register const Char *str2 = str;
			while(++str2 < end && *str2 != '\r' && *str2 != '\n') {}
			if (str2 < end) {
				m_out->addLine(str, str2, m_struOption.bNotPutCR ? 1 : 3);
				str = str2;
				if (*str == _T('\r'))
					++str;
				if (*str == _T('\n'))
					++str;
			}
			else {
				m_line.addLine(str, str2, 0);
				m_uhLineIndents = 0;
				break;
			}
		}
	}
	// ����/warn: ���'.'���normal_char, ��"."���㲻��TOKEN_OPER����, �˴���Ҫ�ж�'.', 
	m_line.more = (m_tokenB.more >= TOKEN_ID && m_tokenB[0] != _T('.'))
		? INSERT_SPACE : INSERT_UNKNOWN;
}

void RealJSFormatter::ProcessAndPutBlockComment() const {
	const Char *const end = m_tokenA.c_end();
	if (m_line.more < INSERT_NEWLINE) {
		register const Char *str = m_tokenA.c_str() - 1;
		while(end > ++str && *str != _T('\r') && *str != _T('\n')) {}
		if (str >= end) {
			m_line.more = INSERT_SPACE; // �����ж� m_line.nempty()
			PutTokenA();
			return;
		}
		m_line.more = INSERT_NEWLINE;
	}
	
	m_out->expand(m_out->size() + m_tokenA.size() + m_line.size() + 64);
	PutTokenRaw(NULL, 0);

	for (register const Char *str = m_tokenA.c_str(); ; ) {
		while (*str == _T('\t') || *str == _T(' '))
			++str;
		register const Char *str2 = str;
		while(++str2 < end && *str2 != '\r' && *str2 != '\n') {}
		PutLine(str, str2 - str, *str == _T('*'));
		if (str2 < end) {
			str = str2;
			if (*str == _T('\r'))
				++str;
			if (*str == _T('\n'))
				++str;
		}
		else
			break;
	}
	m_line.more = INSERT_NULL;
}

