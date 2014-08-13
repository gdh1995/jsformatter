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

// ���ݺ���Ҫ�������ŵĹؼ��ֵļ������ж�
RealJSFormatter::JS_TOKEN_TYPE RealJSFormatter::findSomeKeyword(const Token &str) {
	switch(str.size()) {
	case 2:
		if (str.equals(_T('i'), _T('f'))) return JS_IF;
		break;
	case 3:
		if (str.equals(_T("for"), 3)) return JS_FOR;
		break;
	case 4:
		if (str.equals(_T("with"), 4)) return JS_BLOCK; // �˷���ֵ���ñ���һ��ʹ��
		break;
	case 5:
		if (str.equals(_T("while"), 5)) return JS_WHILE;
		else if (str.equals(_T("catch"), 5)) return JS_CATCH;
		break;
	case 6:
		if (str.equals(_T("return"), 6)) return JS_BRACKET; // �˷���ֵ���ñ���һ��ʹ��
		else if (str.equals(_T("switch"), 6)) return JS_SWITCH;
		break;
	case 8:
		if (str.equals(_T("function"), 8)) return JS_FUNCTION;
		break;
	default: break;
	}
	return JS_NULL;
}

void RealJSFormatter::Init() const {
	m_line.expand(sc_lineBufferReservedSize);

	m_nIndents = 0;
	m_uhLineIndents = 0;
	m_line.more = INSERT_NULL;
	m_bMoreIndent = false;
	m_bNeedBracket = false;
	m_blockStack.clear();
	m_blockStack.reserve(64);
	m_blockStack.push_back(JS_NULL); // stay having 1 element when finished
	//m_brcNeedStack.push_back(true); // stay having 1 element when finished
}

// ����/mod: �޸�';'��'{'������; "if"�Ƚ���';'ʱ����ո�
// ����/mod: �����������еĴ����߼�
// TODO: ����"var"���ж�, ����','�����߼�, �ĳ�var�ڿ��Բ�����

void RealJSFormatter::PutTokenRaw(const Char*const str, int len) const {
	if (m_line.more >= INSERT_NEWLINE) {
		if (m_line.more != INSERT_NULL)
			PutBufferLine();
		if (m_nIndents < 0)
			m_nIndents = 0; // ��������
		m_uhLineIndents = m_nIndents;
	}
	else if (m_line.more >= INSERT_SPACE) {
		// ����Ҫ�ж�m_line.nempty(), ��ΪҪ��PutTokenA()֮ǰ�жϺ�
		m_line.addOrDouble(' ');
	}
	if (len > 1)
		m_line.addLine(str, str + len, 0);
	else if (len == 1)
		m_line.addOrDouble(*str);
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
		(previousStackTop != JS_TRY || m_tokenB.nequals(_T("catch"), 5)) &&
		(previousStackTop != JS_DO  || m_tokenB.nequals(_T("while"), 5)) )
	{
		// ; �����ܽ������ if, do, while, for, try, catch
		bool next = true;
		do {
			register Char topStack = StackTop();
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
			StackPop();
		} while (next);
	}
}

void RealJSFormatter::Go() const {
	static void (RealJSFormatter::*const s_func[])() const = {NULL,
		&RealJSFormatter::ProcessOper, &RealJSFormatter::ProcessCommonToken,
		&RealJSFormatter::ProcessID, &RealJSFormatter::ProcessRegular,
		&RealJSFormatter::ProcessOrPutString, &RealJSFormatter::ProcessOrPutString,
		&RealJSFormatter::ProcessLineComment, &RealJSFormatter::ProcessAndPutBlankLine,
		&RealJSFormatter::ProcessAndPutBlockComment,
	};
	StartParse();
	while (GetToken()) {
		(this->*s_func[m_tokenA.more])();
	}
	EndParse();
}

