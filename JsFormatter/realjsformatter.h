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
#include <stack>

class RealJSFormatter: public JSParser
{
public:
	static const size_t sc_lineBufferReservedSize = 4000;
	typedef CharString<Char> CharString; // out format

	typedef std::stack<Char> CharStack;
	typedef std::stack<bool> BoolStack;
	// ����/warn: �Ͻ��޸�JS_xxx�ı��˳��, ���ǵ�cpp�������ȫ����
	enum JS_TOKEN_TYPE {
		JS_IF		=  0, // 'i'
		// -
		JS_FOR		=  1, // 'f'
		JS_WHILE	=  2, // 'w'
		JS_CATCH	=  3, // 'h'
		JS_SWITCH	=  4, // 's'
		// -
		JS_ELSE		=  5, // 'e'
		// -
		JS_TRY		=  6, // 't'; "finally"
		// -
		JS_FUNCTION	=  7, // 'n'
		JS_ASSIGN	=  8, // means '='
		// -
		JS_HELPER	=  9, // means '\\'
		// -
		JS_DO		= 10, // 'd'
		// -
		JS_CASE		= 11, // 'c'
		JS_BLOCK	= 12, // means '{'
		JS_BRACKET	= 13, // means '('
		JS_SQUARE	= 14, // means '['
		JS_NULL		= 15
	};
	static const Byte s_tokenIDMap[128];
	static inline JS_TOKEN_TYPE GetTokenIndex(Char ch) { return (JS_TOKEN_TYPE) s_tokenIDMap[ch]; }

	struct FormatterOption {
		short hnChPerInd;
		Char chIndent;
		bool bNotPutCR; // false: ����ʹ��"\r\n"; true: ����ʹ��'\n'
		bool bBracketAtNewLine; // false: ����ǰ������
		bool bEmpytLineIndent; // false: ���в���������ַ�
	};

	inline void setInitIndents(const int initIndents) { m_nIndents = initIndents; }
	inline void reserveLineBuffer(const size_t len) { m_lineBuffer.expand(len); }

	void Go();

	RealJSFormatter(const ::CharString<Byte> &input, CharString &output, const FormatterOption &option)
		: m_struOption(option), out(output)
		, JSParser(input.c_str(), input.length())
	{
		Init();
	}
	
protected:
	mutable CharString &out;
	void PutLine(const Char *start, int len) const;
	void StartParse() const {
		m_lineBuffer.setLength(0);
		m_lineBuffer.c_str()[0] = 0;
		this->JSParser::StartParse();
	}
	void EndParse() const {
		this->JSParser::EndParse();
		int len;
		const Char *start = m_lineBuffer.trim(&len);
		if(len > 0) { PutLine(m_lineBuffer.c_str(), m_lineBuffer.length()); }
		out.c_str()[out.length()] = 0;
	}

	// rem: after it, m_eInsertChar should be set INSERT_NULL manually
	void PutTokenA();
	void PutNewLine();
	
	// ������
	const FormatterOption m_struOption;

private:
	mutable CharString m_lineBuffer;

	void Init();

	void PopMultiBlock(const Char previousStackTop) const;
	void ProcessOper();
	void ProcessString();
	
	mutable CharStack m_blockStack;
	mutable int m_nIndents; // �������� (����blockStackЧ������)
	short m_nLineIndents;

	// ʹ��ջ��Ϊ�˽�����ж������г���ѭ��������
	BoolStack m_brcNeedStack; // if ֮��ĺ��������

	// "Force"ֻ��ʾ��������, �������������
	enum INSERT_MODE {
		INSERT_NULL = 0,
		INSERT_FORCE_NULL = 1,
		INSERT_SPACE = 2, // Ҫ��ǰ���һ���ո�
		INSERT_FORCE_SPACE = 3, // Ҫ��ǰ���һ���ո�
		INSERT_NEWLINE = 4, // ׼�����еı�־
		INSERT_NEWLINE_MAY_SPACE = 5,
		INSERT_FORCE_NEWLINE = 6
	};
	char m_eInsertChar;

	// TODO: check if we can remove it
	bool m_bBlockStmt; // block ������ʼ��
	bool m_bAssign;

private: // ��ֹ����
	RealJSFormatter(const RealJSFormatter&);
	RealJSFormatter(RealJSFormatter&&);
	RealJSFormatter& operator= (const RealJSFormatter&);
	RealJSFormatter& operator= (RealJSFormatter&&);
};

#endif
