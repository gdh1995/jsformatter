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
//		if (!m_debug)
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

/**
 * ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
 *  (ch >= 'A' && ch <= 'Z') || ch == '_' || ch == '$' ||
 *   ch == '.' || ch  > 126  || ch < 0)
 */
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

/**
 * IsNormalChar(ch) || IsBlankChar(ch) || IsQuote(ch)
 */
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

// 更改/mod: ":"被识别为单字符符号, 因为"::"的情况被放弃支持
/**
 * (ch == '(' || ch == ')' ||
 *  ch == '[' || ch == ']' || ch == '{' || ch == '}' ||
 *  ch == ',' || ch == ';' || ch == '~' || ch == ':')
 */
const JSParser::Byte JSParser::s_singleOperMap[128] = {
	/*  + x: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
	/* 0+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 1+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 2+ */ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0,
	/* 3+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
	/* 4+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 5+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0,
	/* 6+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 7+ */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0
};

void JSParser::GetTokenRaw(int bFlag) const {
	const Char * start1;

	if (bFlag & BoolFlag::PosNeg)
		start1 = CurPos() - 1;
	else {
		start1 = CurPos();
		register Char chB = LastChar();
		for (; chB == _T(' ') || chB == _T('\t'); chB = GetChar()) {}
		if (chB == _T('\r') || chB == _T('\n')) {
			if (chB == _T('\r'))
				chB = GetChar();
			if (chB == _T('\n'))
				chB = GetChar();
			for (; chB == _T(' ') || chB == _T('\t'); chB = GetChar()) {}
			if (chB == _T('\r') || chB == _T('\n')) {
				// 更改/mod: 补回连续换行的处理逻辑
				for (; chB = GetChar(), IsBlankChar(chB); ) {}
				m_tokenB.setData(const_cast<Char *>(start1 - 1));
				m_tokenB.setLength(CurPos() - start1);
				m_tokenB.more = TOKEN_BLANK_LINE;
				return;
			}
		}
		start1 = CurPos();
	}
	const Char charA = LastChar();
	if (charA == 0) {
		m_tokenB.setData(_T(""));
		m_tokenB.setLength(0);
		m_tokenB.more = TOKEN_NULL;
		return;
	}
	Char charB = GetChar();
	if (IsNormalChar(charA)) {
		// 更改/mod: 将_T('.')视为单词的一部分
		// 如果要改回, 需要大范围修改若干地方, 不建议
		if (charA >= _T('0') && charA <= _T('9')) {
			register Char chB = charB;
			for (; (chB >= _T('0') && chB <= _T('9')) || chB == _T('.'); chB = GetChar()) {}
			charB = chB;
		}
		else if (charA == _T('.') && charB >= _T('0') && charB <= _T('9')) {
			register Char chB;
			for (; chB = GetChar(), chB >= _T('0') && chB <= _T('9'); ) {}
			charB = chB;
		}
		else {
			register Char chB = charB;
			for (; chB >= _T('a') && chB <= ('z'); chB = GetChar()) {}
			if (IsNormalChar(chB)) {//TODO:
				m_tokenB.more = TOKEN_COMMON;
				for (; chB = GetChar(), IsNormalChar(chB); ) {}
			}
			else
				m_tokenB.more = TOKEN_ID;
			goto L_notNumber;
		}
		if ((charB | _T('\x20')) == _T('e')) { // 解决类似 82.e-2, 44.2e+6, 555E6, .32e5 的问题
			register Char chB = GetChar();
			if (chB == _T('-') || chB == _T('+'))
				chB = GetChar();
			for (; chB >= _T('0') && chB <= _T('9'); chB = GetChar()) {}
#ifdef _DEBUG
			if (!IsNormalChar(chB))
				throw chB;
#endif
		}
		else { // if ((m_charB | _T('\x20')) == _T('x')) {
			// 不需要区分 0x7F 和 123434; , 因为 x 和 oper 直接用IsNormalChar()即可区分
			register Char chB = charB;
			for (; IsNormalChar(chB); chB = GetChar()) {}
		}
		m_tokenB.more = TOKEN_COMMON;

		L_notNumber: ;
	}
	else if (IsQuote(charA)) { // 引号
		const Char chQuote = charA;
		register Char chB = charB;
		for (; chB && chB != chQuote; chB = GetChar()) {
			if (chB != _T('\\')) // 转义字符
				continue;
			chB = GetChar();
			if (chB == _T('\r') || chB == _T('\n'))
				goto L_whenMultiLineString;
		}
		m_tokenB.more = TOKEN_STRING;
		goto L_finishReadString;

		L_whenMultiLineString:
		{
			for (; chB && chB != chQuote; ) { // 引号状态，全部输出，直到引号结束
				if (chB == _T('\\')) { // 转义字符
					chB = GetChar();
					if (chB == '\r')
						chB = GetChar();
					if (chB != '\n')
						continue;
				}
				chB = GetChar();
			}
			m_tokenB.more = TOKEN_STRING_MULTI_LINE;
		}

		L_finishReadString:
		GetChar();
	}
	else if (charA == _T('/') && (charB == _T('/') || charB == _T('*'))) { // 注释
		if (charB == _T('/')) { // 直到换行
			m_tokenB.more = TOKEN_COMMENT_LINE;
			for (register Char chB; (chB = GetChar()) && chB != _T('\r') && chB != _T('\n'); ) {}
		}
		else { // 直到 */
			m_tokenB.more = TOKEN_COMMENT_BLOCK;
			for (register Char chB = GetChar(); chB; ) {
				register Char chA = chB;
				chB = GetChar();
				if (chA == '*' && chB == '/') {
					break;
				}
			}
			GetChar();
		}
	}
	else if (IsSingleOper(charA) || IsSingleOperNextChar(charB)) { // 单字符符号
		m_tokenB.more = TOKEN_OPER;
	}
	else if (charB == _T('=') || charB == charA) { // 多字符符号
		m_tokenB.more = TOKEN_OPER;
		register Char charC = GetChar();
		if (charC == _T('=')) {
			if ( charB == _T('<') || charB == _T('>') || charA == _T('=') ||
				(charA == _T('!') && charB == _T('=')) )
			{ // 三字符 ===, !==, <<=, >>=
				GetChar();
			}
		}
		else if (charC == _T('>') && charB == _T('>')) {
			if (GetChar() == _T('=')) // >>>=
				GetChar();
		}
	}
	// 更改/mod: ECMAScript的操作符列表不包含"->", 所以屏蔽之
	//else if ((charA == _T('-') && m_charB == _T('>'))) {
	//m_charB = GetChar();
	//}
	else { // 还是单字符的
		m_tokenB.more = TOKEN_OPER;
	}

	m_tokenB.setData(const_cast<Char *>(start1 - 1));
	m_tokenB.setLength(CurPos() - start1);
}