void RealJSFormatter::ProcessOper() const {
	const int alen = m_tokenA.size();
	Char topStack = StackTop();

	if (alen <= 2) {
		if (m_tokenA.findIn(_T("()[]!~^.")) || m_tokenA.equals(_T('!'), _T('!'))) {
			// ()[]!. ����ǰ��û����ʽ�������
			const Char ach0 = m_tokenA[0];
			if ((topStack == JS_BRACKET && ach0 == _T(')')) ||
				(topStack == JS_SQUARE && ach0 == _T(']')))
			{ // )] ��Ҫ��ջ����������
				--m_nIndents;
				PutTokenA();
				StackPop();
				topStack = StackTop();
				if (topStack == JS_BRACKET) // TODO: check
					++m_nIndents;
			}
			else
				PutTokenA();

			if (ach0 == _T(')') && m_bNeedBracket && topStack <= JS_SWITCH) { // !m_brcNeedStack.top()
				// ջ���� if, for, while, switch, catch ���ڵȴ� )��֮������������
				m_line.more = INSERT_NEWLINE;
				m_bNeedBracket = false; // m_brcNeedStack.pop_back();
				//m_bBlockStmt = false; // �ȴ� statment
				if (topStack == JS_WHILE && StackTop2() == JS_DO) {
					StackPop();
					StackPop();
					PopMultiBlock(JS_WHILE); // ���� do...while
					topStack = StackTop();
				}
				else
					++m_nIndents;
			}
			else if (ach0 == _T(')') && (m_tokenB == _T('{')))
				m_line.more = INSERT_SPACE; // { ���߻���֮ǰ�����ո�
			else if (ach0 == _T('(') || ach0 == _T('[')) {
				// ([ ��ջ����������
				// ����/mod: "([...])"�м���һ������
				if (topStack != JS_BRACKET)
					++m_nIndents;
				StackPush(ach0 == _T('(') ? JS_BRACKET : JS_SQUARE);
				m_line.more = INSERT_UNKNOWN;
			}
			else
				m_line.more = INSERT_UNKNOWN;
			return;
		}
		if (alen == 1) {
			switch (m_tokenA[0]) {
			case _T(';'):
				//if (topStack == JS_OLD_START) { // TODO:
				//}
				// ; ���� if, else, while, for, try, catch
				if (topStack != JS_SWITCH && topStack <= JS_DO) {
					if (topStack != JS_DO) // do �ڶ�ȡ�� while ����޸ļ���, ���� do{} Ҳһ��
						StackPop();
					if (m_line.more == INSERT_NEWLINE)
						m_line.more = INSERT_SPACE;
					--m_nIndents;
					PopMultiBlock(topStack);
					topStack = StackTop();
				}
				//	if (topStack == _T('t'))
				//		StackPop(); // ; Ҳ����������������ʱ��ѹ�� t
				// �������"(...)"��� ';' �ͻ���
				if (topStack != JS_BRACKET) {
					if (m_bMoreIndent) {
						m_bMoreIndent = false;
						--m_nIndents;
					}
					PutTokenA();
					m_line.more = INSERT_NEWLINE_SHOULD;
				}
				else {
					PutTokenA();
					m_line.more = INSERT_SPACE; // "(...; ...)" �ո�
				}
				return; // ;
			case _T(','):
				//if (topStack == JS_OLD_START) { // TODO:
				//}
				if (topStack != JS_BRACKET && m_bMoreIndent) {
					m_bMoreIndent = false;
					--m_nIndents;
				}
				PutTokenA();
				// ����� "{...}" ��ľͻ���
				m_line.more = topStack >= JS_BLOCK ? INSERT_NEWLINE : INSERT_SPACE;
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
					if (topStack <= JS_DO)
						--m_nIndents;
					// TODO: check the value of JS_TOKEN_TYPE
					else if (topStack == JS_FUNCTION) { // "(function ... {"����һ������
						if (StackTop2() == JS_BRACKET)
							--m_nIndents;
					}
					else if (topStack == JS_BRACKET) // ����"({...})"�ж�һ������
						--m_nIndents;
					StackPush(JS_BLOCK); // ��ջ����������
					PutTokenA();
					if (m_bMoreIndent && eNextInsert != INSERT_NONE)
						m_bMoreIndent = false;
					else
						++m_nIndents;
					m_line.more = eNextInsert;
				}
				return; // {
			case _T('}'):
				// �����Ĳ��ԣ�} һֱ���� {
				// ���������ٿ���ʹ�� {} ֮������ȷ��
				do {
					StackPop();
					if (topStack == JS_BLOCK)
						break;
					else if (topStack <= JS_FUNCTION)
						--m_nIndents;
					topStack = StackTop();
				} while (topStack != JS_NULL);
				if (topStack == JS_BLOCK) {
					topStack = StackTop();
					if (topStack <= JS_TRY || topStack == JS_FUNCTION)
						StackPop(); // �����Ѿ�����do ���� while
					--m_nIndents;
				}
				{
					const bool bSpace = (m_line.more != INSERT_NONE && m_line.more < INSERT_NULL)
						&& (m_line.more = INSERT_NEWLINE, !m_struOption.bBracketAtNewLine);
					if (m_line.more != INSERT_NONE && m_bMoreIndent) {
						m_bMoreIndent = false;
						--m_nIndents;
					}
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
						(topStack == JS_TRY && m_tokenB.equals(_T("catch"), 5)) ||
						(topStack == JS_DO  && m_tokenB.equals(_T("while"), 5)) ))
					{
						m_line.more = INSERT_SPACE;
					}
					else
						m_line.more = INSERT_NEWLINE;
					// ���Է���: JS�﷨����"{xxx}.xxx"���ǲ��Ϸ���, ����"{xxx}"�Ǵʵ仹������
				}
				if (StackTopEq(JS_BRACKET))
					++m_nIndents;
				PopMultiBlock(topStack);
				return; // }
			case _T(':'):
				PutTokenA();
				if (topStack == JS_CASE) { // case, default
					m_line.more = INSERT_NEWLINE;
					StackPop();
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
			StackPush(JS_FUNCTION); // �� function Ҳѹ��ջ������ } ����
		}
		else if (m_tokenA.equals(_T("case"), 4) || m_tokenA.equals(_T("default"), 7)) {
			// case, default ��������һ��
			if (m_line.more == INSERT_NEWLINE)
				m_line.more = INSERT_SPACE;
			--m_nIndents;
			PutTokenA();
			++m_nIndents;
			m_line.more = (m_tokenA[0] != _T('d')) ? INSERT_SPACE : INSERT_UNKNOWN;
			StackPush(JS_CASE);
			return;
		}

		PutTokenA();
		if (m_tokenB.more >= TOKEN_COMMON || m_tokenB == _T('{')) {
			m_line.more = INSERT_SPACE; // '{': such as "return {'a':1};"
			//	if (StackTop() != _T('v'))
			//		StackPush(_T('v')); // ��������
			return;
		}
		if (token_type1 == JS_NULL) { token_type1 = findSomeKeyword(m_tokenA); }
		if (token_type1 <= JS_SWITCH) {
			StackPush(token_type1);
			m_bNeedBracket = true; // m_brcNeedStack.push_back(false);
		}
		m_line.more = (token_type1 != JS_NULL && m_tokenB != _T(';'))
			? INSERT_SPACE : INSERT_UNKNOWN;
		return;
	}
	// do, else (EXCEPT else if), try
	PutTokenA();

	StackPush(token_type1);
	++m_nIndents; // ���� ()��ֱ������
	m_line.more = (m_tokenB.more >= TOKEN_COMMON || m_struOption.bBracketAtNewLine)
		? INSERT_NEWLINE : INSERT_SPACE;
}

