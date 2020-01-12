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

#undef _T
#undef JS_PARSER_CHAR_SIZE
#if defined UNICODE || defined _UNICODE
	typedef wchar_t Char;
	typedef unsigned short UChar;
	#define _T(x) (L##x)
	#define JS_PARSER_CHAR_SIZE 2
#else
	typedef char Char;
	typedef unsigned char UChar;
	#define _T(x) (x)
	#define JS_PARSER_CHAR_SIZE 1
#endif

	// 严禁修改TOKEN_xxx的编号顺序, 除非到cpp里进行完全除错
	enum TOKEN_TYPE: unsigned char {
		TOKEN_OPER = 0,
		
		TOKEN_COMMON = 1,
		TOKEN_STRING = TOKEN_COMMON,
		TOKEN_REGULAR = TOKEN_COMMON,
		TOKEN_ID = 2,
		TOKEN_STRING_MULTI_LINE = 3,

		// 为了配合prepareTokenB()
		// Comment类的token需要大于任意其它token
		TOKEN_COMMENT_LINE = 4, // 单行注释
		TOKEN_COMMENT_NEWLINE = 5, // 单行注释 (位于新行)
		TOKEN_BLANK_LINE = 6, // 空行标记
		TOKEN_COMMENT_BLOCK = 7, // 多行注释
		TOKEN_NULL = TOKEN_COMMENT_LINE
	};
	// .more: Token 类型
  typedef ConstString<Char, char, TOKEN_TYPE> Token;

	// 一般字符 (包括数字和 . ) 的bool映射
	static const  Byte s_normalCharMap[128];
	
	// 一般字符 (包括数字和 . )
	static inline bool IsNormalChar(Char ch) { return ((UChar) ch > 126u) || s_normalCharMap[ch]; }
	
	// 数字和. (unused)
	static inline bool IsNumChar(Char ch) { return ((ch >= _T('0') && ch <= _T('9')) || ch == _T('.')); }
	
	// 空白字符
	static inline bool IsBlankChar(Char ch) { return (ch == _T(' ') || ch == _T('\t') || ch == _T('\n') || ch == _T('\r')); }
	
	// 引号
	static inline bool IsQuote(Char ch) { return (ch == _T('\'') || ch == _T('\"')); }
	
	// 单字符操作符之后可能跟的字符的bool映射
	static const  Byte s_singleOperNextCharMap[128];

	// 一般字符 + 空白字符 + 引号
	static inline bool IsSingleOperNextChar(Char ch) { return ((UChar) ch > 126u) || s_singleOperNextCharMap[ch]; }

	// 单字符操作符的bool映射
	static const  Byte s_singleOperMap[128];
	
	// 单字符符号
	static inline bool IsSingleOper(Char ch) { return ((UChar) ch <= 126u) && s_singleOperMap[ch]; }
	
	// 判断正则时，正则前面可以出现的字符
	static const  Char s_operCharBeforeReg[];
	
public:
	explicit JSParser() {
		// m_debug = false;
		init();
	}

	explicit JSParser(const Byte *in, UInt len_in)
		: m_in(in), m_in_next(in)
		, m_in_end(in + len_in)
	{
		init();
	}
	
	inline void init() const {
	}

	inline void setInput(const Byte *in, UInt len_in = 0) {
		m_in = in;
		m_in_next = in;
		m_in_end = in + len_in;
	}

	// bool m_debug;
	// inline const char *GetDebugOutput() const { return m_debugOutput; }
	// void PrintDebug() const;
	
public:
	// char m_debugOutput[1024];
	
public:		mutable Token m_tokenA;
protected:	mutable Token m_tokenB;
public:
	inline const Token& getTokenB() const { return m_tokenB; }

	inline bool getToken() const {
		prepareTokenB();
		// Attention: In the debug mode, raw() is so ugly and slow
		//     that we have to give up it.
		//	m_tokenA.raw() = m_tokenB.raw();
		(Token::BaseString&)m_tokenA = (const Token::BaseString&)m_tokenB;
		getTokenRaw();
		return m_tokenA.nempty();
	}

	inline void startParse() const {
		// m_startClock = clock();
		// m_bRegular = false;
		m_tokenA.more = TOKEN_NULL;
		m_tokenB.more = TOKEN_NULL;
		lastChar(getChar());
		getTokenRaw();
	}

	inline void endParse() const {
		// m_endClock = clock();
		// m_duration = (double)(m_endClock - m_startClock) / CLOCKS_PER_SEC;
		// PrintDebug();
	}

protected:
	inline Char getChar() const {
		return (m_in_next < m_in_end) ? (*m_in_next++) : 0;
	}
	
	inline const Char *curPos() const { return m_in_next; }
	
	inline void readBack(const Char *const pos) const { m_in_next = pos; }
#if		JS_PARSER_CHAR_SIZE == 1
	inline Char lastChar() const { return m_tokenB.getFlag1(); }
	inline void lastChar(const Char ch) const { m_tokenB.setFlag1(ch); }
#elif	JS_PARSER_CHAR_SIZE == 2
	inline Char lastChar() const { return m_tokenB.getFlag(); }
	inline void lastChar(const Char ch) const { m_tokenB.setFlag(ch); }
#endif
	inline void checkEnd() const {
		if (lastChar() == 0) m_in_next = m_in_end + 1;
	}
	
protected:
	enum BoolFlag {
		PosNeg = 0x1, // tokenB 实际是正负数
		InNewLine = 0x2, // 在token之前是换行
	};

	// 获取下一个token并存储到m_tokenB
	void getTokenRaw(int bFlag = 0) const;
	
	// 通过词法判断 m_tokenB 是否代表正则或者正负数等等并加以处理
	void prepareTokenB() const;

private:
	mutable const Char *m_in_next;
	const Char *m_in_end;
	const Char *m_in;

private: // 阻止拷贝
	JSParser(const JSParser&);
	JSParser(JSParser&&);
	JSParser& operator =(const JSParser&);
	JSParser& operator =(JSParser&&);
};

#endif
