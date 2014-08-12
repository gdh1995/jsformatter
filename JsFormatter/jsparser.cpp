/* jsparser.cpp
2012-3-11
Version: 0.9.8

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
#include "alldefs.h"
#include "jsparser.h"

// 更改/mod: 除去'\n', 因为OPER_TOKEN下属字符串不会包含'\n'
const JSParser::Char JSParser::s_operCharBeforeReg[] = _T("(,=:[!&|?+{};");

//	void JSParser::PrintDebug()
//	{
//		if(!m_debug)
//		{
//			return;
//		}
//		_snprintf(m_debugOutput, sizeof(m_debugOutput) / sizeof(m_debugOutput[0])
//			, "%s%d\n" // "%s%d%s%.3fs\n%.3f%s"
//			, "Processed tokens: ", m_tokenCount
//			// , "\nTime used: ", m_duration
//			// , m_tokenCount / m_duration, " tokens/second\n"
//			);
//		printf("%s", m_debugOutput);
//	}

//	static const JSParser::Byte s_normalMap_0[128] = {
//		/*  + x: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
//		/* 0+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		/* 1+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		/* 2+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		/* 3+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		/* 4+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		/* 5+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		/* 6+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		/* 7+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
//	};

const JSParser::Byte JSParser::s_normalCharMap[128] = {
	/*  + x: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
	/* 0+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 1+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 2+ */ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, // '.' == 0x2E;
	/* 3+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	/* 4+ */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 5+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
	/* 6+ */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 7+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1
};

const JSParser::Byte JSParser::s_singleOperNextCharMap[128] = {
	/*  + x: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
	/* 0+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,
	/* 1+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 2+ */ 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 3+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	/* 4+ */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 5+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
	/* 6+ */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 7+ */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1
};

