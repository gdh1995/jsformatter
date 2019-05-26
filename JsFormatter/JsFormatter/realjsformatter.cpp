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

// 根据后面要跟着括号的关键字的集合来判断
RealJSFormatter::JS_TOKEN_TYPE RealJSFormatter::findSomeKeyword() const {
	switch(m_tokenA.size()) {
	case 2:
		if (m_tokenA.equals(_T('i'), _T('f'))) return JS_IF;
		break;
	case 3:
		if (m_tokenA.equals0(_T("for"))) return JS_FOR;
		break;
	case 4:
		if (m_tokenA.equals0(_T("with"))) return JS_WITH;
		break;
	case 5:
		if (m_tokenA.equals0(_T("while"))) return JS_WHILE;
		else if (m_tokenA.equals0(_T("catch"))) return JS_TRY;
		break;
	case 6:
		if (m_tokenA.equals0(_T("return"))) return JS_BLOCK; // 此返回值不该被进一步使用
		else if (m_tokenA.equals0(_T("switch"))) return JS_SWITCH;
		break;
	default: break;
	}
	return JS_NULL;
}

void RealJSFormatter::init() const {
	m_line.expand(sc_lineBufferReservedSize);

	m_nIndents = 0;
	m_uhLineIndents = 0;
	m_line.more = INSERT_NULL;
	m_blockStack.clear();
	m_blockStack.reserve(64);
	m_blockStack.push_back(JS_NULL); // stay having 1 element when finished
	//m_brcNeedStack.push_back(true); // stay having 1 element when finished
}

// 更改/mod: 修复';'后'{'不换行; "if"等紧跟';'时插入空格
// 更改/mod: 补回连续换行的处理逻辑
// TO_DO_MAY: 增加"var"的判定, 调整','换行逻辑, 改成var内可以不换行

void RealJSFormatter::putTokenRaw(const Char*const str, int len) const {
	if (m_line.more >= INSERT_NEWLINE) {
		if (m_line.more != INSERT_NULL)
			putBufferLine();
		if (m_nIndents < 0)
			m_nIndents = 0; // 出错修正
		m_uhLineIndents = m_nIndents;
	}
	else if (m_line.more >= INSERT_SPACE) {
		// 不需要判断m_line.nempty(), 因为要在putTokenA()之前判断好
		m_line.addOrDouble(' ');
	}
	if (len > 1)
		m_line.addStr(str, len);
	else if (len == 1)
		m_line.addOrDouble(*str);
}

void RealJSFormatter::putLine(const Char *start, int len, bool insertBlank) const {
	UInt len2 = 4;
	bool indent = false;
	if (len > 0) {
		len2 += len;
	}
	if (len > 0 || m_struOption.bEmpytLineIndent) { // Fix "JSLint unexpect space" bug
		len2 += m_uhLineIndents * m_struOption.uhChPerInd;
		indent = true;
	}
	m_out->expand(m_out->size() + len2);

	Char *str = m_out->c_str() + m_out->size();

	// TODO: asm it
	if (indent) {
		for (register int i = m_uhLineIndents * m_struOption.uhChPerInd; --i >= 0; )
			*str++ = m_struOption.chIndent; // 输出缩进
	}
	if (insertBlank) {
		*str++ = _T(' ');
	}
	// 输出内容
	const Char* const end = start + len;
	while (start < end) {
		*str++ = *start++;
	}
	if (!m_struOption.bNotPutCR)
		*str++ = _T('\r');
	*str++ = _T('\n');
	*str = 0;
	m_out->setLength(str - *m_out);
}

