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

const JSParser::Byte JSParser::s_singleOperMap[128] = {
	/*  + x: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
	/* 0+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 1+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 2+ */ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0,
	/* 3+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
	/* 4+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 5+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0,
	/* 6+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 7+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0
};

void JSParser::GetTokenRaw() const
{
	m_tokenB.more = OPER_TOKEN;
	const Char * start0;

	if (m_bFlag & BoolFlag::PosNeg) {
		start0 = CurPos() - 1;
	}
	else {
		register Char chB = m_charB;
		while (IsBlankChar(chB)) {
			chB = GetChar();
		}
		m_charB = chB;
		start0 = CurPos();
	}
	const Char charA = m_charB;
	m_charB = GetChar();
	if (IsNormalChar(charA)) {
		// ����/mod: ��_T('.')��Ϊ���ʵ�һ����,
		// ���Ҫ�Ļ�, ֻ���޸�s_normalCharMap[0x2E] = 0;
		m_tokenB.more = STRING_TOKEN;
		register Char chB = m_charB;
		if (IsNumChar(charA)) {
			while (IsNumChar(chB)) {
				chB = GetChar(); // loop until m_charB is not a normal character
			}
			if ((chB | _T('\x20')) == _T('e')) {
				chB = GetChar();
				// ������� 82e-2, 442e+6, 555E-6 ������
				if (chB == _T('-') || chB == _T('+')) {
					chB = GetChar();
				}
			}
		}
		while (IsNormalChar(chB)) {
			chB = GetChar(); // loop until m_charB is not a normal character
		}
		m_charB = chB;
	}
	else if (IsQuote(charA)) { // ����
		m_tokenB.more = STRING_TOKEN;
		const Char chQuote = charA;
		register Char chB = m_charB;
		for (; chB; ) { // ����״̬��ȫ�������ֱ�����Ž���
			if (chB == chQuote) {
				chB = GetChar();
				break;
			}
			if (chB == _T('\\')) { // ת���ַ�
				chB = GetChar();
				if (chB == '\r') {
					chB = GetChar();
				}
				if (chB != '\n') {
					continue;
				}
			}
			chB = GetChar();
		}
		m_charB = chB;
	}
	else if (charA == _T('/') && (m_charB == _T('/') || m_charB == _T('*'))) { // ע��
		if (m_charB == _T('/')) { // ֱ������
			m_tokenB.more = COMMENT_1_TOKEN;
			size_t len0 = 2;
			register Char chB;
			for (; chB = GetChar(); ) {
				if (chB == '\n') {
					len0 = CurPos() - start0;
					chB = GetChar();
					break;
				}
				else if (chB == _T('\r')) {
					len0 = CurPos() - start0;
					chB = GetChar();
					if (chB == _T('\n')) {
						chB = GetChar();
					}
					break;
				}
			}
			m_charB = chB;
			m_tokenB.setData(const_cast<Char *>(start0));
			m_tokenB.setLength(len0);
			// m_tokenB.setFlag1();
			return;
		}
		else { // ֱ�� */
			m_tokenB.more = COMMENT_2_TOKEN;
			//len0 = 2;
			//{
			register Char chB = GetChar();
			for (; chB; ) {
				//if (chB == '\n') {
				//	len0 = CurPos() - start0 - 1;
				//	chB = GetChar();
				//	break;
				//}
				//else if (chB == _T('\r')) {
				//	len0 = CurPos() - start0 - 1;
				//	chB = GetChar();
				//	if (chB == _T('\n')) { chB = GetChar(); }
				//	break;
				//}
				register Char chA = chB;
				chB = GetChar();
				if (chA == '*') {
					if (chB == '/') {
						chB = GetChar();
						// len0 = 0;
						break;
					}
				}
			}
			m_charB = chB;
			//}
			//if (len0) {
			//m_tokenB.reset(len0 + 4);
			//m_tokenB.copyFrom(start0, len0);
			//m_tokenB.addOrDouble(_T('\n'));
			//bool bLineBegin = true;
			//register Char chB = m_charB;
			//for (; chB; ) {
			//	if (bLineBegin) {
			//		if (chB == _T('\t') || chB == _T(' ')) {
			//			chB = GetChar();
			//			continue; // /*...*/ÿ��ǰ���_T('\t'), _T(' ')��Ҫɾ��
			//		}
			//		bLineBegin = false;
			//		if (chB == _T('*')) {
			//			m_tokenB.addOrDouble(_T(' '));
			//		}
			//	}
			//	if (chB == _T('\r')) {
			//		bLineBegin = true;
			//		m_tokenB.addOrDouble(_T('\n'));
			//		chB = GetChar();
			//		if (chB == _T('\n'))
			//			chB = GetChar();
			//		continue;
			//	} else if (chB == _T('\n')) {
			//		bLineBegin = true;
			//	}
			//	m_tokenB.addOrDouble(chB);
			//	if (chB == _T('*')) {
			//		chB = GetChar();
			//		if (chB == _T('/')) {
			//			m_tokenB.addOrDouble(chB);
			//			chB = GetChar();
			//			break;
			//		}
			//		continue;
			//	}
			//	chB = GetChar();
			//}
			//m_charB = chB;
			//return;
			//}
		}
	}
	else if (IsSingleOper(charA) || IsSingleOperNextChar(m_charB)) // ���ַ�����
	{
	}
	else if (m_charB == _T('=') || m_charB == charA) // ���ַ�����
	{
		register Char charB = m_charB;
		m_charB = GetChar();
		if (m_charB == _T('=')) {
			if (( charB == _T('<') && charA == _T('<') ) ||
				( charB == _T('>') && charA == _T('>') ) ||
				( charB == _T('=') && (charA == _T('=') || charA == _T('!')) ))
			{ // ���ַ� ===, !==, <<=, >>=
				m_charB = GetChar();
			}
		}
		else if (m_charB == _T('>') && charB == _T('>') && charA == _T('>')) {
			m_charB = GetChar();
			if (m_charB == _T('=')) { // >>>=
				m_charB = GetChar();
			} else { // >>>
			}
		}
	}
	// ����/mod: ECMAScript�Ĳ������б�����"->", ��������֮
	//else if ((charA == _T('-') && m_charB == _T('>'))) {
	//m_charB = GetChar();
	//}
	else { // ���ǵ��ַ���
	}

	m_tokenB.setData(const_cast<Char *>(start0));
	m_tokenB.setLength(CurPos() - start0);
	// m_tokenB.setFlag1();
}

