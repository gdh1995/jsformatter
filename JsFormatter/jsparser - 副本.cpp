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

const JSParser::Char JSParser::s_strBeforeReg[] = _T("(,=:[!&|?+{};\n");

//	void JSParser::PrintDebug()
//	{
//		if(!m_debug)
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

static const JSParser::Byte s_normalCharMap[128] = {
	/*  + x: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
	/* 0+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 1+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 2+ */ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 3+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	/* 4+ */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 5+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
	/* 6+ */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 7+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1
};
bool JSParser::IsNormalChar(Char ch)
{
	return ((UChar) ch > 126u) || s_normalCharMap[ch];
	// ((ch >= _T('a') && ch <= _T('z')) || (ch >= _T('0') && ch <= _T('9')) ||
	//	(ch >= _T('A') && ch <= _T('Z')) || ch == _T('_') || ch == _T('$') ||
	//	((unsigned int) ch > 126u) );
}

static const JSParser::Byte s_singleOperMap[128] = {
	/*  + x: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
	/* 0+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	/* 1+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 2+ */ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0,
	/* 3+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
	/* 4+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 5+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0,
	/* 6+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 7+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0
};
bool JSParser::IsSingleOper(Char ch)
{
	return ((UChar) ch <= 126u) && s_singleOperMap[ch];
	// (ch == _T('.') || ch == _T('(') || ch == _T(')') ||
	//	ch == _T('[') || ch == _T(']') || ch == _T('{') || ch == _T('}') ||
	//	ch == _T(',') || ch == _T(';') || ch == _T('~') ||
	//	ch == _T('\n'));
}

void JSParser::GetTokenRaw()
{
	//	if(!m_bGetTokenInit)
	//	{
	//		m_charB = GetChar();
	//	}

	// normal procedure
	if (m_tokenB.length() > 0) {
		m_tokenB.reset(0);
	}
	if(!m_bRegular && !m_bPosNeg) {
		m_tokenB.expand(8);
		m_tokenB.setLength(0);
		m_tokenB.c_str()[0] = 0;

		m_tokenB.more = STRING_TYPE;
	}
	else if(m_bRegular) {
	}
	else {
		m_tokenB.more = STRING_TYPE; // ������
	}

	bool bQuote = false;
	bool bComment = false;
	bool bRegularFlags = false;
	bool bFirst = true;
	bool bNum = false; // �ǲ�������
	bool bLineBegin = false;
	Char chQuote; // ��¼�������� ' �� "
	Char chComment; // ע������ / �� *
	
	// ������Ҫ�� token �����ж�
	if(m_bRegular) { // ����״̬ȫ�������ֱ�� /
		size_t b_len = 1;
		for(; m_charB; ) {
			m_charA = m_charB;
			b_len++;
			m_charB = GetChar();

			if(m_charA == _T('\\')) {
				if ((m_charB == _T('/') || m_charB == _T('\\'))) { // ת���ַ�
					b_len++;
					m_charB = GetChar();
				}
			}
			else if(m_charA == _T('/') && m_charB != _T('*') && m_charB != _T('|')) { // ������ܽ���
				if(!bRegularFlags && IsNormalChar(m_charB)) {
					bRegularFlags = true; // ����� flags ����
				}
				else {
					break;
				}
			}

			if(bRegularFlags && !IsNormalChar(m_charB)) {
				bRegularFlags = false;
				break;
			}
		}
		m_tokenB.setLength(b_len);
		m_tokenB.more = REGULAR_TYPE;
		m_bRegular = false;
		return;
	}
		
	for(; ; ) {
		if(m_charB == 0) {
			m_bRegular = false; // js content error
			break;
		}

		m_charA = m_charB;
		m_charB = GetChar();
		// \r\n -> \n (next character)
		if(m_charA == _T('\r')) {
			m_charA = _T('\n');
			if (m_charB == _T('\n'))
				m_charB = GetChar();
		}
		//	if(m_charA == _T('\n'))
		//		++m_lineCount;

		/*
		* �ο� m_charB ������ m_charA
		* �������� m_charA
		* ��һ��ѭ��ʱ�Զ����� m_charB ���� m_charA
		*/
		if(bQuote) { // ����״̬��ȫ�������ֱ�����Ž���
			m_tokenB.addOrDouble(m_charA);

			if(m_charA == _T('\\') && (m_charB == chQuote || m_charB == _T('\\'))) { // ת���ַ�
				m_tokenB.addOrDouble(m_charB);
				m_charB = GetChar();
			}

			if(m_charA == chQuote) // ���Ž���
				break;

			continue;
		}
		else if(bComment) { // ע��״̬��ȫ�����
			if(m_tokenB.more == COMMENT_TYPE_2) {
				// /*...*/ÿ��ǰ���\t, _T(' ')��Ҫɾ��
				if(bLineBegin) {
					if(m_charA == _T('\t') || m_charA == _T(' '))
						continue;
					else if (m_charA == _T('*'))
						m_tokenB.addOrDouble(_T(' '));
				}
				bLineBegin = (m_charA == _T('\n'));
			}
			m_tokenB.addOrDouble(m_charA);

			if(chComment == _T('*')) { // ֱ�� */
				m_tokenB.more = COMMENT_TYPE_2;
				if(m_charA == _T('*') && m_charB == _T('/')) {
					m_tokenB.addOrDouble(m_charB);
					m_charB = GetChar();
					break;
				}
			}
			else { // ֱ������
				m_tokenB.more = COMMENT_TYPE_1;
				if(m_charA == _T('\n'))
					break;
			}

			continue;
		}

		if (IsNormalChar(m_charA)) {
			m_tokenB.more = STRING_TYPE;
			m_tokenB.addOrDouble(m_charA);

			// ������� 82e-2, 442e+6, 555E-6 ������
			// ��Ϊ����������������ֻ�ܷ������µı����ʽ
			bool bNumOld = bNum;
			if (bFirst || bNumOld) { // ֻ��֮ǰ�����ֲŸı�״̬
				bNum = IsNumChar(m_charA);
				bFirst = false;
			}
			if (bNumOld && !bNum && (m_charA == _T('e') || m_charA == _T('E'))) {
				if (m_charB == _T('-') || m_charB == _T('+')) {
					bNum = true;
					m_tokenB.addOrDouble(m_charB);
					m_charB = GetChar();
				}
				else if (IsNumChar(m_charB)) {
					bNum = true;
				}
			}

			if(!IsNormalChar(m_charB)) { // loop until m_charB is not a normal character
				m_bPosNeg = false;
				break;
			}
		}
		else if(IsBlankChar(m_charA))
			continue; // ���Կհ��ַ�

		else if(IsQuote(m_charA)) {
			// ����
			bQuote= true;
			chQuote = m_charA;

			m_tokenB.more = STRING_TYPE;
			m_tokenB.addOrDouble(m_charA);
			continue;
		}
		else if(IsComment()) {
			// ע��
			bComment = true;
			chComment = m_charB;

			m_tokenB.addOrDouble(m_charA);
			continue;
		}
		else if(IsSingleOper(m_charA) || IsNormalChar(m_charB) ||
			IsBlankChar(m_charB) || IsQuote(m_charB))
		{
			m_tokenB.more = OPER_TYPE;
			m_tokenB.setLength(1);
			register Char *s1 = m_tokenB.c_str();
			s1[0] = m_charA; // ���ַ�����
			s1[1] = 0;
			break;
		}
		else if((m_charB == _T('=') || m_charB == m_charA) ||
			(m_charA == _T('-') && m_charB == _T('>'))) // ���ַ�����
		{
			// ��ȷ�Ƕ��ַ�����
			m_tokenB.more = OPER_TYPE;
			m_tokenB.addOrDouble(m_charA);
			m_tokenB.addOrDouble(m_charB);
			m_charB = GetChar();
			if (m_charB == _T('=') && (m_tokenB == _T("==") || m_tokenB == _T("!=") ||
				m_tokenB == _T("<<") || m_tokenB == _T(">>")))
			{
				// ���ַ� ===, !==, <<=, >>=
				m_tokenB.addOrDouble(m_charB);
				m_charB = GetChar();
			}
			else if (m_charB == _T('>') && m_tokenB == _T(">>"))
			{
				// >>>, >>>=
				m_tokenB.addOrDouble(m_charB);
				m_charB = GetChar();
				if(m_charB == _T('=')) // >>>=
				{
					m_tokenB.addOrDouble(m_charB);
					m_charB = GetChar();
				}
			}
			break;
		}
		else {
			// ���ǵ��ַ���
			m_tokenB.more = OPER_TYPE;
			m_tokenB.setLength(1);
			register Char *s1 = m_tokenB.c_str();
			s1[0] = m_charA; // ���ַ�����
			s1[1] = 0;
			break;
		}
	}
}

