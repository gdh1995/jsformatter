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

class RealJSFormatter: public JSParser
{
public:
	// typedef map<string, Char> StrCharMap;
	typedef const Char *StrItemInSet;

	/*
	 * CR_READ
	 *   READ_CR 读取 \r
	 *   SKIP_READ_CR 读取时跳过 \r
	 */
	enum CR_READ { SKIP_READ_CR, READ_CR };
	/*
	 * CR_PUT
	 *   PUT_CR 换行使用 \r\n
	 *   NOT_PUT_CR 换行使用 \n
	 */
	enum CR_PUT { NOT_PUT_CR, PUT_CR };
	/*
	 * BRAC_NEWLINE
	 *   NEWLINE_BRAC 括号前换行
	 *   NO_NEWLINE_BRAC 括号前不换行
	 */
	enum BRAC_NEWLINE { NO_NEWLINE_BRAC, NEWLINE_BRAC };
	/*
	 * INDENT_IN_EMPTYLINE
	 *   INDENT_IN_EMPTYLINE 空行输出缩进字符
	 *   NO_INDENT_IN_EMPTYLINE 空行不输出缩进字符
	 */
	enum EMPTYLINE_INDENT { NO_INDENT_IN_EMPTYLINE, INDENT_IN_EMPTYLINE };

	struct FormatterOption 
	{
		int chIndent;
		int nChPerInd;
		CR_READ eCRRead;
		CR_PUT eCRPut;
		BRAC_NEWLINE eBracNL;
		EMPTYLINE_INDENT eEmpytIndent;

	};
	
	enum KEY_INDEX { KEY_NONE = 0
		, KEY_WITH		= 1 , KEY_SWITCH	= 2 , KEY_CATCH	= 3 , KEY_WHILE	= 4
		, KEY_FUNCTION	= 5 , KEY_RETURN	= 6 , KEY_FOR	= 7 , KEY_IF	= 8
	};

	//virtual ~RealJSFormatter()
	//{}

	inline void SetInitIndent(const Char *initIndent) { m_initIndent = initIndent; }

	void Go();

	static const Char* Trim(const CharString &str, const Char ** const pend);
	static const Char* RealJSFormatter::TrimRightSpace(const CharString &str);

	RealJSFormatter(const ::CharString<Byte> &input, CharString &output, const FormatterOption &option)
		: m_struOption(option), out(output)
		, JSParser(input.c_str(), input.length())
	{
		Init();
	}
	
protected:
	CharString &out;
	void PutLine(const Char *start, const Char *const end);
	inline void StartParse() {
		this->JSParser::StartParse();
		m_lineBuffer.setLength(0);
		m_lineBuffer.c_str()[0] = 0;
	}
	inline void EndParse() {
		const Char *end;
		const Char *start = Trim(m_lineBuffer, &end);
		if(start < end) { PutLine(start, end); }
		out.c_str()[out.length()] = 0;
		this->JSParser::EndParse();
	}

private:
	CharString m_lineBuffer;

	void Init();

	void PopMultiBlock(const Char previousStackTop);
	void ProcessOper(const Char ach0);
	void ProcessString();
	
	void correctCommentFlag() { if(!m_bNewLine) m_bCommentPut = false; } // 这个一定会发生在注释之后的任何输出后面
	
	inline void PutLF()		{ m_bNewLine = true; } // { PutString("\n", 1); }
	inline void PutSpace()	{ m_lineBuffer.addOrDouble(' '); } // { PutString(" ", 1); }
	inline void PutTokenA()	{ PutString(m_tokenA.c_str(), m_tokenA.length()); }
	void PutString(const Char *str, size_t const length);
	
	int m_nLineIndents;
	
	// StrCharMap m_blockMap;
	CharStack m_blockStack;
	int m_nIndents; // 缩进数量，不用计算 blockStack，效果不好

	// 使用栈是为了解决在判断条件中出现循环的问题
	BoolStack m_brcNeedStack; // if 之类的后面的括号

	bool m_bNewLine; // 准备换行的标志
	bool m_bHaveNewLine; // m_bHaveNewLine 表示后面将要换行，m_bNewLine 表示已经换行了
	bool m_bBlockStmt; // block 真正开始了
	bool m_bAssign;
	bool m_bEmptyBracket; // 空 {}

	bool m_bCommentPut; // 刚刚输出了注释

	const Char *m_initIndent; // 起始缩进
	size_t m_len_initIndent;

	// 以下为配置项
	const FormatterOption m_struOption;

private:
	// 阻止拷贝
	RealJSFormatter(const RealJSFormatter&);
	RealJSFormatter& operator=(const RealJSFormatter&);
};

#endif