void RealJSFormatter::popMultiBlock(const Char previousStackTop) const {
	bool tmp = true;
	if (previousStackTop == JS_IF) {
		if (m_tokenB.equals(_T("else" ), 4)) { return; }
		tmp = false;
	}
	//else if (previousStackTop == JS_DO) {
	//	if (getTokenB().equals(_T("while"), 5)) { return; }
	//	tmp &= ~4;
	//}
	//else if (previousStackTop == JS_TRY) {
	//	if (m_tokenB.equals(_T("catch"), 5)) { return; }
	//	tmp &= ~1;
	//}
	// ; 还可能结束多个 if, do, while, for, try, catch
	// bool next = true;
	for (; ; ) {
		register Char topStack = stackTop();
		if (topStack > JS_OthersGt) {
			stackPop();
			continue;
		}
		else if (topStack > JS_IF_LE)
			break;
		else if (topStack == JS_IF) {
			if (tmp) {
				if (getTokenB().nequals(_T("else"), 4))
					tmp = false;
				else {
					--m_nIndents;
					stackPop();
					break;
				}
			}
		}
		else { // JS_FUNCTION, JS_TRY, JS_SWITCH不可能直接在栈里
		}
		//switch(stackTop()) {
		//case JS_IF:
		//	if (tmp & 2) {
		//		if (getTokenB().equals(_T("else"), 4)) { next = false; }
		//		else { tmp &= ~2; }
		//	}
		//	break;
		//case JS_FOR: case JS_WHILE: case JS_CATCH: break;
		//case JS_ELSE: break;
		//case JS_DO:
		//	if (tmp & 4) {
		//		if (getTokenB().equals(_T("while"), 5)) { next = false; }
		//		else { tmp &= ~4; }
		//	}
		//	break;
		//case JS_TRY:
		//	if (tmp & 1) {
		//		if (getTokenB().equals(_T("catch"), 5)) { next = false; }
		//		else { tmp &= ~1; }
		//	}
		//	break;
		//default: goto L_noMorePopBlock;
		//}
		--m_nIndents;
		stackPop();
	}
	// L_noMorePopBlock: ;
}

void RealJSFormatter::go() const {
	static const ProcessFuncPtr s_func[] = {
		&RealJSFormatter::processOper, &RealJSFormatter::processCommonToken,
		&RealJSFormatter::processID, &RealJSFormatter::processOrPutMultiLineString,
		&RealJSFormatter::processLineComment, &RealJSFormatter::processNewLineComment,
		&RealJSFormatter::processAndPutBlankLine, &RealJSFormatter::processAndPutBlockComment,
	};
	startParse();
	while (getToken()) {
		(this->*s_func[m_tokenA.more])();
	}
	endParse();
}

void RealJSFormatter::processCommonToken() const {
	putTokenA();
	m_line.more = (getTokenB().more != TOKEN_OPER) ? INSERT_SPACE : INSERT_UNKNOWN;
}