bool JSParser::GetToken()
{
	//	if(!m_bGetTokenInit)
	//	{
	//		// ��һ�ζ����һ�� GetTokenRaw
	//		GetTokenRaw();
	//		m_bGetTokenInit = true;
	//	}

	PrepareRegular(); // �ж�����
	PreparePosNeg(); // �ж�������

	// ++m_tokenCount;
	m_tokenA = m_tokenB;

	if(m_tokenBQueue.size() == 0)
	{
		GetTokenRaw();
		PrepareTokenB(); // �����ǲ���Ҫ��������
	}
	else
	{
		// ���ŶӵĻ���
		m_tokenB = m_tokenBQueue.front();
		m_tokenBQueue.pop();
	}

	return m_tokenA.length() != 0;
}

void JSParser::PrepareRegular()
{
	/*
	* �ȴ���һ������
	* m_tokenB[0] == /���� m_tokenB ����ע��
	* m_tokenA ���� STRING (���� m_tokenA == return)
	* ���� m_tokenA �����һ���ַ���������Щ
	*/
	Char tokenALast = m_tokenA.size() > 0 ? m_tokenA[m_tokenA.size() - 1] : 0;
	Char tokenBFirst = m_tokenB[0];
	if (tokenBFirst == _T('/') && m_tokenB.more != COMMENT_TYPE_1 &&
		m_tokenB.more != COMMENT_TYPE_2 && ( m_tokenA == _T("return") ||
		(m_tokenA.more != STRING_TYPE && CharString::findIn(tokenALast, s_strBeforeReg))
		) )
	{
		m_bRegular = true;
		GetTokenRaw(); // ���������ݼӵ� m_tokenB
	}
}

