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
//			, "processed tokens: ", m_tokenCount
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

void JSParser::getTokenRaw(int bFlag) const {
	const Char * start1;

	if (bFlag == BoolFlag::PosNeg)
		start1 = m_tokenB + 1;
	else {
		register Char chB = lastChar();
		for (; chB == _T(' ') || chB == _T('\t'); chB = getChar()) {}
		if (chB == _T('\r') || chB == _T('\n')) {
			if (chB == _T('\r'))
				chB = getChar();
			if (chB == _T('\n'))
				chB = getChar();
			start1 = curPos();
			for (; chB == _T(' ') || chB == _T('\t'); chB = getChar()) {}
			if (chB == _T('\r') || chB == _T('\n')) {
				// ����/mod: �����������еĴ����߼�
				for (; chB = getChar(), IsBlankChar(chB); ) {}
				lastChar(chB);
				m_tokenB.setData(_T("\n\n"));
				m_tokenB.setLength(2);
				m_tokenB.more = TOKEN_BLANK_LINE;
				goto L_finishGetToken;
			}
			bFlag = BoolFlag::InNewLine;
		}
		checkEnd();
		start1 = curPos();
		lastChar(chB); // ��3�е�˳��û��ϵ
	}
	const Char charA = lastChar();
	Char charB = getChar();
	if (IsNormalChar(charA)) {
		// ����/mod: ��_T('.')��Ϊ���ʵ�һ����
		// ���Ҫ�Ļ�, ��Ҫ��Χ�޸����ɵط�, ������
		if (charA >= _T('0') && charA <= _T('9')) {
			register Char chB = charB;
			for (; (chB >= _T('0') && chB <= _T('9')) || chB == _T('.'); chB = getChar()) {}
			charB = chB;
		}
		else if (charA == _T('.') && charB >= _T('0') && charB <= _T('9')) {
			register Char chB;
			for (; chB = getChar(), chB >= _T('0') && chB <= _T('9'); ) {}
			charB = chB;
		}
		else {
			register Char chB = charB;
			for (; chB >= _T('a') && chB <= ('z'); chB = getChar()) {}
			if (IsNormalChar(chB)) {
				m_tokenB.more = TOKEN_COMMON;
				for (; chB = getChar(), IsNormalChar(chB); ) {}
			}
			else if (bFlag == BoolFlag::PosNeg || charA < _T('a') || charA > ('z'))
				m_tokenB.more = TOKEN_COMMON;
			else
				m_tokenB.more = TOKEN_ID;
			lastChar(chB);
			goto L_notNumber;
		}
		if ((charB | _T('\x20')) == _T('e')) { // ������� 82.e-2, 44.2e+6, 555E6, .32e5 ������
			register Char chB = getChar();
			if (chB == _T('-') || chB == _T('+'))
				chB = getChar();
			for (; chB >= _T('0') && chB <= _T('9'); chB = getChar()) {}
			lastChar(chB);
#ifdef _DEBUG
			if (IsNormalChar(chB))
				throw chB;
#endif
		}
		else { // else if ((m_charB | _T('\x20')) == _T('x')) {
			// ����Ҫ���� 0x7F �� 123434; , ��Ϊ x �� oper ֱ����IsNormalChar()��������
			register Char chB = charB;
			for (; IsNormalChar(chB); chB = getChar()) {}
			lastChar(chB);
		}
		m_tokenB.more = TOKEN_COMMON;

		L_notNumber: ;
	}
	else if (IsQuote(charA)) { // ����
		const Char chQuote = charA;
		register Char chB = charB;
		for (; chB && chB != chQuote; chB = getChar()) {
			if (chB != _T('\\')) // ת���ַ�
				continue;
			chB = getChar();
			if (chB == _T('\r') || chB == _T('\n'))
				goto L_whenMultiLineString;
		}
		m_tokenB.more = TOKEN_STRING;
		goto L_finishReadString;

		L_whenMultiLineString:
		{
			for (; chB && chB != chQuote; ) { // ����״̬��ȫ�������ֱ�����Ž���
				if (chB == _T('\\')) { // ת���ַ�
					chB = getChar();
					if (chB == '\r')
						chB = getChar();
					if (chB != '\n')
						continue;
				}
				chB = getChar();
			}
			m_tokenB.more = TOKEN_STRING_MULTI_LINE;
		}

		L_finishReadString:
		chB = getChar();
		if (chB == '.')
			for (; chB = getChar(), IsNormalChar(chB); ) {}
		lastChar(chB);
	}
	else if (charA == _T('/') && (charB == _T('/') || charB == _T('*'))) { // ע��
		if (charB == _T('/')) { // ֱ������
			m_tokenB.more = (bFlag == BoolFlag::InNewLine) ? TOKEN_COMMENT_NEWLINE : TOKEN_COMMENT_LINE;
			register Char chB;
			for (; (chB = getChar()) && chB != _T('\r') && chB != _T('\n'); ) {}
			lastChar(chB);
		}
		else { // ֱ�� */
			m_tokenB.more = TOKEN_COMMENT_BLOCK;
			for (register Char chB; chB = getChar(); ) {
				L_continueJudgeStar:
				if (chB == '*') {
					chB = getChar();
					if (chB == '/') break;
					else goto L_continueJudgeStar;
				}
			}
			lastChar(getChar());
		}
	}
	else if (IsSingleOper(charA) || IsSingleOperNextChar(charB)) { // ���ַ�����
		m_tokenB.more = TOKEN_OPER;
		lastChar(charB);
	}
	else if (charB == _T('=') || charB == charA) { // ���ַ�����
		m_tokenB.more = TOKEN_OPER;
		register Char charC = getChar();
		if (charC == _T('=')) {
			if ( charB == _T('<') || charB == _T('>') || charA == _T('=') ||
				(charA == _T('!') && charB == _T('=')) )
			{ // ���ַ� ===, !==, <<=, >>=
				charC = getChar();
			}
		}
		else if (charC == _T('>') && charB == _T('>')) {
			charC = getChar();
			if (charC == _T('=')) // >>>=
				charC = getChar();
		}
		lastChar(charC);
	}
	// ����/mod: ECMAScript�Ĳ������б�����"->", ��������֮
	//else if ((charA == _T('-') && m_charB == _T('>'))) {
	//m_charB = getChar();
	//}
	else { // ���ǵ��ַ���
		m_tokenB.more = TOKEN_OPER;
		lastChar(charB);
	}
	
	checkEnd();
	m_tokenB.setData(const_cast<Char *>(start1 - 1));
	m_tokenB.setLength(curPos() - start1);
L_finishGetToken: ;
}