void RealJSFormatter::processOper() const {
	const int alen = m_tokenA.size();
	JsToken topStack = stackTop();

	if (alen <= 2) {
		if (m_tokenA.findIn(_T("()[]!~^.")) || m_tokenA.equals(_T('!'), _T('!'))) {
			// ()[]!. 都是前后没有样式的运算符
			const Char ach0 = m_tokenA[0];
			if (topStack == JS_WRAP && (ach0 == _T(')') || ach0 == _T(']'))) {
				--m_nIndents;
				stackPop();
				topStack = stackTop();
			}
			if ((topStack == JS_BRACKET && ach0 == _T(')')) ||
				(topStack == JS_SQUARE && ach0 == _T(']')))
			{ // )] 需要弹栈，减少缩进
				--m_nIndents;
				putTokenA();
				stackPop();
				topStack = stackTop();
				if (topStack == JS_BRACKET)
					++m_nIndents;
				else if (topStack == JS_WRAP) {
					// for (
					//     ...
					// ) {
					//     ...
					// }
					// NOTE: 视为正常缩进格式, 不修改
					stackPop();
					topStack = stackTop();
				}
			}
			else
				putTokenA();

			if (ach0 == _T(')') && topStack == JS_BRACKET_WANT) {
				// top2应该是 if, for, while, switch, catch, 正在等待 )，之后换行增加缩进
				m_line.more = INSERT_NEWLINE;
				stackPop();
				topStack = stackTop();
				if (topStack == JS_WHILE && stackTop2() == JS_DO) {
					stackPop();
					stackPop();
					popMultiBlock(JS_WHILE); // 结束 do...while
					topStack = stackTop();
					m_line.more = INSERT_UNKNOWN;
				}
				else {
					if (topStack != JS_FUNCTION || stackTop2() != JS_BRACKET) // "(function ... {"减掉一个缩进
						++m_nIndents;
					stackPush(JS_BRACKET_GOT);
				}
			}
			else if (ach0 == _T(')') && (getTokenB() == _T('{')))
				m_line.more = INSERT_SPACE; // { 或者换行之前留个空格
			else if (ach0 == _T('(') || ach0 == _T('[')) {
				// ([ 入栈，增加缩进
				// 更改/mod: "([...])"中减少一次缩进
				if (topStack == JS_WRAP)
					stackTop(ach0 == _T('(') ? JS_BRACKET : JS_SQUARE);
				else {
					if (topStack != JS_BRACKET)
						++m_nIndents;
					stackPush(ach0 == _T('(') ? JS_BRACKET : JS_SQUARE);
				}
				m_line.more = INSERT_UNKNOWN;
			}
			else
				m_line.more = INSERT_UNKNOWN;
			return;
		}
		if (alen == 1) {
			switch (m_tokenA[0]) {
			case _T(';'):
				if (topStack == JS_WRAP) {
					--m_nIndents;
					stackPop();
					topStack = stackTop();
				}
				if (topStack == JS_BRACKET_GOT || topStack == JS_BLOCK_VIRTUAL) {
					stackPop();
					topStack = stackTop();
				}
				// ; 结束 else, if, for, while, do
				if (topStack >= JS_WHILE_GE && topStack < JS_WRAP_Lt) {
					if (m_line.more == INSERT_NEWLINE) {
						m_line.more = INSERT_SPACE;
						--m_nIndents;
						putTokenA();
					}
					else {
						putTokenA();
						--m_nIndents;
					}
					if (topStack != JS_DO) { // do 在读取到 while 后才修改计数, 对于 do{} 也一样
						stackPop();
						popMultiBlock(topStack);
						topStack = stackTop();
					}
				}
				else
					putTokenA();
				// 如果不是"(...)"里的 ';' 就换行, 使用"_SHOULD"保证之后的 '{' 不会还在同一行
				m_line.more = (topStack != JS_BRACKET) ? INSERT_NEWLINE_SHOULD
					: INSERT_SPACE; // "(...; ...)" 空格
				return; // ;
			case _T(','):
				if (topStack == JS_WRAP) {
					--m_nIndents;
					stackPop();
					topStack = stackTop();
				}
				if (topStack == JS_BRACKET_GOT) {
					stackTop(JS_BLOCK_VIRTUAL);
					topStack = JS_BLOCK_VIRTUAL;
				}
				putTokenA();
				// 如果是 "{...}" 里的就换行
				m_line.more = topStack >= JS_BLOCK_GE ? INSERT_NEWLINE : INSERT_SPACE;
				return; // ,
			case _T('{'): 
				if (topStack == JS_WRAP) {
					--m_nIndents;
					stackPop();
					topStack = stackTop();
				}
				{
					INSERT_MODE eNextInsert = (getTokenB() == _T('}')) // 空 {}
						? INSERT_NONE : INSERT_NEWLINE_SHOULD;
					if (m_line.more == INSERT_NEWLINE) {
						if (m_struOption.bBracketAtNewLine)
							eNextInsert = INSERT_NEWLINE_SHOULD;
						else
							m_line.more = INSERT_SPACE;
					}
					
					if (topStack == JS_BRACKET_GOT) {
						stackTop(JS_BLOCK);
						topStack = stackTop2();
					}
					else
						stackPush(JS_BLOCK);
					if (topStack <= JS_BRACKET_LE) // 包含了对"({...})"中多一次缩进的修正
						--m_nIndents;
					putTokenA();
					++m_nIndents; // 入栈，增加缩进
					m_line.more = eNextInsert;
				}
				return; // {
			case _T('}'):
				// 激进的策略，} 一直弹到 {
				// 这样做至少可以使得 {} 之后是正确的
				while (topStack != JS_NULL) {
					stackPop();
					if (topStack <= JS_WRAP_LE)
						--m_nIndents;
					else if (topStack == JS_BLOCK)
						break;
					topStack = stackTop();
				}
				if (topStack == JS_BLOCK) {
					topStack = stackTop();
					// 由"(...)"和"{"三个符号的逻辑保证topStack不会是JS_WRAP
					if (topStack < JS_DO_Lt)
						stackPop(); // 缩进已经处理，do 留给 while
					--m_nIndents;
				}
				{
					const bool bSpace = (m_line.more != INSERT_NONE && m_line.more < INSERT_NULL)
						&& (m_line.more = INSERT_NEWLINE, !m_struOption.bBracketAtNewLine);
					putTokenA();
					if (getTokenB().more == TOKEN_OPER) {
						const Char chB = getTokenB()[0];
						if (getTokenB().size() == 1 && (chB == _T(';') || chB == _T(',') || chB == _T(')')))
						{ // 更改/mod: 已经统一使(...{...})的{}前后不换行
							m_line.more = INSERT_UNKNOWN; // }, }; })
						}
						else
							m_line.more = INSERT_NEWLINE;
					}
					else if (getTokenB().more == TOKEN_ID && bSpace && (
						(topStack == JS_IF  && getTokenB().equals(_T("else"),  4)) ||
						(topStack == JS_DO  && getTokenB().equals(_T("while"), 5)) ||
						(topStack == JS_TRY && ( getTokenB().equals(_T("catch"), 5) ||
							getTokenB().equals(_T("finally"), 7) )) ))
					{
						m_line.more = INSERT_SPACE;
					}
					else
						m_line.more = INSERT_NEWLINE;
					// 测试发现: JS语法限制"{xxx}.xxx"总是不合法的, 无论"{xxx}"是词典还是语句块
				}
				if (stackTopEq(JS_BRACKET))
					++m_nIndents;
				if (getTokenB() != _T(';')) // 如果 m_tokenB 是 ;，弹出多个块的任务留给它
					popMultiBlock(topStack);
				return; // }
			case _T(':'): {
					const JsToken oldTop = topStack;
					if (topStack == JS_WRAP)
						topStack = stackTop2();
					if ((topStack < JS_CASE_Lt && m_line.nempty()) || topStack == JS_NULL)
						m_line.more = INSERT_SPACE;
					putTokenA();
					if (oldTop == JS_WRAP) {
						stackPop();
						--m_nIndents;
					}
				}
				if (topStack == JS_CASE) { // case, default
					++m_nIndents;
					stackPop();
					m_line.more = INSERT_NEWLINE;
				}
				else {
					if (topStack == JS_BRACKET_GOT)
						stackTop(JS_BLOCK_VIRTUAL);
					m_line.more = INSERT_SPACE;
				}
				return;
			}
		}
		else {
			const Char ach0 = m_tokenA[0];
			const Char ach1 = m_tokenA[1];
			if ((ach0 == _T('+') && ach1 == _T('+')) || (ach0 == _T('-') && ach1 == _T('-'))
			//||(ach0 ==  _T(':') && ach1 ==  _T(':')) || (ach0 ==  _T('-') && ach1 ==  _T('>'))
			) {
				if (topStack == JS_BRACKET_GOT)
					stackTop(JS_BLOCK_VIRTUAL);
				putTokenA();
				m_line.more = INSERT_UNKNOWN;
				return;
			}
		}
	}
	if (topStack == JS_BRACKET_GOT)
		stackTop(JS_BLOCK_VIRTUAL);

	if (m_line.more < INSERT_SPACE)
		m_line.more = INSERT_SPACE;
	putTokenA(); // 剩余的操作符都是 空格oper空格
	m_line.more = INSERT_SPACE;
}