const JSParser::Byte JSParser::s_singleOperMap[128] = {
	/*  + x: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
	/* 0+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 1+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 2+ */ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0,
	/* 3+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
	/* 4+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 5+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0,
	/* 6+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 7+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0
};

void JSParser::GetTokenRaw() const
{
	m_tokenB.more = OPER_TOKEN;
	const Char * start0;
	size_t len0 = 0; // 0 means "auto calc the size"

	if(m_bFlag & BoolFlag::PosNeg) {
		start0 = CurPos() - 2;
	}
	else {
		register Char chB = m_charB;
		while (IsBlankChar(chB)) {
			chB = GetChar();
		}
		m_charB = chB;
		start0 = CurPos() - 1;
	}
	const Char charA = m_charB;
	m_charB = GetChar();
	if (IsNormalChar(charA)) {
		// 更改/mod: 将_T('.')视为单词的一部分,
		// 如果要改回, 只需修改s_normalCharMap[0x2E] = 0;
		m_tokenB.more = STRING_TOKEN;
		register Char chB = m_charB;
		if (IsNumChar(charA)) {
			while (IsNumChar(chB)) {
				chB = GetChar(); // loop until m_charB is not a normal character
			}
			if ((chB | _T('\x20')) == _T('e')) {
				chB = GetChar();
				// 解决类似 82e-2, 442e+6, 555E-6 的问题
				if (chB == _T('-') || chB == _T('+')) {
					chB = GetChar();
				}
			}
		}
		while (IsNormalChar(chB)) {
			chB = GetChar(); // loop until m_charB is not a normal character
		}
		m_charB = chB;
	}
	else if (IsQuote(charA)) { // 引号
		m_tokenB.more = STRING_TOKEN;
		const Char chQuote = charA;
		register Char chB = m_charB;
		for(; chB; chB = GetChar()) { // 引号状态，全部输出，直到引号结束
			if (chB == chQuote) {
				chB = GetChar();
				break;
			}
			if (chB == _T('\\')) { // 转义字符
				GetChar();
			}
		}
		m_charB = chB;
	}
	else if (charA == _T('/') && (m_charB == _T('/') || m_charB == _T('*'))) { // 注释
		if (m_charB == _T('/')) { // 直到换行
			m_tokenB.more = COMMENT_1_TOKEN;
			len0 = 2;
			register Char chB;
			for (; chB = GetChar(); ) {
				if (chB == '\n') {
					len0 = CurPos() - start0 - 1;
					chB = GetChar();
					break;
				}
				else if(chB == _T('\r')) {
					len0 = CurPos() - start0 - 1;
					chB = GetChar();
					if (chB == _T('\n')) {
						chB = GetChar();
					}
					break;
				}
			}
			m_charB = chB;
		}
		else { // 直到 */
			m_tokenB.more = COMMENT_2_TOKEN;
			len0 = 2;
			{
				register Char chB;
				for (; chB = GetChar(); ) {
					if (chB == '\n') {
						len0 = CurPos() - start0 - 1;
						chB = GetChar();
						break;
					}
					else if(chB == _T('\r')) {
						len0 = CurPos() - start0 - 1;
						chB = GetChar();
						if (chB == _T('\n')) { chB = GetChar(); }
						break;
					}
					else if (chB == '*') {
						chB = GetChar();
						if (chB == '/') { chB = GetChar(); }
						else if (chB) { continue; }
						len0 = 0;
						break;
					}
				}
				m_charB = chB;
			}
			if (len0) {
				m_tokenB.reset(len0 + 4);
				m_tokenB.copyFrom(start0, len0);
				m_tokenB.addOrDouble(_T('\n'));
				bool bLineBegin = true;
				register Char chB = m_charB;
				for (; chB; ) {
					if(bLineBegin) {
						if(chB == _T('\t') || chB == _T(' ')) {
							chB = GetChar();
							continue; // /*...*/每行前面的_T('\t'), _T(' ')都要删掉
						}
						bLineBegin = false;
						if (chB == _T('*')) {
							m_tokenB.addOrDouble(_T(' '));
						}
					}
					if(chB == _T('\r')) {
						bLineBegin = true;
						m_tokenB.addOrDouble(_T('\n'));
						chB = GetChar();
						if (chB == _T('\n'))
							chB = GetChar();
						continue;
					} else if (chB == _T('\n')) {
						bLineBegin = true;
					}
					m_tokenB.addOrDouble(chB);
					if(chB == _T('*')) {
						chB = GetChar();
						if (chB == _T('/')) {
							m_tokenB.addOrDouble(chB);
							chB = GetChar();
							break;
						}
						continue;
					}
					chB = GetChar();
				}

				m_charB = chB;
				return;
			}
		}
	}
	else if(IsSingleOper(charA) || IsSingleOperNextChar(m_charB)) // 单字符符号
	{
	}
	else if(m_charB == _T('=') || m_charB == charA) // 多字符符号
	{
		register Char charB = m_charB;
		m_charB = GetChar();
		if (m_charB == _T('=')) {
			if (( charB == _T('<') && charA == _T('<') ) ||
				( charB == _T('>') && charA == _T('>') ) ||
				( charB == _T('=') && (charA == _T('=') || charA == _T('!')) ))
			{ // 三字符 ===, !==, <<=, >>=
				m_charB = GetChar();
			}
		}
		else if (m_charB == _T('>') && charB == _T('>') && charA == _T('>')) {
			m_charB = GetChar();
			if(m_charB == _T('=')) { // >>>=
				m_charB = GetChar();
			} else { // >>>
			}
		}
	}
	// 更改/mod: ECMAScript的操作符列表不包含"->", 所以屏蔽之
	//else if ((charA == _T('-') && m_charB == _T('>'))) {
	//m_charB = GetChar();
	//}
	else { // 还是单字符的
	}
	if (m_charB == 0) {
		m_charB = 0; // TODO: check the last
	}
	m_tokenB.setData(const_cast<Char *>(start0));
	m_tokenB.setLength(len0 ? len0 : (CurPos() - start0 - 1));
	m_tokenB.doNotFree();
}

bool JSParser::GetToken()
{
	//	if(!m_bGetTokenInit)
	//	{
	//		// 第一次多调用一次 GetTokenRaw
	//		GetTokenRaw();
	//		m_bGetTokenInit = true;
	//	}

	PrepareRegular(); // 判断正则
	PreparePosNeg(); // 判断正负数

	// ++m_tokenCount;
	m_tokenA = m_tokenB;

	// 更改/mod: .size() == 0恒成立
	//if(m_tokenBQueue.size() == 0)
	{
		GetTokenRaw();
		// PrepareTokenB(); // 看看是不是要跳过换行
	}
	//else
	//{
	//	// 有排队的换行
	//	m_tokenB = m_tokenBQueue.front();
	//	m_tokenBQueue.pop();
	//}

	return m_tokenA.length() != 0;
}