void JSParser::PrepareTokenB() const
{
	if (m_tokenB.more != TOKEN_OPER || m_tokenB.empty())
		return;

	switch (m_tokenB[0]) {
	case _T('/'):
		// 更改/mod: 放开注释的限制
		// 更改/warn: 默认m_tokenA.more == OPER_TOKEN时, m_tokenA不为空
		//     需要m_tokenA不被赋值给其它Token对象 + OPER_TOKEN不是.more的默认值
		/*
		* 先处理一下正则
		* m_tokenB[0] == /，且 m_tokenB 不是注释
		* m_tokenA 不是 STRING (除了 m_tokenA == return)
		* 而且 m_tokenA 的最后一个字符是下面这些
		*/
		if (m_tokenA.more >= TOKEN_COMMENT_LINE || (m_tokenA.more == TOKEN_OPER &&
			ConstString::findIn(m_tokenA[m_tokenA.size() - 1], s_operCharBeforeReg)) ||
			m_tokenA.equals(_T("return"), 6) ) 
		{
			register Char chB = LastChar();
			//	//	// TODO: what are '*' and '|'
			//	//	if (chB != _T('*') && chB != _T('|')) { // 正则可能结束
			for (; chB; chB = GetChar()) { // 正则状态全部输出，直到 /
				if (chB == _T('\\')) // 转义字符
					GetChar();
				else if (chB == _T('/')) {
					chB = GetChar();
					break;
				}
			}
			// 备注/info: 也可以只识别小写"g", "i", "m", 但用IsNormalChar可以产生更长的Token
			while (IsNormalChar(chB)) // 正则的 flags 部分
				chB = GetChar();
			m_tokenB.setLength(CurPos() - m_tokenB.c_str() - 1);
			m_tokenB.more = TOKEN_REGULAR;
		}
		break;
	case _T('-'): case _T('+'):
		if (m_tokenB.size() != 1) {
			return;
		}
		// 更改/mod: 强制 m_tokenA 为 TOKEN_OPER 或者 return
		/*
		* 如果 m_tokenB 是 -,+ 号
		* 而且 m_tokenA 不是字符串型也不是正则表达式
		* 而且 m_tokenA 不是 ++, --, ], )
		* 而且 m_charB 是一个 NormalChar
		* 那么 m_tokenB 实际上是一个正负数
		*/
		if ( IsNormalChar(LastChar()) && ( (m_tokenA.more == TOKEN_OPER && m_tokenA != _T(']') &&
			m_tokenA != _T(')') && m_tokenA.nequals(_T('+'), _T('+')) &&
			m_tokenA.nequals(_T('-'), _T('-'))) || m_tokenA.equals(_T("return"), 6) ) )
		{
			// m_tokenB 实际上是正负数
			GetTokenRaw(BoolFlag::PosNeg);
		}
		break;
	default:
		break;
	}
}