void RealJSFormatter::ProcessCommonToken() const {
	PutTokenA();
	m_line.more = (m_tokenB.more >= TOKEN_COMMON) ? INSERT_SPACE : INSERT_UNKNOWN;
}

void RealJSFormatter::ProcessRegular() const {
	PutTokenA(); // ������ʽֱ�������ǰ��û���κ���ʽ
	if (m_tokenB.more == TOKEN_NULL) {
		const Char *const end = m_tokenA.c_end();
		register const Char *str = m_tokenA.c_str();
		for (; str < end && *str != _T('/'); ) {
			if (*str++ == _T('\\'))
				++str;
		}
		if (str >= end) { // an error end of js document
			if (str > end) // end with single _T('\\')
				m_line.addOrDouble(_T('\\'));
			m_line.addOrDouble(_T('/'));
		}
	}
	m_line.more = (m_tokenB.more >= TOKEN_COMMON && (m_tokenB[0] != _T('.') ||
		(m_tokenB.length() >= 2 && m_tokenB[1] >= _T('0') && m_tokenB[1] <= _T('9'))
		) ) ? INSERT_SPACE : INSERT_UNKNOWN;
}

void RealJSFormatter::ProcessOrPutString() const {
	const Char *const end = m_tokenA.c_end();
	const Char *str0 = m_tokenA.c_str();
	if (m_tokenA.more == TOKEN_STRING)
		PutTokenA();
	else {
		m_out->expand(m_out->size() + m_tokenA.size() + 8);
		for (register const Char *str = str0; ; ) {
			++str;
			if (*str == '\r' || *str == '\n') {
				str0 = str;
				break;
			}
		}
		PutTokenRaw(m_tokenA.c_str(), str0 - m_tokenA.c_str());
		if (*str0 == _T('\r'))
			++str0;
		if (end > str0 && *str0 == _T('\n'))
			++str0;
		if (end > str0)
			PutBufferLine();
		for (register const Char *str = str0; str < end; ) {
			register const Char *str2 = str - 1;
			while(++str2 < end && *str2 != '\r' && *str2 != '\n') {}
			if (str2 < end) {
				m_out->addLine(str, str2, m_struOption.bNotPutCR ? 1 : 3);
				str = str2;
				if (*str == _T('\r'))
					++str;
				if (end > str && *str == _T('\n'))
					++str;
			}
			else {
				m_line.addLine(str, str2, 0);
				m_uhLineIndents = 0;
				break;
			}
		}
	}
	if (m_tokenB.more == TOKEN_NULL) {
		const Char chQuote = m_tokenA[0];
		register const Char *str = str0;
		for (; str < end && *str != chQuote; ) {
			if (*str++ == _T('\\'))
				++str;
		}
		if (str >= end) { // an error end of js document
			if (str > end) // end with single _T('\\')
				m_line.addOrDouble(_T('\\'));
			m_line.addOrDouble(chQuote);
		}
	}
	// ����/warn: ���'.'���normal_char, ��"."���㲻��TOKEN_OPER����, �˴���Ҫ�ж�'.', 
	m_line.more = (m_tokenB.more >= TOKEN_COMMON && (m_tokenB[0] != _T('.') ||
		(m_tokenB.length() >= 2 && m_tokenB[1] >= _T('0') && m_tokenB[1] <= _T('9'))
		) ) ? INSERT_SPACE : INSERT_UNKNOWN;
}

