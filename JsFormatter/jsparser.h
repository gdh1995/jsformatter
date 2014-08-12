/* jsparser.h
2012-3-11
Version: 0.9.9

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
#ifndef _JS_PARSER_H_
#define _JS_PARSER_H_
// #include <queue>
#include "ConstString.h"

/*
* if-i, else-e, else if-i,
* for-f, do-d, while-w,
* switch-s, case-c, default-c
* try-r, catch-h
* {-BLOCK, (-BRACKET
* 0-empty
*/

class JSParser
{
public:
	typedef char Byte;

#if defined UNICODE || defined _UNICODE
	typedef wchar_t Char;
	typedef unsigned short UChar;
	#define _T(x) (L##x)
#else
	typedef char Char;
	typedef unsigned char UChar;
	#define _T(x) (x)
#endif

public:
	typedef ConstString<Char> ConstString; // Token item
	// typedef std::queue<Token> TokenQueue;

	enum TOKEN_TYPE {
		REGULAR_TOKEN = 0,
		OPER_TOKEN = 1,
		STRING_TOKEN = 2,
		// Ϊ�����PrepareRegular()
		// Comment���token��Ҫ������������token
		COMMENT_1_TOKEN = 3, // ����ע��
		COMMENT_2_TOKEN = 4, // ����ע��
		// TODO: ����һ��ע����token��ʾ��β
		//CRLF_TOKEN = 11,
	};
	// .more: Token ����
	typedef ConstString Token;
	
public:
	explicit JSParser() {
		Init();
	}

	// bool m_debug;
	// inline const char *GetDebugOutput() { return m_debugOutput; }
	// void PrintDebug();
	
	inline void Init() {
		// m_debug = false;
	}

public:
	static const Byte s_normalCharMap[128];
	// һ���ַ�
	static inline bool IsNormalChar(Char ch) { return ((UChar) ch > 126u) || s_normalCharMap[ch]; }
	// ���ֺ�.
	static inline bool IsNumChar(Char ch) { return ((ch >= _T('0') && ch <= _T('9')) || ch == _T('.')); }
	// �հ��ַ�
	static inline bool IsBlankChar(Char ch) { return (ch == _T(' ') || ch == _T('\t') || ch == _T('\n') || ch == _T('\r')); }
	// ����
	static inline bool IsQuote(Char ch) { return (ch == _T('\'') || ch == _T('\"')); }

	static const Byte s_singleOperNextCharMap[128];
	// һ���ַ� + �հ��ַ� + ����
	static inline bool IsSingleOperNextChar(Char ch) { return ((UChar) ch > 126u) || s_singleOperNextCharMap[ch]; }

	static const Byte s_singleOperMap[128];
	// ���ַ�����
	static inline bool IsSingleOper(Char ch) { return ((UChar) ch <= 126u) && s_singleOperMap[ch]; }
	
	static const Char s_operCharBeforeReg[]; // �ж�����ʱ������ǰ����Գ��ֵ��ַ�
	
protected:
	// char m_debugOutput[1024];
	
	Token m_tokenA;
	mutable Token m_tokenB;

	inline bool GetToken() {
		PrepareTokenB();
		// Attention: raw__debug() is so ugly and slow that we have to give up it.
		//	m_tokenA.raw() = m_tokenB.raw();
		(Token::BaseString&)m_tokenA = (const Token::BaseString&)m_tokenB;
		GetTokenRaw();
		return m_tokenA.length() != 0;
	}

	inline void StartParse() const {
		// m_startClock = clock();
		// m_bRegular = false;
		m_bFlag = 0;
		m_charB = GetChar();
		GetTokenRaw();
	}

	inline void EndParse() const {
		// m_endClock = clock();
		// m_duration = (double)(m_endClock - m_startClock) / CLOCKS_PER_SEC;
		// PrintDebug();
	}

private:
	const Char *m_in;
	mutable const Char *m_in_cur;
	const Char *m_in_last;
	// ���ַ�ʽ�Ϻõ��չ���m_in == NULL����(int)len_in <= 0������
	inline Char GetChar() const {
		return (m_in_cur < m_in_last) ? (*++m_in_cur)
			: (m_in_cur = m_in_last + 1, 0);
	}
	inline const Char *CurPos() const { return m_in_cur; }

public:
	explicit JSParser(const Byte *in, size_t len_in)
		: m_in(in), m_in_cur(in - 1)
		, m_in_last(in + (len_in / sizeof(Char)) - 1)
	{
		Init();
	}

	inline void setInput(const Byte *in, size_t len_in = 0) {
		m_in = in;
		m_in_cur = in - 1;
		m_in_last = in + (len_in / sizeof(Char)) - 1;
	}
	
protected:
	mutable Char m_charB;
	mutable Byte m_bFlag;
	enum BoolFlag {
		PosNeg = 0x1, // tokenB ʵ����������
	};

private:
	void GetTokenRaw() const;
	// ͨ���ʷ��ж� tokenB ����������������ȵȲ����Դ���
	void PrepareTokenB() const;

private: // ��ֹ����
	JSParser(const JSParser&);
	JSParser(JSParser&&);
	JSParser& operator =(const JSParser&);
	JSParser& operator =(JSParser&&);
};

#endif
