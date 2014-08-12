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
#include "jsparser.h"

const char JSParser::s_strBeforeReg[] = "(,=:[!&|?+{};\n";

//	void JSParser::PrintDebug()
//	{
//		if(!m_debug)
//		{
//			return;
//		}
//		_snprintf(m_debugOutput, sizeof(m_debugOutput), "%s%d\n" // "%s%d%s%.3fs\n%.3f%s"
//			, "Processed tokens: ", m_tokenCount
//			// , "\nTime used: ", m_duration
//			// , m_tokenCount / m_duration, " tokens/second\n"
//			);
//		printf("%s", m_debugOutput);
//	}

//	static const char s_normalMap_0[128] = {
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

static const char s_normalCharMap[128] = {
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
bool JSParser::IsNormalChar(int ch)
{
	return ((unsigned int) ch > 126u) || s_normalCharMap[ch];
	// ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
	//	(ch >= 'A' && ch <= 'Z') || ch == '_' || ch == '$' ||
	//	((unsigned int) ch > 126u) );
}

static const char s_singleOperMap[128] = {
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
bool JSParser::IsSingleOper(int ch)
{
	return ((unsigned int) ch <= 126u) && s_singleOperMap[ch];
	// (ch == '.' || ch == '(' || ch == ')' ||
	//	ch == '[' || ch == ']' || ch == '{' || ch == '}' ||
	//	ch == ',' || ch == ';' || ch == '~' ||
	//	ch == '\n');
}

void JSParser::GetTokenRaw()
{
	//	if(!m_bGetTokenInit)
	//	{
	//		m_charB = GetChar();
	//	}

	// normal procedure
	if(!m_bRegular && !m_bPosNeg) {
		m_tokenB.expand(8);
		m_tokenB.setLength(0);
		m_tokenB.c_str()[0] = 0;

		m_tokenB.more = STRING_TYPE;
	}
	else if(m_bRegular) {
		//m_tokenB.push_back('/');
		m_tokenB.more = REGULAR_TYPE; // ����
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
	char chQuote; // ��¼�������� ' �� "
	char chComment; // ע������ / �� *

	for(; ; ) {
		if(m_charB == 0) {
			m_bRegular = false; // js content error
			return;
		}
		
		m_charA = m_charB;
		m_charB = GetChar();
		// \r\n -> \n(next char)
		if(m_charA == '\r') {
			m_charA = '\n';
			if (m_charB == '\n')
				m_charB = GetChar();
		}

		//	if(m_charA == '\n')
		//		++m_lineCount;

		/*
		* �ο� m_charB ������ m_charA
		* �������� m_charA
		* ��һ��ѭ��ʱ�Զ����� m_charB ���� m_charA
		*/

		// ������Ҫ�� token �����ж�
		if(m_bRegular) { // ����״̬ȫ�������ֱ�� /
			m_tokenB.addOrDouble(m_charA);

			if(m_charA == '\\') {
				if ((m_charB == '/' || m_charB == '\\')) { // ת���ַ�
					m_tokenB.addOrDouble(m_charB);
					m_charB = GetChar();
				}
			}
			else if(m_charA == '/' && m_charB != '*' && m_charB != '|') { // ������ܽ���
				if(!bRegularFlags && IsNormalChar(m_charB)) {
					bRegularFlags = true; // ����� flags ����
				}
				else {
					m_bRegular = false; // �������
					return;
				}
			}

			if(bRegularFlags && !IsNormalChar(m_charB)) {
				bRegularFlags = false;
				m_bRegular = false; // �������
				return;
			}

			continue;
		}
		else if(bQuote) { // ����״̬��ȫ�������ֱ�����Ž���
			m_tokenB.addOrDouble(m_charA);

			if(m_charA == '\\' && (m_charB == chQuote || m_charB == '\\')) { // ת���ַ�
				m_tokenB.addOrDouble(m_charB);
				m_charB = GetChar();
			}

			if(m_charA == chQuote) // ���Ž���
				return;

			continue;
		}
		else if(bComment) { // ע��״̬��ȫ�����
			if(m_tokenB.more == COMMENT_TYPE_2) {
				// /*...*/ÿ��ǰ���\t, ' '��Ҫɾ��
				if(bLineBegin) {
					if(m_charA == '\t' || m_charA == ' ')
						continue;
					else if (m_charA == '*')
						m_tokenB.addOrDouble(' ');
				}
				bLineBegin = (m_charA == '\n');
			}
			m_tokenB.addOrDouble(m_charA);

			if(chComment == '*') { // ֱ�� */
				m_tokenB.more = COMMENT_TYPE_2;
				if(m_charA == '*' && m_charB == '/') {
					m_tokenB.addOrDouble(m_charB);
					m_charB = GetChar();
					return;
				}
			}
			else { // ֱ������
				m_tokenB.more = COMMENT_TYPE_1;
				if(m_charA == '\n')
					return;
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
			if (bNumOld && !bNum && (m_charA == 'e' || m_charA == 'E')) {
				if (m_charB == '-' || m_charB == '+') {
					bNum = true;
					m_tokenB.addOrDouble(m_charB);
					m_charB = GetChar();
				}
				else if (IsNumChar(m_charB)) {
					bNum = true;
				}
			}

			if(!IsNormalChar(m_charB)) { // loop until m_charB is not normal char
				m_bPosNeg = false;
				return;
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
			register char *s1 = m_tokenB.c_str();
			s1[0] = m_charA; // ���ַ�����
			s1[1] = 0;
			return;
		}
		else if((m_charB == '=' || m_charB == m_charA) ||
			(m_charA == '-' && m_charB == '>')) // ���ַ�����
		{
			// ��ȷ�Ƕ��ַ�����
			m_tokenB.more = OPER_TYPE;
			m_tokenB.addOrDouble(m_charA);
			m_tokenB.addOrDouble(m_charB);
			m_charB = GetChar();
			if (m_charB == '=' && (m_tokenB == "==" || m_tokenB == "!=" ||
				m_tokenB == "<<" || m_tokenB == ">>"))
			{
				// ���ַ� ===, !==, <<=, >>=
				m_tokenB.addOrDouble(m_charB);
				m_charB = GetChar();
			}
			else if (m_charB == '>' && m_tokenB == ">>")
			{
				// >>>, >>>=
				m_tokenB.addOrDouble(m_charB);
				m_charB = GetChar();
				if(m_charB == '=') // >>>=
				{
					m_tokenB.addOrDouble(m_charB);
					m_charB = GetChar();
				}
			}
			return;
		}
		else {
			// ���ǵ��ַ���
			m_tokenB.more = OPER_TYPE;
			m_tokenB.setLength(1);
			register char *s1 = m_tokenB.c_str();
			s1[0] = m_charA; // ���ַ�����
			s1[1] = 0;
			return;
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

	return m_tokenA.length();
}

void JSParser::PrepareRegular()
{
	/*
	* �ȴ���һ������
	* m_tokenB[0] == /���� m_tokenB ����ע��
	* m_tokenA ���� STRING (���� m_tokenA == return)
	* ���� m_tokenA �����һ���ַ���������Щ
	*/
	char tokenALast = m_tokenA.size() > 0 ? m_tokenA[m_tokenA.size() - 1] : 0;
	char tokenBFirst = m_tokenB[0];
	if (tokenBFirst == '/' &&
		m_tokenB.more != COMMENT_TYPE_1 && m_tokenB.more != COMMENT_TYPE_2 &&
		( (m_tokenA.more != STRING_TYPE && strchr(s_strBeforeReg, tokenALast)) ||
		m_tokenA == "return") )
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
	if(m_tokenB.more == OPER_TYPE && (m_tokenB == '-' || m_tokenB == '+') &&
		(m_tokenA.more != STRING_TYPE || m_tokenA == "return") &&
		m_tokenA.more != REGULAR_TYPE && m_tokenA != "++" && m_tokenA != "--" &&
		(m_tokenA != ']' && m_tokenA != ')') &&
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
	* ���� else, while, catch, ',', ';', ')', { ֮ǰ�Ļ���
	* ����������Ĳ��������Ǽ������ٰ�ȥ���Ļ��в���
	*/
	int c = 0;
	while(m_tokenB == '\n' || (m_tokenB == '\r' && m_tokenB[2] == '\n')) {
		++c;
		GetTokenRaw();
	}

	if (m_tokenB != "else" && m_tokenB != "while" && m_tokenB != "catch" &&
		(m_tokenB != ',' && m_tokenB != ';' && m_tokenB != ')'))
	{
		// ��ȥ���Ļ���ѹ����У��ȴ���
		if(m_tokenA == '{' && m_tokenB == '}')
			return; // �� {}

		for(c = c > 2 ? 2 : c; c > 0; --c) {
			m_tokenBQueue.push(CharString<char>("\n", 1, 2, 1, OPER_TYPE));
		}
		m_tokenBQueue.push(m_tokenB);
		m_tokenB = m_tokenBQueue.front();
		m_tokenBQueue.pop();
	}
}