void RealJSFormatter::processID() const {
	JS_TOKEN_TYPE token_type1 = JS_NULL;
	if (m_tokenA.equals(_T("else"), 4)) {
		if (getTokenB().equals(_T('i'), _T('f'))) {
			token_type1 = JS_BLOCK;
			goto L_notKeyword; // else if 时不压栈
		}
		token_type1 = JS_ELSE;
	}
	else if (m_tokenA.equals(_T('d'), _T('o')))
		token_type1 = JS_DO;
	else if (m_tokenA.equals(_T("try"), 3))
		token_type1 = JS_TRY;
	else if (m_tokenA.equals(_T("finally"), 7))
		token_type1 = JS_TRY;
	else {
		if (m_tokenA.equals(_T("function"), 8))
			token_type1 = JS_FUNCTION;
		else if (m_tokenA.equals(_T("case"), 4) || m_tokenA.equals(_T("default"), 7)) {
			// case, default 往里面缩一格
			if (m_line.more == INSERT_NEWLINE)
				m_line.more = INSERT_SPACE;
			else if (m_line.more < INSERT_NULL)
				m_line.more = INSERT_NEWLINE;
			if (stackTopEq(JS_WRAP)) {
				stackTop(JS_CASE);
				--m_nIndents;
			}
			else
				stackPush(JS_CASE);
			--m_nIndents;
			putTokenA();
			m_line.more = (m_tokenA[0] != _T('d')) ? INSERT_SPACE : INSERT_UNKNOWN;
			return;
		}
		else
			token_type1 = findSomeKeyword();
		if (token_type1 <= JS_IF_LE) {
			if (token_type1 == JS_IF && stackTopEq(JS_ELSE)) {
				stackTop(JS_IF);
				--m_nIndents;
			}
			else if (stackTopEq(JS_WRAP))
				stackTop(token_type1);
			else
				stackPush(token_type1);
			stackPush(JS_BRACKET_WANT);
		}
		else if (stackTopEq(JS_BRACKET_GOT))
			stackTop(JS_BLOCK_VIRTUAL);

		L_notKeyword:
		putTokenA();
		const Token& tokenB = getTokenB();
		m_line.more = ((tokenB.more != TOKEN_OPER || tokenB == _T('{')) // '{': such as "return {'a':1};"
			|| (tokenB != _T(';') && token_type1 != JS_NULL)
			) ? INSERT_SPACE : INSERT_UNKNOWN;
		return;
	}
	if (stackTopEq(JS_WRAP)) {
		--m_nIndents;
		stackTop(token_type1);
	}
	else
		stackPush(token_type1);
	// do, else (EXCEPT else if), try
	putTokenA();
	++m_nIndents; // 无需 ()，直接缩进
	m_line.more = (getTokenB().more != TOKEN_OPER || m_struOption.bBracketAtNewLine)
		? INSERT_NEWLINE : INSERT_SPACE;
}

