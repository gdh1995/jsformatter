/* jsparser.cpp
2012-3-11
Version: 0.9.8

Copyright (c) 2012- SUN Junwen

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
#include "jsparser.h"

// ����/mod: ��ȥ'\n', ��ΪOPER_TOKEN�����ַ����������'\n'
const JSParser::Char JSParser::s_operCharBeforeReg[] = _T("(,=:[!&|?+{};");

//	void JSParser::PrintDebug()
//	{
//		if (!m_debug)
//		{
//			return;
//		}
//		_snprintf(m_debugOutput, sizeof(m_debugOutput) / sizeof(m_debugOutput[0])
//			, "%s%d\n" // "%s%d%s%.3fs\n%.3f%s"
//			, "Processed tokens: ", m_tokenCount
//			// , "\nTime used: ", m_duration
//			// , m_tokenCount / m_duration, " tokens/second\n"
//			);
//		printf("%s", m_debugOutput);
//	}

//	static const JSParser::Byte s_normalMap_0[128] = {
//		/*  + x: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
//		/* 0+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		/* 1+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		/* 2+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		/* 3+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		/* 4+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		/* 5+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		/* 6+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		/* 7+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
//	};

/**
 * ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
 *  (ch >= 'A' && ch <= 'Z') || ch == '_' || ch == '$' ||
 *   ch == '.' || ch  > 126  || ch < 0)
 */
const JSParser::Byte JSParser::s_normalCharMap[128] = {
	/*  + x: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
	/* 0+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 1+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 2+ */ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // '.' == 0x2E;
	/* 3+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	/* 4+ */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 5+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
	/* 6+ */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 7+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1
};

/**
 * IsNormalChar(ch) || IsBlankChar(ch) || IsQuote(ch)
 */
const JSParser::Byte JSParser::s_singleOperNextCharMap[128] = {
	/*  + x: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
	/* 0+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,
	/* 1+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 2+ */ 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 3+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	/* 4+ */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 5+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
	/* 6+ */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 7+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1
};

// ����/mod: ":"��ʶ��Ϊ���ַ�����, ��Ϊ"::"�����������֧��
/**
 * (ch == '(' || ch == ')' ||
 *  ch == '[' || ch == ']' || ch == '{' || ch == '}' ||
 *  ch == ',' || ch == ';' || ch == '~' || ch == ':')
 */
const JSParser::Byte JSParser::s_singleOperMap[128] = {
	/*  + x: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
	/* 0+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 1+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 2+ */ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0,
	/* 3+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
	/* 4+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 5+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0,
	/* 6+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 7+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0
};

void JSParser::GetTokenRaw(int bFlag) const {
	const Char * start1;

	if (bFlag & BoolFlag::PosNeg)
		start1 = CurPos() - 1;
	else {
		start1 = CurPos();
		register Char chB = LastChar();
		for (; chB == _T(' ') || chB == _T('\t'); chB = GetChar()) {}
		if (chB == _T('\r') || chB == _T('\n')) {
			if (chB == _T('\r'))
				chB = GetChar();
			if (chB == _T('\n'))
				chB = GetChar();
			for (; chB == _T(' ') || chB == _T('\t'); chB = GetChar()) {}
			if (chB == _T('\r') || chB == _T('\n')) {
				// ����/mod: �����������еĴ����߼�
				for (; chB = GetChar(), IsBlankChar(chB); ) {}
				m_tokenB.setData(const_cast<Char *>(start1 - 1));
				m_tokenB.setLength(CurPos() - start1);
				m_tokenB.more = TOKEN_BLANK_LINE;
				return;
			}
		}
		start1 = CurPos();
	}
	const Char charA = LastChar();
	if (charA == 0) {
		m_tokenB.setData(_T(""));
		m_tokenB.setLength(0);
		m_tokenB.more = TOKEN_NULL;
		return;
	}
	Char charB = GetChar();
	if (IsNormalChar(charA)) {
		// ����/mod: ��_T('.')��Ϊ���ʵ�һ����
		// ���Ҫ�Ļ�, ��Ҫ��Χ�޸����ɵط�, ������
		if (charA >= _T('0') && charA <= _T('9')) {
			register Char chB = charB;
			for (; (chB >= _T('0') && chB <= _T('9')) || chB == _T('.'); chB = GetChar()) {}
			charB = chB;
		}
		else if (charA == _T('.') && charB >= _T('0') && charB <= _T('9')) {
			register Char chB;
			for (; chB = GetChar(), chB >= _T('0') && chB <= _T('9'); ) {}
			charB = chB;
		}
		else {
			register Char chB = charB;
			for (; chB >= _T('a') && chB <= ('z'); chB = GetChar()) {}
			if (IsNormalChar(chB)) {//TODO:
				m_tokenB.more = TOKEN_COMMON;
				for (; chB = GetChar(), IsNormalChar(chB); ) {}
			}
			else
				m_tokenB.more = TOKEN_ID;
			goto L_notNumber;
		}
		if ((charB | _T('\x20')) == _T('e')) { // ������� 82.e-2, 44.2e+6, 555E6, .32e5 ������
			register Char chB = GetChar();
			if (chB == _T('-') || chB == _T('+'))
				chB = GetChar();
			for (; chB >= _T('0') && chB <= _T('9'); chB = GetChar()) {}
#ifdef _DEBUG
			if (!IsNormalChar(chB))
				throw chB;
#endif
		}
		else { // if ((m_charB | _T('\x20')) == _T('x')) {
			// ����Ҫ���� 0x7F �� 123434; , ��Ϊ x �� oper ֱ����IsNormalChar()��������
			register Char chB = charB;
			for (; IsNormalChar(chB); chB = GetChar()) {}
		}
		m_tokenB.more = TOKEN_COMMON;

		L_notNumber: ;
	}
	else if (IsQuote(charA)) { // ����
		const Char chQuote = charA;
		register Char chB = charB;
		for (; chB && chB != chQuote; chB = GetChar()) {
			if (chB != _T('\\')) // ת���ַ�
				continue;
			chB = GetChar();
			if (chB == _T('\r') || chB == _T('\n'))
				goto L_whenMultiLineString;
		}
		m_tokenB.more = TOKEN_STRING;
		goto L_finishReadString;

		L_whenMultiLineString:
		{
			for (; chB && chB != chQuote; ) { // ����״̬��ȫ�������ֱ�����Ž���
				if (chB == _T('\\')) { // ת���ַ�
					chB = GetChar();
					if (chB == '\r')
						chB = GetChar();
					if (chB != '\n')
						continue;
				}
				chB = GetChar();
			}
			m_tokenB.more = TOKEN_STRING_MULTI_LINE;
		}

		L_finishReadString:
		GetChar();
	}
	else if (charA == _T('/') && (charB == _T('/') || charB == _T('*'))) { // ע��
		if (charB == _T('/')) { // ֱ������
			m_tokenB.more = TOKEN_COMMENT_LINE;
			for (register Char chB; (chB = GetChar()) && chB != _T('\r') && chB != _T('\n'); ) {}
		}
		else { // ֱ�� */
			m_tokenB.more = TOKEN_COMMENT_BLOCK;
			for (register Char chB = GetChar(); chB; ) {
				register Char chA = chB;
				chB = GetChar();
				if (chA == '*' && chB == '/') {
					break;
				}
			}
			GetChar();
		}
	}
	else if (IsSingleOper(charA) || IsSingleOperNextChar(charB)) { // ���ַ�����
		m_tokenB.more = TOKEN_OPER;
	}
	else if (charB == _T('=') || charB == charA) { // ���ַ�����
		m_tokenB.more = TOKEN_OPER;
		register Char charC = GetChar();
		if (charC == _T('=')) {
			if ( charB == _T('<') || charB == _T('>') || charA == _T('=') ||
				(charA == _T('!') && charB == _T('=')) )
			{ // ���ַ� ===, !==, <<=, >>=
				GetChar();
			}
		}
		else if (charC == _T('>') && charB == _T('>')) {
			if (GetChar() == _T('=')) // >>>=
				GetChar();
		}
	}
	// ����/mod: ECMAScript�Ĳ������б�����"->", ��������֮
	//else if ((charA == _T('-') && m_charB == _T('>'))) {
	//m_charB = GetChar();
	//}
	else { // ���ǵ��ַ���
		m_tokenB.more = TOKEN_OPER;
	}

	m_tokenB.setData(const_cast<Char *>(start1 - 1));
	m_tokenB.setLength(CurPos() - start1);
}

