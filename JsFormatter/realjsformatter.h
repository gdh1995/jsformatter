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
	 *   READ_CR ��ȡ \r
	 *   SKIP_READ_CR ��ȡʱ���� \r
	 */
	enum CR_READ { SKIP_READ_CR, READ_CR };
	/*
	 * CR_PUT
	 *   PUT_CR ����ʹ�� \r\n
	 *   NOT_PUT_CR ����ʹ�� \n
	 */
	enum CR_PUT { NOT_PUT_CR, PUT_CR };
	/*
	 * BRAC_NEWLINE
	 *   NEWLINE_BRAC ����ǰ����
	 *   NO_NEWLINE_BRAC ����ǰ������
	 */
	enum BRAC_NEWLINE { NO_NEWLINE_BRAC, NEWLINE_BRAC };
	/*
	 * INDENT_IN_EMPTYLINE
	 *   INDENT_IN_EMPTYLINE ������������ַ�
	 *   NO_INDENT_IN_EMPTYLINE ���в���������ַ�
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
	
	void correctCommentFlag() { if(!m_bNewLine) m_bCommentPut = false; } // ���һ���ᷢ����ע��֮����κ��������
	
	inline void PutLF()		{ m_bNewLine = true; } // { PutString("\n", 1); }
	inline void PutSpace()	{ m_lineBuffer.addOrDouble(' '); } // { PutString(" ", 1); }
	inline void PutTokenA()	{ PutString(m_tokenA.c_str(), m_tokenA.length()); }
	void PutString(const Char *str, size_t const length);
	
	int m_nLineIndents;
	
	// StrCharMap m_blockMap;
	CharStack m_blockStack;
	int m_nIndents; // �������������ü��� blockStack��Ч������

	// ʹ��ջ��Ϊ�˽�����ж������г���ѭ��������
	BoolStack m_brcNeedStack; // if ֮��ĺ��������

	bool m_bNewLine; // ׼�����еı�־
	bool m_bHaveNewLine; // m_bHaveNewLine ��ʾ���潫Ҫ���У�m_bNewLine ��ʾ�Ѿ�������
	bool m_bBlockStmt; // block ������ʼ��
	bool m_bAssign;
	bool m_bEmptyBracket; // �� {}

	bool m_bCommentPut; // �ո������ע��

	const Char *m_initIndent; // ��ʼ����
	size_t m_len_initIndent;

	// ����Ϊ������
	const FormatterOption m_struOption;

private:
	// ��ֹ����
	RealJSFormatter(const RealJSFormatter&);
	RealJSFormatter& operator=(const RealJSFormatter&);
};

#endif