void RealJSFormatter::processOrPutMultiLineString() const {
	const Char *const end = m_tokenA.c_end();
	const Char *str0 = m_tokenA;
	//if (m_tokenA.more == TOKEN_STRING)
	//	putTokenA();
	//else {
	getOutput().expand(getOutput().size() + m_tokenA.size() + 16);
	for (register const Char *str = str0; ; ) {
		++str;
		if (*str == '\r' || *str == '\n') {
			str0 = str;
			break;
		}
	}
	putTokenRaw(m_tokenA, str0 - m_tokenA);
	if (*str0 == _T('\r'))
		++str0;
	if (end > str0 && *str0 == _T('\n'))
		++str0;
	if (end > str0)
		putBufferLine();
	for (register const Char *str = str0; str < end; ) {
		register const Char *str2 = str - 1;
		while(++str2 < end && *str2 != '\r' && *str2 != '\n') {}
		if (str2 < end) {
			getOutput().addLine(str, str2, m_struOption.bNotPutCR ? 1 : 3);
			str = str2;
			if (*str == _T('\r'))
				++str;
			if (end > str && *str == _T('\n'))
				++str;
		}
		else {
			m_line.addStr(str, str2 - str);
			m_uhLineIndents = 0;
			break;
		}
	}
	//}
	//if (m_tokenB.more == TOKEN_NULL) {
	//	const Char chQuote = m_tokenA[0];
	//	register const Char *str = str0;
	//	for (; str < end && *str != chQuote; ) {
	//		if (*str++ == _T('\\'))
	//			++str;
	//	}
	//	if (str >= end) { // an error end of js document
	//		if (str > end) // end with single _T('\\')
	//			m_line.addOrDouble(_T('\\'));
	//		m_line.addOrDouble(chQuote);
	//	}
	//}

	// 更改/warn: 如果'.'算进normal_char, 则"."就算不是TOKEN_OPER类型, 此处需要判断'.', 
	m_line.more = (m_tokenB.more != TOKEN_OPER) ? INSERT_SPACE : INSERT_UNKNOWN;
}

