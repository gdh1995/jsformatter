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

using namespace std;

class RealJSFormatter: public JSParser
{
public:
	// typedef map<string, char> StrCharMap;
	typedef const char *StrItemInSet;

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
	
	static const StrItemInSet s_specKeywordSet[]; // ����Ҫ�������ŵĹؼ��ּ���
	static int RealJSFormatter::findInKeywordSet(const CharString<char> &str);

	//virtual ~RealJSFormatter()
	//{}

	inline void SetInitIndent(const char *initIndent) { m_initIndent = initIndent; }

	void Go();

	static const char* Trim(const CharString<char> &str, const char **pend);
	static const char* RealJSFormatter::TrimRightSpace(const CharString<char> &str);

	RealJSFormatter(const CharString<char> &input, CharString<char> &output, const FormatterOption &option)
		: m_struOption(option), out(&output)
		, JSParser(input.c_str(), input.length())
	{
		Init();
	}
	
protected:
	CharString<char> *out;
	inline void PutChar(int ch)
	{ 
		out->addOrDouble(ch);
	}

private:

	void Init();

	void PopMultiBlock(char previousStackTop);
	void ProcessOper(bool bHaveNewLine, char tokenAFirst, char tokenBFirst);
	void ProcessString(bool bHaveNewLine, char tokenAFirst, char tokenBFirst);
	
	void correctCommentFlag();
	void PutToken(const char token);
	void PutToken(const CharString<char>& token);
	void PutString(const char *str, size_t const length);
	inline void PutLineBuffer() { PutLineBuffer(m_lineBuffer.c_str(), TrimRightSpace(m_lineBuffer)); }
	void PutLineBuffer(const char *start, const char *const end);

	int m_nLineIndents;
	CharString<char> m_lineBuffer;
	
	// StrCharMap m_blockMap;
	CharStack m_blockStack;
	int m_nIndents; // �������������ü��� blockStack��Ч������

	// ʹ��ջ��Ϊ�˽�����ж������г���ѭ��������
	BoolStack m_brcNeedStack; // if ֮��ĺ��������

	bool m_bNewLine; // ׼�����еı�־
	bool m_bBlockStmt; // block ������ʼ��
	bool m_bAssign;
	bool m_bEmptyBracket; // �� {}

	bool m_bCommentPut; // �ո������ע��

	const char *m_initIndent; // ��ʼ����

	// ����Ϊ������
	FormatterOption m_struOption;

private:
	// ��ֹ����
	RealJSFormatter(const RealJSFormatter&);
	RealJSFormatter& operator=(const RealJSFormatter&);
};

#endif