void JSParser::PrepareTokenB() const
{
	if (m_tokenB.more != TOKEN_OPER || m_tokenB.empty())
		return;

	switch (m_tokenB[0]) {
	case _T('/'):
		// ����/mod: �ſ�ע�͵�����
		// ����/warn: Ĭ��m_tokenA.more == OPER_TOKENʱ, m_tokenA��Ϊ��
		//     ��Ҫm_tokenA������ֵ������Token���� + OPER_TOKEN����.more��Ĭ��ֵ
		/*
		* �ȴ���һ������
		* m_tokenB[0] == /���� m_tokenB ����ע��
		* m_tokenA ���� STRING (���� m_tokenA == return)
		* ���� m_tokenA �����һ���ַ���������Щ
		*/
		if (m_tokenA.more >= TOKEN_COMMENT_LINE || (m_tokenA.more == TOKEN_OPER &&
			ConstString::findIn(m_tokenA[m_tokenA.size() - 1], s_operCharBeforeReg)) ||
			m_tokenA.equals(_T("return"), 6) ) 
		{
			register Char chB = LastChar();
			//	//	// TODO: what are '*' and '|'
			//	//	if (chB != _T('*') && chB != _T('|')) { // ������ܽ���
			for (; chB; chB = GetChar()) { // ����״̬ȫ�������ֱ�� /
				if (chB == _T('\\')) // ת���ַ�
					GetChar();
				else if (chB == _T('/')) {
					chB = GetChar();
					break;
				}
			}
			// ��ע/info: Ҳ����ֻʶ��Сд"g", "i", "m", ����IsNormalChar���Բ���������Token
			while (IsNormalChar(chB)) // ����� flags ����
				chB = GetChar();
			m_tokenB.setLength(CurPos() - m_tokenB.c_str() - 1);
			m_tokenB.more = TOKEN_REGULAR;
		}
		break;
	case _T('-'): case _T('+'):
		if (m_tokenB.size() != 1) {
			return;
		}
		// ����/mod: ǿ�� m_tokenA Ϊ TOKEN_OPER ���� return
		/*
		* ��� m_tokenB �� -,+ ��
		* ���� m_tokenA �����ַ�����Ҳ����������ʽ
		* ���� m_tokenA ���� ++, --, ], )
		* ���� m_charB ��һ�� NormalChar
		* ��ô m_tokenB ʵ������һ��������
		*/
		if ( IsNormalChar(LastChar()) && ( (m_tokenA.more == TOKEN_OPER && m_tokenA != _T(']') &&
			m_tokenA != _T(')') && m_tokenA.nequals(_T('+'), _T('+')) &&
			m_tokenA.nequals(_T('-'), _T('-'))) || m_tokenA.equals(_T("return"), 6) ) )
		{
			// m_tokenB ʵ������������
			GetTokenRaw(BoolFlag::PosNeg);
		}
		break;
	default:
		break;
	}
}
