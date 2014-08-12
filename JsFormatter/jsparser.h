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
		STRING_TOKEN = 0,
		OPER_TOKEN = 1,
		REGULAR_TOKEN = 2,
		// 为了配合PrepareRegular()
		// Comment类的token需要大于任意其它token
		COMMENT_1_TOKEN = 9, // 单行注释
		COMMENT_2_TOKEN = 10, // 多行注释
	};

	// .type: Token 类型
	typedef CharString<Char> CharString; // Token item
protected:
	typedef CharString Token;
	
public:
	typedef std::stack<Char> CharStack;
	typedef std::stack<bool> BoolStack;
	//typedef std::queue<Token> TokenQueue;

	explicit JSParser() {
		Init();
	}

	//virtual ~JSParser()
	//{}

	// bool m_debug;
	// inline const char *GetDebugOutput() { return m_debugOutput; }
	// void PrintDebug();
	
	inline void Init() {
		// m_debug = false;

		// m_lineCount = 1; // 行号从 1 开始
		// m_tokenCount = 0;


		// m_bGetTokenInit = false;
	}

protected:
	// char m_debugOutput[1024];

	// Char m_charA;
	Token m_tokenA;
	mutable Token m_tokenB;
	mutable Char m_charB;
	// int m_lineCount;
	// int m_tokenCount;
	
	static const Byte s_normalCharMap[128];
	// 一般字符
	static inline bool IsNormalChar(Char ch) { return ((UChar) ch > 126u) || s_normalCharMap[ch]; }
	// 数字和.
	static inline bool IsNumChar(Char ch) { return ((ch >= _T('0') && ch <= _T('9')) || ch == _T('.')); }
	// 空白字符
	static inline bool IsBlankChar(Char ch) { return (ch == _T(' ') || ch == _T('\t') || ch == _T('\n') || ch == _T('\r')); }
	// 引号
	static inline bool IsQuote(Char ch) { return (ch == _T('\'') || ch == _T('\"')); }

	static const Byte s_singleOperNextCharMap[128];
	// 一般字符 + 空白字符 + 引号
	static inline bool IsSingleOperNextChar(Char ch) { return ((UChar) ch > 126u) || s_singleOperNextCharMap[ch]; }

	static const Byte s_singleOperMap[128];
	// 单字符符号
	static inline bool IsSingleOper(Char ch) { return ((UChar) ch <= 126u) && s_singleOperMap[ch]; }
	
	static const Char s_operCharBeforeReg[]; // 判断正则时，正则前面可以出现的字符
	
	
	bool GetToken(); // 处理过负数, 正则等等的 GetToken 函数

	inline void StartParse() {
		// m_startClock = clock();
		// m_bRegular = false;
		m_bFlag = 0;
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
	
	// 注释
	// inline bool IsComment() const { return (m_charA == _T('/') && (m_charB == _T('/') || m_charB == _T('*'))); } // 要联合判断 charA, charB

	void GetTokenRaw() const;

	void PrepareRegular() const; // 通过词法判断 tokenB 正则
	void PreparePosNeg()  const; // 通过词法判断 tokenB 正负数
	//void PrepareTokenB();

protected:
	// bool m_bRegular; // tokenB 实际是正则 GetToken 用到的两个成员状态
	mutable Byte m_bFlag;
	enum BoolFlag {
		PosNeg = 0x1, // tokenB 实际是正负数
	};
	//TokenQueue m_tokenBQueue;

	// bool m_bGetTokenInit; // 是否是第一次执行 GetToken

	// double m_duration;

private:

	// 阻止拷贝
	JSParser(const JSParser&);
	JSParser& operator =(const JSParser&);
};

#endif