#define canHaveSpecialToken() (m_tokenA.more == TOKEN_ID\
	&& (m_tokenA.equals(_T("return"), 6) || \
	m_tokenA.equals(_T("case"), 4)) \
	)

void JSParser::prepareTokenB() const
{
	if (m_tokenB.more != TOKEN_OPER)
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
		if (m_tokenA.more >= TOKEN_COMMENT_LINE) {
		}
		else if (m_tokenA.more == TOKEN_OPER) {
			if (! ConstString::findIn(m_tokenA[m_tokenA.size() - 1], s_operCharBeforeReg))
				return;
		}
		else if (! canHaveSpecialToken())
			return;
		{
			register Char chB = lastChar();
			for (; ; chB = getChar()) { // ����״̬ȫ�������ֱ�� /
				if (!chB || chB == _T('/'))
					break;
				else if (chB == _T('\\')) // ת���ַ�
					getChar();
				else if (chB == _T('\r') || chB == _T('\n'))
					break;
				else if (chB == _T('[')) {
					while ((chB = getChar()) && chB != _T(']')) { // ����״̬ȫ�������ֱ�� /
						if (chB == _T('\\')) // ת���ַ�
							getChar();
						else if (chB == _T('\r') || chB == _T('\n'))
							break;
					}
					if (chB != _T(']'))
						break;
				}
			}
			if (chB != _T('/')) { // ����/warn: ����ƥ��ʧ�ܺ���˵Ĺ���
				readBack(m_tokenB.c_end());
				lastChar(getChar());
				break;
			}
			// ��ע/info: Ҳ����ֻʶ��Сд"g", "i", "m", ������ȷ�﷨�����"/a/in{}"��д��
			for (; chB = getChar(), chB >= _T('a') && chB <= _T('z'); ) {} // ����� flags ����
			if (chB == '.')
				for (; chB = getChar(), IsNormalChar(chB); ) {} // ����֮��Ĳ���
			lastChar(chB);

			m_tokenB.setLength(curPos() - m_tokenB - 1);
			m_tokenB.more = TOKEN_COMMON;
		}
		break;
	case _T('-'): case _T('+'):
		// ����/mod: ǿ�� m_tokenA Ϊ TOKEN_OPER ���� return
		/*
		* ��� m_tokenB �� -,+ ��
		* ���� m_tokenA �����ַ�����Ҳ����������ʽ
		* ���� m_tokenA ���� ++, --, ], )
		* ���� m_charB ��һ�� NormalChar
		* ��ô m_tokenB ʵ������һ��������
		*/
		if (m_tokenB.size() != 1 || !IsNormalChar(lastChar()))
			return;
		else if (m_tokenA.more == TOKEN_OPER) {
			if (m_tokenA == _T(']') || m_tokenA == _T(')') ||
				m_tokenA.equals(_T('+'), _T('+')) || m_tokenA.equals(_T('-'), _T('-')))
			{
				return;
			}
		}
		else if (! canHaveSpecialToken())
			return;
		getTokenRaw(BoolFlag::PosNeg);
		break;
	default:
		break;
	}
}