//void RealJSFormatter::processRegular() const {
//	putTokenA(); // 正则表达式直接输出，前后没有任何样式
//	if (m_tokenB.more == TOKEN_NULL) {
//		const Char *const end = m_tokenA.c_end();
//		register const Char *str = m_tokenA;
//		for (; str < end && *str != _T('/'); ) {
//			if (*str++ == _T('\\'))
//				++str;
//		}
//		if (str >= end) { // an error end of js document
//			if (str > end) // end with single _T('\\')
//				m_line.addOrDouble(_T('\\'));
//			m_line.addOrDouble(_T('/'));
//		}
//	}
//	m_line.more = (m_tokenB.more != TOKEN_OPER) ? INSERT_SPACE : INSERT_UNKNOWN;
//}

void RealJSFormatter::processLineComment() const {
	if (m_line.more < INSERT_NULL) {
		if (m_line.more < INSERT_NEWLINE && stackTop() > JS_WRAP_LE) {
			const Char ch = m_line[m_line.size() - 1];
			if (ch != _T('(') && ch != _T(',') && ch != _T(';')) {
				stackPush(JS_WRAP);
				++m_nIndents;
			} 
		}
		m_line.more = INSERT_SPACE;
	}
	putTokenA();
	putBufferLine();
	m_line.more = INSERT_NULL;
}

void RealJSFormatter::processNewLineComment() const {
	if (m_line.more < INSERT_NULL)
		m_line.more = INSERT_NEWLINE;
	putTokenA();
	putBufferLine();
	m_line.more = INSERT_NULL;
}

void RealJSFormatter::processAndPutBlankLine() const {
	if (m_line.more < INSERT_NULL)
		putBufferLine();
	putLine(NULL, 0, 0); // 输出空行
	m_line.more = INSERT_NULL;
}

void RealJSFormatter::processAndPutBlockComment() const {
	const Char *const end = m_tokenA.c_end();
	if (m_line.more <= INSERT_NEWLINE) {
		register const Char *str = m_tokenA;
		while(end > ++str && *str != _T('\r') && *str != _T('\n')) {}
		if (str == end) {
			m_line.more = INSERT_SPACE; // 不用判断 m_line.nempty()
			putTokenA();
			//if (str[-2] != _T('*') || str[-1] != _T('/')) {
			//	if (str[-1] != '*')
			//		m_line.addOrDouble(_T('*'));
			//	m_line.addOrDouble(_T('/'));
			//}
			return;
		}
		m_line.more = INSERT_NEWLINE;
	}
	m_out->expand(m_out->size() + m_tokenA.size() + m_line.size() + 80);
	putTokenRaw(NULL, 0);
	
	for (register const Char *str = m_tokenA; ; ) {
		while (str < end && *str == _T('\t') || *str == _T(' '))
			++str;
		register const Char *str2 = str;
		while(str2 < end && *str2 != '\r' && *str2 != '\n')
			++str2;
		if (str2 < end) {
			putLine(str, str2 - str, *str == _T('*'));
			str = str2;
			if (*str == _T('\r'))
				++str;
			if (*str == _T('\n'))
				++str;
			continue;
		}
		if (str < str2)
			putLine(str, str2 - str, *str == _T('*'));
		//if (str2[-2] != _T('*') || str2[-1] != _T('/'))
		//	putLine(_T("*/"), 2, true);
		break;
	}
	m_line.more = INSERT_NULL;
}