void JSParser::PrepareRegular() const
{
	// 更改/mod: 放开注释的限制
	// 更改/warn: 默认m_tokenA.more == OPER_TOKEN时, m_tokenA不为空
	// >>> 需要m_tokenA不被赋值给其它Token对象 + OPER_TOKEN不是.more的默认值
	/*
	* 先处理一下正则
	* m_tokenB[0] == /，且 m_tokenB 不是注释
	* m_tokenA 不是 STRING (除了 m_tokenA == return)
	* 而且 m_tokenA 的最后一个字符是下面这些
	*/
	if (m_tokenB.more == OPER_TOKEN  && m_tokenB[0] == _T('/') && (
		(m_tokenA.more == OPER_TOKEN && CharString::findIn(m_tokenA[m_tokenA.size() - 1], s_operCharBeforeReg)) ||
		m_tokenA.more >= COMMENT_1_TOKEN || m_tokenA.equals(_T("return"), 6) ) )
	{
		register Char chB = m_charB;
		//	//	// TODO: what are '*' and '|'
		//	//	if (chB != _T('*') && chB != _T('|')) { // 正则可能结束
		for (; chB; chB = GetChar()) { // 正则状态全部输出，直到 /
			if (chB == _T('\\')) { // 转义字符
				GetChar();
			}
			else if (chB == _T('/')) {
				chB = GetChar();
				break;
			}
		}
		// 备注/info: 也可以只识别小写"g", "i", "m", 但用IsNormalChar可以产生更长的Token
		while (IsNormalChar(chB)) { // 正则的 flags 部分
			chB = GetChar();
		}
		m_charB = chB;
		m_tokenB.setLength(CurPos() - m_tokenB.c_str() - 1);
		m_tokenB.doNotFree();
		m_tokenB.more = REGULAR_TOKEN;
	}
}

void JSParser::PreparePosNeg()  const
{
	// 更改/mod: 强制 m_tokenA 为 OPER_TOKEN 或者 return
	/*
	* 如果 m_tokenB 是 -,+ 号
	* 而且 m_tokenA 不是字符串型也不是正则表达式
	* 而且 m_tokenA 不是 ++, --, ], )
	* 而且 m_charB 是一个 NormalChar
	* 那么 m_tokenB 实际上是一个正负数
	*/
	if(m_tokenB.more == OPER_TOKEN && (m_tokenB == _T('-') || m_tokenB == _T('+')) &&
		((m_tokenA.more == OPER_TOKEN && m_tokenA != _T(']') && m_tokenA != _T(')') &&
		m_tokenA.nequals(_T('+'), _T('+')) && m_tokenA.nequals(_T('-'), _T('-'))) || 
		m_tokenA.equals(_T("return"), 6)) &&
		IsNormalChar(m_charB))
	{
		// m_tokenB 实际上是正负数
		m_bFlag |= BoolFlag::PosNeg;
		GetTokenRaw();
		m_bFlag &= ~(BoolFlag::PosNeg);
	}
}

//void JSParser::PrepareTokenB()
//{
//	// 更改/mod: 注释掉while, 因为GetTokenRaw已经直接跳过\r\n
//	/*
//	* 跳过 else, while, catch, ',', ';', ')', '{' 之前的换行
//	* 如果最后读到的不是上面那几个，再把去掉的换行补上
//	*/
//	//int c = 0;
//	//while (m_tokenB == _T('\n')) {
//	//	++c;
//	//	GetTokenRaw();
//	//}
//
//	if ( (m_tokenB != _T(',') && m_tokenB != _T(')') && m_tokenB != _T(';')) &&
//		m_tokenB.nequals(_T("else"), 4) && m_tokenB.nequals(_T("while"), 5) && m_tokenB.nequals(_T("catch"), 5) )
//	{
//		// 将去掉的换行压入队列，先处理 空{}
//		if(m_tokenA == _T('{') && m_tokenB == _T('}'))
//			return;
//
//		//for(c = c > 2 ? 2 : c; c > 0; --c) {
//		//	m_tokenBQueue.push(CharString(_T("\n"), 1, 2, 1, OPER_TOKEN));
//		//}
//		if (m_tokenBQueue.size() == 0) {
//			return;
//		}
//		// 更改/mod: 测试发现完全执行不到这儿
//		//m_tokenBQueue.push(m_tokenB);
//		//m_tokenB = m_tokenBQueue.front();
//		//m_tokenBQueue.pop();
//	}
//}
