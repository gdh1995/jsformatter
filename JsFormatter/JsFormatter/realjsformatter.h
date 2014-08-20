/* realjsformatter.h
   2010-12-16

Copyright (c) 2010-2013 SUN Junwen

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
#ifndef _REAL_JSFORMATTER_H_
#define _REAL_JSFORMATTER_H_

#include "jsparser.h"
#include "CharString.h"
#include <vector>
// #include <stack>

class RealJSFormatter: public JSParser
{
public:
	static const UInt sc_lineBufferReservedSize = 4000;
	typedef CharString<Char> OutputString; // output format

	// 更改/warn: 严禁修改JS_xxx的编号顺序, 除非到cpp里进行完全除错
	enum JS_TOKEN_TYPE {
		JS_FUNCTION	, // 'n'
		JS_TRY		, // 'try'; 'catch'; 'finally'
		JS_SWITCH	, // 's'
		// -
		JS_WHILE	, // 'w'
		JS_WHILE_GE	= JS_WHILE,
		JS_FOR		, // 'f'
		JS_WITH		= JS_FOR, // 'h'
		JS_IF		, // 'i'
		JS_IF_LE	= JS_IF,
		// -
		JS_ELSE		, // 'e'
		// -
		JS_DO		, // 'd'
		JS_DO_Lt	= JS_DO,
		// -
		JS_WRAP		, // wrap the line in a statment
		JS_WRAP_Lt	= JS_WRAP,
		JS_WRAP_LE	= JS_WRAP,
		// -
		JS_BRACKET	, // means '('
		JS_BRACKET_LE=JS_BRACKET,
		// -
		JS_SQUARE	, // means '['
		// -
		JS_CASE		, // 'c'
		JS_CASE_Lt	= JS_CASE,
		// -
		JS_BLOCK	, // means '{'
		JS_BLOCK_GE	= JS_BLOCK,
		JS_NULL		,
		JS_OthersGt	= JS_NULL,
		// -
		// more items
		JS_BLOCK_VIRTUAL, // 已经读入"()"
		JS_BRACKET_GOT	, // 已经读入"()"
		JS_BRACKET_WANT	, // 尚未读入"()"
	};
	typedef Byte JsToken;
	typedef std::vector<JsToken> ByteStack;

	struct FormatterOption {
		Char chIndent;
		unsigned short uhChPerInd;
		bool bNotPutCR; // false: 换行使用"\r\n"; true: 换行使用'\n'
		bool bBracketAtNewLine; // false: 括号前不换行
		bool bEmpytLineIndent; // false: 空行不输出缩进字符
	};

public:
	RealJSFormatter(const Byte* input, UInt len_in, OutputString &output, const FormatterOption &option)
		: m_struOption(option), m_out(&output)
		, JSParser(input, len_in)
	{
		init();
	}

	void init() const;
	inline OutputString &getOutput() const { return *m_out; }
	inline void setInitIndents(const int initIndents) const { m_nIndents = initIndents; }
	inline void reserveLineBuffer(const UInt len) const { m_line.expand(len); }

	void go() const;
	
	JS_TOKEN_TYPE findSomeKeyword() const;

protected:
	// "Force"只表示需求这种, 不该起决定作用
	// 严禁修改INSERT_xxx的编号顺序, 除非到cpp里进行完全除错
	enum INSERT_MODE {
		INSERT_NONE = 0,
		INSERT_UNKNOWN = 1,
		// -
		INSERT_SPACE = 2, // 要提前输出一个空格
		// -
		INSERT_NEWLINE = 3, // 准备换行, 但有可能变成空格
		// -
		INSERT_NEWLINE_SHOULD = 4, // 更应该输出换行
		// -
		INSERT_NULL = 5 // 已经输出换行过了
	};

	// 带格式输出指定字符串到行缓冲区
	void putTokenRaw(const Char *const start, int len) const;
	
	// 带格式输出m_tokenA到行缓冲区
	inline void putTokenA() const { putTokenRaw(m_tokenA, m_tokenA.size()); }
	
	// 带缩进输出字符串及换行标志到m_out
	void putLine(const Char *start, int len, bool insertBlank) const;
	
	// 带缩进输出行缓冲区到m_out
	inline void putBufferLine() const {
		putLine(m_line, m_line.size(), false);
		m_line.setLength(0);
	}

	inline void stackPop() const { return m_blockStack.pop_back(); }
	inline Byte stackTop() const { return m_blockStack.back(); }
	inline Byte stackTop2()const { return m_blockStack.end()[-2]; }
	inline void stackTop  (const JS_TOKEN_TYPE ch) const { m_blockStack.back() = ch; }
	inline bool stackTopEq(const JS_TOKEN_TYPE eq) const { return eq == m_blockStack.back(); }
	inline void stackPush (const JS_TOKEN_TYPE ch) const { m_blockStack.push_back(ch); }
	
	void startParse() const {
		m_line.setLength(0);
		m_line.c_str()[0] = 0;
		this->JSParser::startParse();
	}
	void endParse() const {
		this->JSParser::endParse();
		if (m_line.more < INSERT_NULL) {
			m_line.more = INSERT_NULL;
			putBufferLine();
		}
		m_out->c_str()[m_out->size()] = 0;
	}
	
protected:
	mutable int m_nIndents; // 缩进数量
	mutable unsigned short m_uhLineIndents;
private:
	mutable ByteStack m_blockStack;
protected:
	mutable OutputString m_line;

	void popMultiBlock(const Char previousStackTop) const;

	typedef void RealJSFormatter::ProcessFunc() const;
	typedef void (RealJSFormatter::*ProcessFuncPtr)() const;
	ProcessFunc processOper, processCommonToken,
				processID, processOrPutMultiLineString,
				processLineComment, processNewLineComment,
				processAndPutBlankLine, processAndPutBlockComment;
	
private:
	OutputString *m_out;
public:
	FormatterOption m_struOption; // 配置项

private: // 阻止拷贝
	RealJSFormatter(const RealJSFormatter&);
	RealJSFormatter(RealJSFormatter&&);
	RealJSFormatter& operator= (const RealJSFormatter&);
	RealJSFormatter& operator= (RealJSFormatter&&);
};

#endif
