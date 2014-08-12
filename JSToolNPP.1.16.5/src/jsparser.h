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

using namespace std;

#define STRING_TYPE 0
#define OPER_TYPE 1
#define REGULAR_TYPE 2
#define COMMENT_TYPE_1 9 // 单行注释
#define COMMENT_TYPE_2 10 // 多行注释

/*
* if-i, else-e, else if-i,
* for-f, do-d, while-w,
* switch-s, case-c, default-c
* try-r, catch-h
* {-BLOCK, (-BRACKET
* 0-empty
*/
#define JS_IF 'i'
#define JS_ELSE 'e'
#define JS_FOR 'f'
#define JS_DO 'd'
#define JS_WHILE 'w'
#define JS_SWITCH 's'
#define JS_CASE 'c'
#define JS_TRY 't'
#define JS_CATCH 'h'
#define JS_FUNCTION 'n'
#define JS_ASSIGN '='
#define JS_BLOCK '{'
#define JS_BRACKET '('
#define JS_SQUARE '['
#define JS_HELPER '\\'
#define JS_EMPTY 0

class JSParser
{
protected:
	// .type: Token 类型
	typedef CharString<char> Token; // Token item

public:
	typedef stack<char> CharStack;
	typedef stack<bool> BoolStack;
	typedef queue<Token> TokenQueue;

	explicit JSParser() {
		Init();
	}

	//virtual ~JSParser()
	//{}

	bool m_debug;
	inline const char *GetDebugOutput() { return m_debugOutput; }
	
	void Init();

protected:
	char m_debugOutput[1024];

	int m_charA;
	int m_charB;
	Token m_tokenA;
	Token m_tokenB;
	int m_lineCount;
	int m_tokenCount;
	
	// 一般字符
	bool inline IsNormalChar(int ch);
	// 数字和.
	bool inline IsNumChar(int ch) { return ((ch >= '0' && ch <= '9') || ch == '.'); }
	// 空白字符
	bool inline IsBlankChar(int ch) { return (ch == ' ' || ch == '\t' || ch == '\r'); }
	// 单字符符号
	bool inline IsSingleOper(int ch);
	// 引号
	bool inline IsQuote(int ch) { return (ch == '\'' || ch == '\"'); }

	bool GetToken(); // 处理过负数, 正则等等的 GetToken 函数

	void inline StartParse() {
		// m_startClock = clock();
	}

	void inline EndParse()
	{
		// m_endClock = clock();
		// m_duration = (double)(m_endClock - m_startClock) / CLOCKS_PER_SEC;
		PrintDebug();
	}

protected:
	const char *m_in;
	size_t m_len_in;
	size_t m_getPos;
	inline int GetChar()
	{
		if(m_getPos < m_len_in)
			return m_in[m_getPos++];
		else
			return 0;
	}

	inline const char *pos() const {
		return m_in + m_getPos;
	}

	explicit JSParser(const char *in, size_t len_in)
		: m_in(in), m_len_in(len_in), m_getPos(0)
	{
		Init();
	}

private:
	// Should be implemented in derived class
	//virtual int GetChar() = 0; // JUST get next char from input
	
	// 注释
	bool inline IsComment() { return (m_charA == '/' && (m_charB == '/' || m_charB == '*')); } // 要联合判断 charA, charB

	void GetTokenRaw();

	void PrepareRegular(); // 通过词法判断 tokenB 正则
	void PreparePosNeg(); // 通过词法判断 tokenB 正负数
	void PrepareTokenB();

	void PrintDebug();

	static const char s_strBeforeReg[]; // 判断正则时，正则前面可以出现的字符

	bool m_bRegular; // tokenB 实际是正则 GetToken 用到的两个成员状态
	bool m_bPosNeg; // tokenB 实际是正负数
	TokenQueue m_tokenBQueue;

	bool m_bGetTokenInit; // 是否是第一次执行 GetToken

	// double m_duration;

private:
	// 阻止拷贝
	JSParser(const JSParser&);
	JSParser& operator=(const JSParser&);
};

#endif
