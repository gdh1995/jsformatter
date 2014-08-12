/* realjsformatter.cpp
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
#include "alldefs.h"
#include "realjsformatter.h"

static inline RealJSFormatter::Char GetStackTop(const RealJSFormatter::CharStack& stk) {
	return stk.top(); // outside will ensure that it's not empty
}

static inline bool StackTopEq(const RealJSFormatter::CharStack& stk, const RealJSFormatter::Char eq) {
	return eq == stk.top(); // outside will ensure that it's not empty
}
	
enum KEY_INDEX { KEY_NULL = 0
	, KEY_WITH		= 1 , KEY_SWITCH	= 2 , KEY_CATCH	= 3 , KEY_WHILE	= 4
	, KEY_FUNCTION	= 5 , KEY_RETURN	= 6 , KEY_FOR	= 7 , KEY_IF	= 8
};
// 后面要跟着括号的关键字的集合
static const RealJSFormatter::Char *const s_specKeywordSet[] = {
	_T("with"), _T("switch"), _T("catch"), _T("while"), _T("function"), _T("return"), _T("for"), _T("if")
};
static const size_t s_len_specKeywordSet[] = {
	4, 6, 5, 5, 8, 6, 3, 2
};
KEY_INDEX findInKeywordSet(const RealJSFormatter::Token &str) {
	const size_t len = str.size();
	int i = sizeof(s_specKeywordSet) / sizeof(s_specKeywordSet[0]);
	while (--i >= 0) {
		if (str.equals(s_specKeywordSet[i], s_len_specKeywordSet[i])) {
			return (KEY_INDEX) (i + 1);
		}
	}
	return (KEY_INDEX)0;
}


void RealJSFormatter::Init()
{
	m_lineBuffer.expand(sc_lineBufferReservedSize);

	m_nIndents = 0;
	m_nLineIndents = 0;
	m_eInsertChar = INSERT_NULL;
	m_bBlockStmt = true;
	m_bAssign = false;
	m_blockStack.push(JS_NULL); // stay having 1 element when finished
	m_brcNeedStack.push(true); // stay having 1 element when finished

}

// TODO: 修复';'后'{'不换行; 调整','换行逻辑, 改成var内可以不换行
// TODO: 增加'\n'识别逻辑
// TODO: 增加"var"的判定

//void RealJSFormatter::PutNewLine() {
//	if (!m_struOption.bNotPutCR)
//		m_lineBuffer.addOrDouble(_T('\r'));
//	m_lineBuffer.addOrDouble(_T('\n'));
//}

void RealJSFormatter::PutTokenA()
{
	const Char *const str = m_tokenA.c_str();
	if (m_eInsertChar >= INSERT_NEWLINE) {
		const Char ch = str[0];
		// 更改/mod: m_tokenA.length() >= 2的前提是'.'被算入normal_char
		if (m_eInsertChar == INSERT_FORCE_NEWLINE || ( (ch != _T('{') || m_struOption.bBracketAtNewLine) &&
			ch != _T(';') && ch != _T(',') && !(m_tokenA.length() >= 2 &&
			ch == _T('.') && (str[1] < _T('0') || str[1] > _T('9')) ) ))
		{
			// 换行后面不是紧跟着 {,; 才真正换
			PutLine(m_lineBuffer.c_str(), m_lineBuffer.size()); // 输出行缓冲
			m_lineBuffer.setLength(0);

			if (m_nIndents < 0)
				m_nIndents = 0; // 出错修正
			m_nLineIndents = m_nIndents;
			if (ch == _T('{') || ch == _T(',') || ch == _T(';')) // 行结尾是注释，使得{,;不得不换行
				--m_nLineIndents;
		}
	} else if (m_eInsertChar == INSERT_SPACE && m_lineBuffer.nempty()) {
		m_lineBuffer.addOrDouble(' ');
	}
	
	size_t const length = m_tokenA.size();
	for (size_t i = 0; i < length; ++i) {
		m_lineBuffer.addOrDouble(str[i]);
	}
}

void RealJSFormatter::PutLine(const Char *start, int len) const
{
	size_t len2 = 4;
	bool indent = false;
	if (len > 0 || m_struOption.bEmpytLineIndent) { // Fix "JSLint unexpect space" bug
		len2 += m_nLineIndents * m_struOption.hnChPerInd;
		indent = true;
	}
	if (len > 0) {
		len2 += len;
	}
	out.expand(out.size() + len2);

	Char *str = out.c_str() + out.size();

	// TODO: asm it
	if (indent) {
		for (register int i = m_nLineIndents * m_struOption.hnChPerInd; --i >= 0; )
			*str++ = m_struOption.chIndent; // 输出缩进
	}
	// 输出内容
	const Char* const end = start + len;
	while (start < end) {
		*str++ = *start++;
	}
	if (!m_struOption.bNotPutCR)
		*str++ = _T('\r');
	*str++ = _T('\n');
	out.setLength(str - out.c_str());
}

void RealJSFormatter::PopMultiBlock(const Char previousStackTop) const
{
	if (m_tokenB == _T(';')) // 如果 m_tokenB 是 ;，弹出多个块的任务留给它
		return;

	if ((previousStackTop != JS_IF  || m_tokenB.nequals(_T("else"),  4)) &&
		(previousStackTop != JS_DO  || m_tokenB.nequals(_T("while"), 5)) &&
		(previousStackTop != JS_TRY || m_tokenB.nequals(_T("catch"), 5)) )
	{
		// ; 还可能结束多个 if, do, while, for, try, catch
		bool next = true;
		do {
			register Char topStack = GetStackTop(m_blockStack);
			if (topStack == JS_IF) {
				if (m_tokenB.equals(_T("else"), 4)) { next = false; }
			} else if (topStack <= JS_ELSE) {
				if (topStack == JS_SWITCH) {
					break; // TODO: check whether it's useful
				}
			} else if (topStack == JS_TRY) {
				if (m_tokenB.equals(_T("catch"), 5)) { next = false; }
			} else if (topStack == JS_DO) {
				--m_nIndents;
				if (m_tokenB.equals(_T("while"), 5)) { break; }
				continue;
			} else {
				break;
			}
			--m_nIndents;
			m_blockStack.pop();
		} while (next);
	}
}

void RealJSFormatter::Go()
{
	StartParse();
	while (GetToken()) {
		if (!m_bBlockStmt && m_tokenA.more < COMMENT_1_TOKEN
			&& m_tokenA != _T('{') && m_tokenA != _T('\n')
			)
		{
			m_bBlockStmt = true;
		}

		/**
		 * 参考 m_tokenB 来处理 m_tokenA
		 * 输出或不输出 m_tokenA
		 * 下一次循环时自动会用 m_tokenB 覆盖 m_tokenA
		 */
		switch (m_tokenA.more) {
		case REGULAR_TOKEN:
			PutTokenA(); // 正则表达式直接输出，前后没有任何样式
			m_eInsertChar = INSERT_NULL;
			break;
		case OPER_TOKEN:
			ProcessOper();
			break;
		case STRING_TOKEN:
			ProcessString();
			break;
		case COMMENT_1_TOKEN:
			// 单行注释
			// 更改/mod: 手动换行
			m_eInsertChar = INSERT_SPACE;
			PutTokenA();
			m_eInsertChar = INSERT_NEWLINE;
			break;
		case COMMENT_2_TOKEN:
			// 多行注释
			PutTokenA();
			// TODO: 判断是否在语句内部, 比如用top == '{'来判断, 但不够完善
			m_eInsertChar = INSERT_NEWLINE; // 需要换行
			break;
		}
	}
	EndParse();
}