void JSParser::PreparePosNeg()
{
	/*
	* ��� m_tokenB �� -,+ ��
	* ���� m_tokenA �����ַ�����Ҳ����������ʽ
	* ���� m_tokenA ���� ++, --, ], )
	* ���� m_charB ��һ�� NormalChar
	* ��ô m_tokenB ʵ������һ��������
	*/
	if(m_tokenB.more == OPER_TYPE && (m_tokenB == _T('-') || m_tokenB == _T('+')) &&
		(m_tokenA.more != STRING_TYPE || m_tokenA == _T("return")) &&
		m_tokenA.more != REGULAR_TYPE && m_tokenA != _T("++") && m_tokenA != _T("--") &&
		(m_tokenA != _T(']') && m_tokenA != _T(')')) &&
		IsNormalChar(m_charB))
	{
		// m_tokenB ʵ������������
		m_bPosNeg = true;
		GetTokenRaw();
	}
}

void JSParser::PrepareTokenB()
{
	/*
	* ���� else, while, catch, ',', ';', ')', '{' ֮ǰ�Ļ���
	* ����������Ĳ��������Ǽ������ٰ�ȥ���Ļ��в���
	*/
	int c = 0;
	while(m_tokenB == _T('\n') || m_tokenB.equals(_T('\r'), _T('\n'))) {
		++c;
		GetTokenRaw();
	}

	if ( (m_tokenB != _T(',') && m_tokenB != _T(')') && m_tokenB != _T(';')) &&
		m_tokenB != _T("else") && m_tokenB != _T("while") && m_tokenB != _T("catch") )
	{
		// ��ȥ���Ļ���ѹ����У��ȴ��� ��{}
		if(m_tokenA == _T('{') && m_tokenB == _T('}'))
			return;

		for(c = c > 2 ? 2 : c; c > 0; --c) {
			m_tokenBQueue.push(CharString(_T("\n"), 1, 2, 1, OPER_TYPE));
		}
		m_tokenBQueue.push(m_tokenB);
		m_tokenB = m_tokenBQueue.front();
		m_tokenBQueue.pop();
	}
}
