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
// #include <map>

#include "jsparser.h"
#include "CharString.h"
#include <vector>

class RealJSFormatter: public JSParser
{
public:
	static const size_t sc_lineBufferReservedSize = 4000;
	typedef CharString<Char> CharString; // output format

	// 更改/warn: 严禁修改JS_xxx的编号顺序, 除非到cpp里进行完全除错
	enum JS_TOKEN_TYPE {
		JS_IF		=  0, // 'i'
		// - 发现JS_FOR和JS_CATCH都没有具体出现过
		JS_FOR		=  1, JS_CATCH	=  1, // 'f', 'h'
		JS_WHILE	=  2, // 'w'
		// -
		JS_SWITCH	=  3, // 's'
		// -
		JS_ELSE		=  4, // 'e'
		// -
		JS_TRY		=  5, // 't'; "finally"
		// -
		JS_DO		=  6, // 'd'
		// -
		JS_FUNCTION	=  7, // 'n';
		// -
		JS_CASE		=  8, // 'c'
		JS_SQUARE	=  9, // means '['
		JS_BRACKET	= 10, // means '('
		// -
		JS_BLOCK	= 11, // means '{'
		// -
		JS_NULL		= 12
	};
	
	typedef std::vector<Char> ByteStack;
	
	static JS_TOKEN_TYPE findSomeKeyword(const Token &str);

	struct FormatterOption {
		unsigned short uhChPerInd;
		Char chIndent;
		bool bNotPutCR; // false: 换行使用"\r\n"; true: 换行使用'\n'
		bool bBracketAtNewLine; // false: 括号前不换行
		bool bEmpytLineIndent; // false: 空行不输出缩进字符
	};
	
private:
	CharString *m_out;

public:
	RealJSFormatter(const Byte* input, size_t len_in, CharString &output, const FormatterOption &option)
		: m_struOption(option), m_out(&output)
		, JSParser(input, len_in)
	{
		Init();
	}

	void Init() const;
	inline CharString &getOutputObject() { return *m_out; }
	inline void setInitIndents(const int initIndents) { m_nIndents = initIndents; }
	inline void reserveLineBuffer(const size_t len) { m_line.expand(len); }

	void Go() const;

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
	void PutTokenRaw(const Char *const start, int len) const;
	
	// 带格式输出m_tokenA到行缓冲区
	inline void PutTokenA() const { PutTokenRaw(m_tokenA.c_str(), m_tokenA.size()); }
	
	// 带缩进输出字符串及换行标志到m_out
	void PutLine(const Char *start, int len, bool insertBlank) const;
	
	// 带缩进输出行缓冲区到m_out
	inline void PutBufferLine() const {
		PutLine(m_line.c_str(), m_line.size(), false);
		m_line.setLength(0);
	}

	inline Char StackTop() const { return m_blockStack.back(); }
	inline Char StackTop2() const { return m_blockStack.end()[-2]; }
	inline void StackPush(const JS_TOKEN_TYPE ch) const { return m_blockStack.push_back(ch); }
	inline void StackPop() const { return m_blockStack.pop_back(); }
	inline bool StackTopEq(const JS_TOKEN_TYPE eq) const { return eq == m_blockStack.back(); }
	inline bool StackTopGE(const JS_TOKEN_TYPE eq) const { return eq <= m_blockStack.back(); }

	void StartParse() const {
		m_line.setLength(0);
		m_line.c_str()[0] = 0;
		this->JSParser::StartParse();
	}
	void EndParse() const {
		this->JSParser::EndParse();
		PutBufferLine();
		m_out->c_str()[m_out->size()] = 0;
	}
	
protected:
	mutable CharString m_line;
	mutable int m_nIndents; // 缩进数量 (计算blockStack效果不好)
	mutable unsigned short m_uhLineIndents;
	mutable bool m_bMoreIndent; // 是否在语句中
	mutable bool m_bNeedBracket; // if 之类的后面的括号 (使用栈是为了解决在判断条件中出现循环的问题)

	void PopMultiBlock(const Char previousStackTop) const;
	void ProcessOper() const;
	void ProcessRegular() const;
	void ProcessID() const;
	void ProcessCommonToken() const;
	void ProcessOrPutString() const;
	void ProcessLineComment() const;
	void ProcessAndPutBlockComment() const;
	void ProcessAndPutBlankLine() const;

private:
	mutable ByteStack m_blockStack;
	//mutable BoolStack m_brcNeedStack; // if 之类的后面的括号 (使用栈是为了解决在判断条件中出现循环的问题)

public:
	FormatterOption m_struOption; // 配置项

private: // 阻止拷贝
	RealJSFormatter(const RealJSFormatter&);
	RealJSFormatter(RealJSFormatter&&);
	RealJSFormatter& operator= (const RealJSFormatter&);
	RealJSFormatter& operator= (RealJSFormatter&&);
};

#endif