void RealJSFormatter::ProcessOper()
{
	const int alen = m_tokenA.size();
	Char topStack = GetStackTop(m_blockStack);

	if (alen <= 2) {
		if (m_tokenA.findIn(_T("()[]!~^.")) || m_tokenA.equals(_T('!'), _T('!'))) {
			// ()[]!. 都是前后没有样式的运算符
			if ((topStack == JS_ASSIGN || topStack == JS_HELPER) &&
				(m_tokenA == _T(')') || m_tokenA == _T(']')))
			{
				if (topStack == JS_ASSIGN)
					--m_nIndents;
				m_blockStack.pop();
				topStack = GetStackTop(m_blockStack);
			}
			if ((topStack == JS_BRACKET && m_tokenA == _T(')')) ||
				(topStack == JS_SQUARE && m_tokenA == _T(']')))
			{
				// )] 需要弹栈，减少缩进
				--m_nIndents;
				m_blockStack.pop();
				topStack = GetStackTop(m_blockStack);
				if (topStack == JS_ASSIGN || topStack == JS_HELPER) {
					m_blockStack.pop();
					topStack = GetStackTop(m_blockStack);
				}
			}

			PutTokenA(); // 此处不能移到上方if之前, 因为PutString可能要用到m_nIndents
			
			if (m_tokenA == _T(')') && !m_brcNeedStack.top() && topStack <= JS_SWITCH) {
				// 栈顶的 if, for, while, switch, catch 正在等待 )，之后换行增加缩进
				m_eInsertChar = INSERT_NEWLINE;

				m_brcNeedStack.pop();
				m_bBlockStmt = false; // 等待 statment
				if (topStack == JS_WHILE && m_blockStack._Get_container().end()[-2] == JS_DO) {
					m_blockStack.pop();
					m_blockStack.pop();
					PopMultiBlock(JS_WHILE); // 结束 do...while
					topStack = GetStackTop(m_blockStack);
				} else
					++m_nIndents;
			}
			else if (m_tokenA == _T(')') && (m_tokenB == _T('{'))) {
				m_eInsertChar = INSERT_SPACE; // { 或者换行之前留个空格
			}
			else if (m_tokenA == _T('(') || m_tokenA == _T('[')) {
				// ([ 入栈，增加缩进
				if (topStack == JS_ASSIGN) {
					if (m_bAssign) {
						m_blockStack.push(JS_HELPER);
						++m_nIndents;
					}
					else {
						//TODO:
					}
				}
				else
					++m_nIndents;
				m_blockStack.push(m_tokenA[0] == _T('(') ? JS_BRACKET : JS_SQUARE);
				m_eInsertChar = INSERT_NULL;
			}
			else
				m_eInsertChar = INSERT_NULL;
			return;
		}
		const Char ach0 = m_tokenA[0];
		if (alen == 1 && ach0 != _T('\n')) {
			if (ach0 == _T(';')) {
				if (topStack == JS_ASSIGN) {             
					--m_nIndents;
					m_blockStack.pop();
					topStack = GetStackTop(m_blockStack);
				}
				// 更改/mod: ; 结束 switch, 虽然语法上不合理, 但正确的js里这个没影响
				// ; 结束 if, else, while, for, try, catch
				if (topStack <= JS_TRY && (m_blockStack.pop(), true) ||
					topStack == JS_DO) // do 在读取到 while 后才修改计数, 对于 do{} 也一样
				{
					if (m_eInsertChar == INSERT_NEWLINE)
						m_eInsertChar = INSERT_SPACE;
					--m_nIndents;
					PopMultiBlock(topStack);
					topStack = GetStackTop(m_blockStack);
				}

				//	if (m_blockStack.top() == _T('t'))
				//		m_blockStack.pop(); // ; 也结束变量声明，暂时不压入 t

				PutTokenA();
				m_eInsertChar = (topStack != JS_BRACKET) ? INSERT_FORCE_NEWLINE // 如果不是"(...)"里的 ';' 就换行
					: INSERT_SPACE; // "(...; ...)" 空格
				return; // ;
			}
			else if (ach0 == _T(',')) {
				if (topStack == JS_ASSIGN) {
					--m_nIndents;
					m_blockStack.pop();
					topStack = GetStackTop(m_blockStack);
				}
				PutTokenA();
				// 如果是 "{...}" 里的就换行
				m_eInsertChar = (topStack == JS_BLOCK) ? INSERT_NEWLINE : INSERT_SPACE;
				return; // ,
			}
			else if (ach0 == _T('{')) {
				if (topStack <= JS_TRY || topStack == JS_DO) {
					if (m_bBlockStmt) {
						++m_nIndents;
						m_blockStack.push(JS_HELPER); // 压入一个 JS_HELPER 统一状态
					}
					else
						m_bBlockStmt = true;
				}
				else if (topStack == JS_FUNCTION) { // "(function ... {"减掉一个缩进
					if (m_blockStack._Get_container().end()[-2] != JS_BRACKET)
						++m_nIndents;
				}
				else if (topStack == JS_ASSIGN) {
					m_bBlockStmt = true;
				}
				else if (topStack != JS_BRACKET) { // 修正"({...})"中多一次缩进
					++m_nIndents;
				}
				m_blockStack.push(JS_BLOCK); // 入栈，增加缩进

				INSERT_MODE eNextInsert = (m_tokenB == _T('}'))  // 空 {}
					? INSERT_FORCE_NULL : INSERT_NEWLINE;
				if (m_eInsertChar == INSERT_NEWLINE) {
					if (m_struOption.bBracketAtNewLine)
						eNextInsert = INSERT_NEWLINE;
					else
						m_eInsertChar = INSERT_SPACE;
				}
				PutTokenA();
				m_eInsertChar = eNextInsert;
				return; // {
			}
			else if (ach0 == _T('}')) {
				// 激进的策略，} 一直弹到 {
				// 这样做至少可以使得 {} 之后是正确的
				do {
					m_blockStack.pop();
					if (topStack == JS_BLOCK) {
						topStack = GetStackTop(m_blockStack);
						if (topStack <= JS_HELPER) {
							m_blockStack.pop();
						}
						// JS_DO: (do nothing); // 缩进已经处理，do 留给 while
						--m_nIndents;
						break;
					}
					else if (topStack <= JS_DO) {
						--m_nIndents;
					}
					topStack = GetStackTop(m_blockStack);
				} while (topStack != JS_NULL);

				const bool bSpace = (m_eInsertChar != INSERT_FORCE_NULL)
					&& (m_eInsertChar = INSERT_NEWLINE, !m_struOption.bBracketAtNewLine);
				PutTokenA();
				if (m_tokenB.more == OPER_TOKEN) {
					if (m_tokenB == _T(';') || m_tokenB == _T(',') || (m_tokenB == _T(')'))) {
						// 更改/mod: 已经统一使(...{...})的{}前后不换行
						m_eInsertChar = INSERT_NULL; // }, }; })
					}
					else
						m_eInsertChar = INSERT_NEWLINE;
				}
				else if (m_tokenB.more == STRING_TOKEN && bSpace && (
					(topStack == JS_IF  && m_tokenB.equals(_T("else"),  4)) ||
					(topStack == JS_DO  && m_tokenB.equals(_T("while"), 5)) ||
					(topStack == JS_TRY && m_tokenB.equals(_T("catch"), 5))
					))
				{
					m_eInsertChar = INSERT_SPACE;
				}
				else
					m_eInsertChar = INSERT_NEWLINE;
				
				if (topStack != JS_ASSIGN && StackTopEq(m_blockStack, JS_BRACKET))
					++m_nIndents;

				PopMultiBlock(topStack);

				return; // }
			}
			else if (ach0 == _T(':')) {
				PutTokenA();
				if (topStack == JS_CASE) { // case, default
					m_eInsertChar = INSERT_NEWLINE_MAY_SPACE;
					m_blockStack.pop();
				}
				else
					m_eInsertChar = INSERT_SPACE;
				return;
			}
		}
		else {
			const Char ach1 = m_tokenA[1];
			if ((ach0 ==  _T('+') && ach1 ==  _T('+'))
				|| (ach0 ==  _T('-') && ach1 ==  _T('-'))
				//	|| (ach0 == _T('\r') && ach1 == _T('\n'))
				//	|| (ach0 == _T('\n') && alen ==   1 )
				//	|| (ach0 ==  _T(':') && ach1 ==  _T(':'))
				//	|| (ach0 ==  _T('-') && ach1 ==  _T('>'))
				)
			{
				PutTokenA();
				m_eInsertChar = INSERT_NULL;
				return;
			}
		}
	}

	if (topStack == JS_ASSIGN)
		; //m_bAssign = true;
	else if (m_tokenA == _T('=')) {
		m_blockStack.push(JS_ASSIGN);
		++m_nIndents;
		m_bAssign = false;
	}

	m_eInsertChar = INSERT_SPACE;
	PutTokenA(); // 剩余的操作符都是 空格oper空格
	m_eInsertChar = INSERT_SPACE;
}