void JSParser::PrepareTokenB() const
{
	if (m_tokenB.more != OPER_TOKEN) {
		return;
	}
	switch (m_tokenB[0]) {
	case _T('/'):
		// ����/mod: �ſ�ע�͵�����
		// ����/warn: Ĭ��m_tokenA.more == OPER_TOKENʱ, m_tokenA��Ϊ��
		// >>> ��Ҫm_tokenA������ֵ������Token���� + OPER_TOKEN����.more��Ĭ��ֵ
		/*
		* �ȴ���һ������
		* m_tokenB[0] == /���� m_tokenB ����ע��
		* m_tokenA ���� STRING (���� m_tokenA == return)
		* ���� m_tokenA �����һ���ַ���������Щ
		*/
		if (m_tokenA.more >= COMMENT_1_TOKEN || (m_tokenA.more == OPER_TOKEN &&
			ConstString::findIn(m_tokenA[m_tokenA.size() - 1], s_operCharBeforeReg)) ||
			m_tokenA.equals(_T("return"), 6) ) 
		{
			register Char chB = m_charB;
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
			m_charB = chB;
			m_tokenB.setLength(CurPos() - m_tokenB.c_str());
			m_tokenB.setFlag1();
			m_tokenB.more = REGULAR_TOKEN;
		}
		break;
	case _T('-'): case _T('+'):
		if (m_tokenB.size() != 1) {
			return;
		}
		// ����/mod: ǿ�� m_tokenA Ϊ OPER_TOKEN ���� return
		/*
		* ��� m_tokenB �� -,+ ��
		* ���� m_tokenA �����ַ�����Ҳ����������ʽ
		* ���� m_tokenA ���� ++, --, ], )
		* ���� m_charB ��һ�� NormalChar
		* ��ô m_tokenB ʵ������һ��������
		*/
		if ( IsNormalChar(m_charB) && ( m_tokenA.equals(_T("return"), 6) || 
			(m_tokenA.more == OPER_TOKEN && m_tokenA != _T(']') && m_tokenA != _T(')') &&
			m_tokenA.nequals(_T('+'), _T('+')) && m_tokenA.nequals(_T('-'), _T('-'))) ) )
		{
			// m_tokenB ʵ������������
			m_bFlag |= BoolFlag::PosNeg;
			GetTokenRaw();
			m_bFlag &= ~(BoolFlag::PosNeg);
		}
		break;
	default:
		break;
	}
}
