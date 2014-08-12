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
#include <stack>
#include <queue>
#include "CharString.h"

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
	enum JS_TOKEN_TYPE {
		JS_IF		= _T('i'),
		JS_ELSE		= _T('e'),
		JS_FOR		= _T('f'),
		JS_DO		= _T('d'),
		JS_WHILE	= _T('w'),
		JS_SWITCH	= _T('s'),
		JS_CASE		= _T('c'),
		JS_TRY		= _T('t'),
		JS_CATCH	= _T('h'),
		JS_FUNCTION	= _T('n'),
		JS_ASSIGN	= _T('='),
		JS_BLOCK	= _T('{'),
		JS_BRACKET	= _T('('),
		JS_SQUARE	= _T('['),
		JS_HELPER	= _T('\\'),
		JS_EMPTY	= 0
	};

	enum TOKEN_TYPE {
		TYPE_STRING = 0,
		TYPE_OPER = 1,
		TYPE_REGULAR = 2,
		TYPE_COMMENT_1 = 9, // ����ע��
		TYPE_COMMENT_2 = 10, // ����ע��
	};

	// .type: Token ����
	typedef CharString<Char> CharString; // Token item
protected:
	typedef CharString Token;
	
public:
	typedef std::stack<Char> CharStack;
	typedef std::stack<bool> BoolStack;
	typedef std::queue<Token> TokenQueue;

	explicit JSParser() {
		Init();
	}

	//virtual ~JSParser()
	//{}

	// bool m_debug;
	// inline const char *GetDebugOutput() { return m_debugOutput; }
	
	inline void Init() {
		// m_debug = false;

		// m_lineCount = 1; // �кŴ� 1 ��ʼ
		// m_tokenCount = 0;


		// m_bGetTokenInit = false;
	}

protected:
	// char m_debugOutput[1024];

	Char m_charA;
	Char m_charB;
	Token m_tokenA;
	Token m_tokenB;
	// int m_lineCount;
	// int m_tokenCount;
	
	static const Byte s_normalCharMap[128];
	// һ���ַ�
	static inline bool IsNormalChar(Char ch) { return ((UChar) ch > 126u) || s_normalCharMap[ch]; }
	// ���ֺ�.
	static inline bool IsNumChar(Char ch) { return ((ch >= _T('0') && ch <= _T('9')) || ch == _T('.')); }
	// �հ��ַ�
	static inline bool IsBlankChar(Char ch) { return (ch == _T(' ') || ch == _T('\t') || ch == _T('\r')); }

	static const Byte s_singleOperMap[128];
	// ���ַ�����
	static inline bool IsSingleOper(Char ch) { return ((UChar) ch <= 126u) && s_singleOperMap[ch]; }
	// ����
	static inline bool IsQuote(Char ch) { return (ch == _T('\'') || ch == _T('\"')); }

	bool GetToken(); // ���������, ����ȵȵ� GetToken ����

	inline void StartParse() {
		// m_startClock = clock();
		m_bRegular = false;
		m_bPosNeg = false;
		m_charB = GetChar();
		GetTokenRaw();
	}

	inline void EndParse() {
		// m_endClock = clock();
		// m_duration = (double)(m_endClock - m_startClock) / CLOCKS_PER_SEC;
		// PrintDebug();
	}

protected:
	const Byte *m_in;
	size_t m_len_in;
	mutable size_t m_getPos;
	inline Char GetChar() const {
		return (m_getPos < m_len_in) ? (((const Char*)m_in)[m_getPos++]) : 0;
	}

	inline const Char *CurPos() const {
		return (Char *)(m_in + m_getPos);
	}

	explicit JSParser(const Byte *in, size_t len_in)
		: m_in(in), m_len_in(len_in / sizeof(Char)), m_getPos(0)
	{
		Init();
	}

	void setInput(const Byte *in, size_t len_in = 0) {
		m_in = in;
		m_getPos = 0;
		m_len_in = len_in / sizeof(Char);
	}

private:
	// Should be implemented in derived class
	//virtual int GetChar() = 0; // JUST get next char from input
	
	// ע��
	inline bool IsComment() const { return (m_charA == _T('/') && (m_charB == _T('/') || m_charB == _T('*'))); } // Ҫ�����ж� charA, charB

	void GetTokenRaw();

	void PrepareRegular(); // ͨ���ʷ��ж� tokenB ����
	void PreparePosNeg(); // ͨ���ʷ��ж� tokenB ������
	void PrepareTokenB();

	// void PrintDebug();

	static const Char s_strBeforeReg[]; // �ж�����ʱ������ǰ����Գ��ֵ��ַ�

	bool m_bRegular; // tokenB ʵ�������� GetToken �õ���������Ա״̬
	bool m_bPosNeg; // tokenB ʵ����������
	TokenQueue m_tokenBQueue;

	// bool m_bGetTokenInit; // �Ƿ��ǵ�һ��ִ�� GetToken

	// double m_duration;

private:
	// ��ֹ����
	JSParser(const JSParser&);
	JSParser& operator =(const JSParser&);
};

#endif