void RealJSFormatter::ProcessString() {
	JS_TOKEN_TYPE token_type1 = JS_NULL;
	if (m_tokenA.equals(_T("else"), 4) && m_tokenB.nequals(_T("if"), 2)) {
		token_type1 = JS_ELSE;
	}
	else if (m_tokenA.equals(_T("do" ), 2)) {
		token_type1 = JS_DO;
	}
	else if (m_tokenA.equals(_T("try"), 3)) {
		token_type1 = JS_TRY;
	}
	else if (m_tokenA.equals(_T("finally"), 7)) {
		token_type1 = JS_TRY;
	}
	else {
		KEY_INDEX ki = KEY_NULL;
		if (m_tokenA.equals(_T("function"), 8)) {
			ki = KEY_FUNCTION;
			if (StackTopEq(m_blockStack, JS_ASSIGN)) {
				--m_nIndents;
				m_blockStack.top() = JS_FUNCTION;
			} else {
				m_blockStack.push(JS_FUNCTION); // 把 function 也压入栈，遇到 } 弹掉
			}
		}
		else if (m_tokenA.equals(_T("case"), 4) || m_tokenA.equals(_T("default"), 7)) {
			// case, default 往里面缩一格
			if (m_eInsertChar == INSERT_NEWLINE_MAY_SPACE) {
				m_eInsertChar = INSERT_SPACE;
				PutTokenA();
			} else {
				--m_nIndents;
				PutTokenA();
				++m_nIndents;
			}
			m_eInsertChar = (m_tokenA[0] != _T('d')) ? INSERT_SPACE : INSERT_NULL;
			m_blockStack.push(JS_CASE);
			return;
		}
		else if (StackTopEq(m_blockStack, JS_ASSIGN))
			m_bAssign = true;
	
		PutTokenA();

		if (m_tokenB.more >= STRING_TOKEN || m_tokenB == _T('{')) {
			m_eInsertChar = INSERT_SPACE; // '{': such as "return {'a':1};"
			//	if (m_blockStack.top() != _T('v'))
			//		m_blockStack.push(_T('v')); // 声明变量
			return;
		}

		if (ki == KEY_NULL) { ki = findInKeywordSet(m_tokenA); }
		m_eInsertChar = (ki != KEY_NULL && m_tokenB != _T(';'))
			? INSERT_SPACE : INSERT_NULL;

		switch (ki) {
		case KEY_IF:		token_type1 = JS_IF;		break;
		case KEY_FOR:		token_type1 = JS_FOR;		break;
		case KEY_WHILE:		token_type1 = JS_WHILE;		break;
		case KEY_CATCH:		token_type1 = JS_CATCH;		break;
		case KEY_SWITCH:	token_type1 = JS_SWITCH;	break;
		}
		if (token_type1 != JS_NULL) {
			m_blockStack.push(token_type1);
			m_brcNeedStack.push(false);
		}
		return;
	}
	// do, else (NOT else if), try
	PutTokenA();
	m_eInsertChar = INSERT_SPACE;
		
	m_blockStack.push(token_type1);
	++m_nIndents; // 无需 ()，直接缩进
	m_bBlockStmt = false; // 等待 block 内部的 statment
	if ((m_tokenB.more == STRING_TOKEN || m_struOption.bBracketAtNewLine)) {
		m_eInsertChar = INSERT_NEWLINE;
	}
	return;
}

