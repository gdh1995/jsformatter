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
	typedef unsigned char UByte;

#if defined UNICODE || defined _UNICODE
	typedef wchar_t Char;
	typedef unsigned short UChar;
	#define _T(x) (L##x)
#else
	typedef char Char;
	typedef unsigned char UChar;
	#define _T(x) (x)
#endif

	typedef ConstString<Char> ConstString; // Token item
	// typedef std::queue<Token> TokenQueue;
	
	// �Ͻ��޸�TOKEN_xxx�ı��˳��, ���ǵ�cpp�������ȫ����
	enum TOKEN_TYPE {
		TOKEN_REGULAR = 0,
		TOKEN_OPER = 1,

		TOKEN_ID = 2,
		TOKEN_COMMON = 2,
		TOKEN_STRING = 3,
		TOKEN_STRING_MULTI_LINE = 3,

		// Ϊ�����PrepareTokenB()
		// Comment���token��Ҫ������������token
		TOKEN_COMMENT_LINE = 4, // ����ע��
		TOKEN_COMMENT_BLOCK = 5, // ����ע��
		TOKEN_BLANK_LINE = 6, // ���б��
		TOKEN_NULL = 7,
	};
	// .more: Token ����
	typedef ConstString Token;

	// һ���ַ� (�������ֺ� . ) ��boolӳ��
	static const  Byte s_normalCharMap[128];
	
	// һ���ַ� (�������ֺ� . )
	static inline bool IsNormalChar(Char ch) { return ((UChar) ch > 126u) || s_normalCharMap[ch]; }
	
	// ���ֺ�. (unused)
	static inline bool IsNumChar(Char ch) { return ((ch >= _T('0') && ch <= _T('9')) || ch == _T('.')); }
	
	// �հ��ַ�
	static inline bool IsBlankChar(Char ch) { return (ch == _T(' ') || ch == _T('\t') || ch == _T('\n') || ch == _T('\r')); }
	
	// ����
	static inline bool IsQuote(Char ch) { return (ch == _T('\'') || ch == _T('\"')); }
	
	// ���ַ�������֮����ܸ����ַ���boolӳ��
	static const  Byte s_singleOperNextCharMap[128];

	// һ���ַ� + �հ��ַ� + ����
	static inline bool IsSingleOperNextChar(Char ch) { return ((UChar) ch > 126u) || s_singleOperNextCharMap[ch]; }

	// ���ַ���������boolӳ��
	static const  Byte s_singleOperMap[128];
	
	// ���ַ�����
	static inline bool IsSingleOper(Char ch) { return ((UChar) ch <= 126u) && s_singleOperMap[ch]; }
	
	// �ж�����ʱ������ǰ����Գ��ֵ��ַ�
	static const  Char s_operCharBeforeReg[];
	
public:
	explicit JSParser() {
		// m_debug = false;
		Init();
	}

	explicit JSParser(const Byte *in, size_t len_in)
		: m_in(in), m_in_next(in)
		, m_in_end(in + len_in)
	{
		Init();
	}
	
	inline void Init() const {
	}

	inline void setInput(const Byte *in, size_t len_in = 0) {
		m_in = in;
		m_in_next = in;
		m_in_end = in + len_in;
	}

	// bool m_debug;
	// inline const char *GetDebugOutput() const { return m_debugOutput; }
	// void PrintDebug() const;
	
public:
	// char m_debugOutput[1024];
	
	mutable Token m_tokenA;
	mutable Token m_tokenB;

	inline bool GetToken() const {
		PrepareTokenB();
		// Attention: In the debug mode, raw() is so ugly and slow
		//     that we have to give up it.
		//	m_tokenA.raw() = m_tokenB.raw();
		(Token::BaseString&)m_tokenA = (const Token::BaseString&)m_tokenB;
		GetTokenRaw();
		return m_tokenA.length() != 0;
	}

	inline void StartParse() const {
		// m_startClock = clock();
		// m_bRegular = false;
		GetChar();
		GetTokenRaw();
	}

	inline void EndParse() const {
		// m_endClock = clock();
		// m_duration = (double)(m_endClock - m_startClock) / CLOCKS_PER_SEC;
		// PrintDebug();
	}

protected:
	enum BoolFlag {
		PosNeg = 0x1, // tokenB ʵ����������
	};

	inline Char GetChar() const {
		return (m_in_next < m_in_end) ? (*m_in_next++) : 0;
	}
	inline const Char *CurPos() const { return m_in_next; }
	
	// һ��Ҫ��StartParse()����ܵ���
	inline const Char LastChar() const {
		return (m_in_next < m_in_end) ? (m_in_next[-1]) : 0;
	}

	// ��ȡ��һ��token���洢��m_tokenB
	void GetTokenRaw(int bFlag = 0) const;
	
	// ͨ���ʷ��ж� m_tokenB �Ƿ������������������ȵȲ����Դ���
	void PrepareTokenB() const;

private:
	mutable const Char *m_in_next;
	const Char *m_in;
	const Char *m_in_end;

private: // ��ֹ����
	JSParser(const JSParser&);
	JSParser(JSParser&&);
	JSParser& operator =(const JSParser&);
	JSParser& operator =(JSParser&&);
};

#endif