#define CheckMoreIndent1() (m_line.more < INSERT_NEWLINE)
#define CheckMoreIndent2() StackTopGE(JS_BLOCK)

void RealJSFormatter::ProcessLineComment() const {
	bool bOldMoreIndent = m_bMoreIndent;
	// TODO: check whether to use "m_bMoreIndent == false && " in the Release mode
	if (CheckMoreIndent1() && CheckMoreIndent2())
		m_bMoreIndent = true;
	if (m_line.more < INSERT_NULL)
		m_line.more = INSERT_SPACE;
	PutTokenA();
	PutBufferLine();
	m_line.more = INSERT_NULL;
	if (bOldMoreIndent != m_bMoreIndent)
		++m_nIndents;
}

void RealJSFormatter::ProcessAndPutBlankLine() const {
	bool bOldMoreIndent = m_bMoreIndent;
	if (CheckMoreIndent1() && CheckMoreIndent2())
		m_bMoreIndent = true;
	if (m_line.more < INSERT_NULL)
		PutBufferLine();
	PutLine(NULL, 0, 0); // �������
	m_line.more = INSERT_NULL;
	if (bOldMoreIndent != m_bMoreIndent)
		++m_nIndents;
}

void RealJSFormatter::ProcessAndPutBlockComment() const {
	const Char *const end = m_tokenA.c_end();
	if (m_line.more < INSERT_NEWLINE) {
		register const Char *str = m_tokenA.c_str();
		while(end > ++str && *str != _T('\r') && *str != _T('\n')) {}
		if (str == end) {
			m_line.more = INSERT_SPACE; // �����ж� m_line.nempty()
			PutTokenA();
			if (str[-2] != _T('*') || str[-1] != _T('/')) {
				if (str[-1] != '*')
					m_line.addOrDouble(_T('*'));
				m_line.addOrDouble(_T('/'));
			}
			return;
		}
		m_line.more = INSERT_NEWLINE;
		if (CheckMoreIndent2() && m_bMoreIndent == false) {
			m_bMoreIndent = true;
			++m_nIndents;
		}
	}
	m_out->expand(m_out->size() + m_tokenA.size() + m_line.size() + 64);
	PutTokenRaw(NULL, 0);
	
	for (register const Char *str = m_tokenA.c_str(); ; ) {
		while (str < end && *str == _T('\t') || *str == _T(' '))
			++str;
		register const Char *str2 = str;
		while(str2 < end && *str2 != '\r' && *str2 != '\n')
			++str2;
		if (str2 < end) {
			PutLine(str, str2 - str, *str == _T('*'));
			str = str2;
			if (*str == _T('\r'))
				++str;
			if (*str == _T('\n'))
				++str;
			continue;
		}
		if (str < str2)
			PutLine(str, str2 - str, *str == _T('*'));
		if (str2[-2] != _T('*') || str2[-1] != _T('/'))
			PutLine(_T("*/"), 2, true);
		break;
	}
	m_line.more = INSERT_NULL;
}
