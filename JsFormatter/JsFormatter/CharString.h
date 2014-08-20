#ifndef _GDH_CHAR_STRING_
#define _GDH_CHAR_STRING_

#include <malloc.h>
#include "ConstString.h"

// 所有用做返回值的pos的起点均为1, 不存在记0
namespace KMP
{

	template <typename ttype>
	void get_next(ttype* str, UInt len, int nextval[])
	{
		nextval[0] = -1;
		for (int i = 1; i < len; i++)
		{
			int j = nextval[i - 1];
			while (str[i] != str[j+1] && j > -1)
				j = nextval[j];
			if (str[i] == str[j+1])
				nextval[i] = j+1;
			else
				nextval[i] = 0;
		}
		// nextval[1] = 0;
		// str--;
		// register UInt i = 1, j = 0;
		// while(i < len)
		// {
		//   if(j == 0 || str[i] == str[j])
		//   {
		//     ++i;
		//     ++j;
		//     // 测试发现以下两种方式均不能用(计算错误)，只好换成直接比较;
		//     //*/
		//     nextval[i] = j;/*/
		//     if(str[i] != str[j])
		//       nextval[i] = j;
		//     else
		//       nextval[i] = nextval[j];*/
		//   }
		//   else
		//     j = nextval[j];
		// }
	}

	template <typename ttype>
	UInt index_kmp(ttype* s, ttype* t, UInt lens, UInt lent, int next[], UInt pos2)
	{
		register int Lindex = pos2 - 1;
		register int pos = 0;
		while(lens > ++Lindex)
		{
			while (pos >= 0 && t[pos+1] != s[Lindex])
				pos = next[pos];
			if (t[pos + 1] == s[Lindex])
				pos++;
			if (pos == lent - 1)
				break;
		}
		if (pos== lent - 1)
			return (Lindex - pos + 1);
		else
			return -1;
		/*s--; t--;
		register UInt i = pos, j = 0;
		while(i < lens && j < lent)
		{
		if((0 == j) || (s[i] == t[j])) { ++i; ++j; }
		else j = next[j];
		}
		return (j > lent) ? (i - lent) : 0;*/
	}
};

template <typename ttype>
class CharString: public ConstString<ttype>
{
protected:
	UInt m_capacity;
	static const UInt sc_step_autoalloc = (4096 / sizeof(ttype)) > 4
		? (4096 / sizeof(ttype)) : 4;

//*
private:
	void operator = (const CharString& ori);
/*/
public:
	// 一般使用 右值构造函数拷贝后右值赋值，它本身可以用成员函数copy_from(const CharString&);代替
	inline void operator = (const CharString& ori);
//*/

public:
	typedef ConstString<ttype> BaseString;

	explicit inline CharString() {
		m_data = NULL; m_length = 0; m_flag = 0; more = 0; m_capacity = 0;
	}
	explicit inline CharString(ttype* ori, UInt len, UInt capa, short flag = 0, short more = 0) {
		m_data = ori; m_length = len; m_flag = flag; more = more; m_capacity = capa;
	}
	explicit CharString(const ttype* ori);
	// copy the value
	explicit CharString(const ttype* ori, const ttype* end);
	explicit CharString(const CharString& ori);
	explicit CharString(CharString&& ori);
	inline ~ CharString() {
		if(m_data && !m_flag)
			free(m_data);
#ifdef _DEBUG
		m_data = NULL; // remove 'm_data = NULL' to speed up
#endif
	}

public:
	using BaseString::c_str;
	inline ttype* c_str() { return m_data; }
	
	// Attention: below are some dangerous functions
	inline void reset(UInt size) {
		if(size > m_capacity || m_flag) {
			if (m_data && !m_flag) {
				free(m_data);
			}
			m_data = (ttype*)malloc(sizeof(ttype) * size);
			m_capacity = size;
			m_flag = 0;
		}
		m_length = 0;
	}
	protected:inline void expand0(UInt size) {
		m_data = (ttype*)realloc(m_data, sizeof(ttype) * size);
		m_capacity = size;
	}
	public:
	inline void expand(UInt size) {
		if(size > m_capacity) {
#ifdef _DEBUG
			if (m_flag)
				throw m_flag;
#endif
			m_data = (ttype*)realloc(m_data, sizeof(ttype) * size);
			m_capacity = size;
		}
	}
	inline void reserve(UInt size) { expand(size); }
	inline UInt capacity() const { return m_capacity; }
	
	void operator = (CharString&& ori);
	// 使用“右值构造”和“右值赋值”还不能完全代替它，因为要有减少内存分配的考虑;
	void copyFrom(const CharString&) { copyFrom(ori.m_data, ori.m_length); }
	void copyFrom(const ttype* ori, UInt newlen);
	void operator += (const CharString& ori);
	inline void push_back(const ttype ch) { addOrDouble(ch); }
	inline void addOrDouble(const ttype ch) {
		if(m_capacity <= m_length + 2) {
			expand0(m_length * 2 + 4);
		}
		m_data[m_length++] = ch;
	}
	// 将begin指向的以end为结束位置的字符串复制到自身末尾;
	// addCrLf: 3表示附加CRLF, 2表示附加CR, 1表示附加LF, 0表示什么都不附加
	void addLine(const ttype* begin, const ttype* end, char addCrLf);
	void addStr(const ttype* begin, const UInt len);
	static CharString Concat(const CharString& s1, const CharString& s2);

	int subString(UInt pos, UInt len, CharString& target) const;
	CharString subString(UInt pos, UInt pos2) const;
	UInt index_kmp(ttype *tstr, UInt pos) const;
	UInt index_kmp(const CharString& T, UInt pos) const;
	UInt index_nokmp(const CharString& tstr, UInt pos) const;
};

template <typename ttype>
CharString<ttype>::CharString(const ttype* ori)
{
	register UInt size = 0u;
	register const ttype* end = ori;
	while(*end++)
		size++;
	m_length = size;
	size++;
	if(size < sc_step_autoalloc)
		size = sc_step_autoalloc;
	register ttype *p = m_data = (ttype*)malloc(sizeof(ttype) * size);
	register const ttype *pori = ori;
	for(; end > pori; )
		*p++ = *pori++;
	m_capacity = size;
	m_flag = 0;
	more = 0;
}

template <typename ttype>
CharString<ttype>::CharString(const ttype* ori, const ttype* end)
{
	if(end < ori) {
		m_data = NULL;
		m_length = 0;
		m_capacity = 0;
	}
	else {
		m_length = end - ori;
		m_capacity = m_length + 1;
		m_data = (ttype*)malloc(sizeof(ttype) * m_capacity);
		ttype* po = m_data;
		for(; end > ori; )
			*po++ = *ori++;
		*po = 0;
	}
	m_flag = 0;
	more = 0;
}

template <typename ttype>
CharString<ttype>::CharString(const CharString& ori)
	: SimpleString((const SimpleString &)ori)
{
	// register const ttype *end = ori.m_data + ori.m_length, *pori = ori.m_data;
	// register ttype *p = m_data = (ttype*)malloc(sizeof(ttype) * (ori.m_length + 1));
	// for(; end >= pori; )
	// 	*p++ = *pori++;
	m_capacity = ori.m_capacity;
	register CharString *const p = const_cast<CharString *>(&ori);
	p->m_data = NULL;
	p->m_length = 0;
	p->m_capacity = 0;
	p->m_flag = 0;
}

template <typename ttype>
CharString<ttype>::CharString(CharString&& ori)
	: SimpleString((const SimpleString &)ori)
{
	m_capacity = ori.m_capacity;
	ori.m_data = NULL;
	ori.m_length = 0;
	ori.m_capacity = 0;
	ori.m_flag = 0;
}

template <typename ttype>
void CharString<ttype>::copyFrom(const ttype* ori, UInt newlen)
{
	if(m_data != ori)
	{
		expand(newlen + 1);
		register const ttype *end = ori + newlen, *pori = ori;
		register ttype *p = m_data;
		for(; end >= pori; )
			*p++ = *pori++;
		m_length = newlen;
	}
}

template <typename ttype>
void CharString<ttype>::operator = (const CharString& ori) {
	if(m_data && !m_flag)
		free(m_data);
	m_data = ori.m_data;
	m_length = ori.m_length;
	m_capacity = ori.m_capacity;
	m_flag = ori.m_flag;
	more = ori.more;
	register CharString *const p = const_cast<CharString *>(&ori);
	p->m_data = NULL;
	p->m_length = 0;
	p->m_capacity = 0;
	p->m_flag = 0;
}

template <typename ttype>
void CharString<ttype>::operator = (CharString&& ori)
{
	if(m_data && !m_flag)
		free(m_data);
	m_data = ori.m_data;
	ori.m_data = NULL;
	m_length = ori.m_length;
	ori.m_length = 0;
	m_capacity = ori.m_capacity;
	ori.m_capacity = 0;
	m_flag = ori.m_flag;
	ori.m_flag = 0;
	more = ori.more;
}

template <typename ttype>
void CharString<ttype>::operator += (const CharString& ori)
{
	if(0 != ori.m_length)
	{
		UInt x = m_length + ori.m_length;
		expand(x + 1);
		register ttype *p1 = m_data + m_length;
		for(register ttype *p2 = ori.m_data, *const end = p2 + ori.m_length; end > p2; )
			*p1++ = *p2++;
		*p1 = 0;
		m_length = x;
	}
}

template <typename ttype>
void CharString<ttype>::addLine(const ttype* begin, const ttype*const end, char addCrLf)
{
	if(end > begin)
	{
		if((end - begin) + 4 >= (int) (m_capacity - m_length))
			expand0(m_length + ((end - begin) + 4));
		register ttype *p1 = m_data + m_length;
		for(register const ttype *p2 = begin, *const end2 = end; end2 > p2; )
			*p1++ = *p2++;
		if (addCrLf & 0x2)
			*p1++ = '\r';
		if (addCrLf & 0x1)
			*p1++ = '\n';
		*p1 = 0;
		m_length = p1 - m_data;
	}
}

template <typename ttype>
void CharString<ttype>::addStr(const ttype* begin, const UInt len)
{
	if(len) {
		if(len + 2 >= (int) (m_capacity - m_length))
			expand0(m_length + (len + 2));
		register ttype *p1 = m_data + m_length;
		for(register const ttype *p2 = begin, *const end2 = begin + len; end2 > p2; )
			*p1++ = *p2++;
		*p1 = 0;
		m_length = p1 - m_data;
	}
}

template <typename ttype>
CharString<ttype> CharString<ttype>::Concat(const CharString<ttype>& s1, const CharString<ttype>& s2)
{
	CharString<ttype> temp(s1);
	temp += s2;
	return (CharString<ttype>&&)temp;
}

// 偏特化未能实现，只好暂时只用strncpy函数
template <typename ttype>
int CharString<ttype>::subString(UInt pos, UInt len, CharString<ttype>& target) const
{
	if(pos > 0 && len > 0 && (--pos) + len <= m_length)
	{
		target.expand(len + 1);
		strncpy(target.c_str(), m_data + pos, len);
		target.c_str()[len] = 0;
		return 0;
	}
	else
		return -1;
}

// 偏特化未能实现，只好暂时只用strncpy函数
template <typename ttype>
CharString<ttype> CharString<ttype>::subString(UInt pos, UInt pos2) const
{
	if(pos2 > pos && --pos > 0 && pos2 <= m_length)
	{
		register UInt size = pos2 - pos;
		ttype* t1 = (ttype*)malloc(sizeof(ttype) * size);
		strncpy(t1, m_data + pos, --size);
		t1[size] = 0;
		return CharString(t1, size , 1 + size, 0);
	}
	else
		return CharString();
}

template <typename ttype>
UInt CharString<ttype>::index_kmp(ttype *tstr, UInt pos) const
{
	if(*tstr && pos <= m_length)
	{
		if(tstr == m_data)
			return (pos == 1);
		else
		{
			register UInt size = 0u;
			register const ttype* end = tstr;
			while(*end++)
				size++;
			if(m_length + 1 < size + pos)
				return 0;
			else
			{
				int next[] = new int [size];
				KMP::get_next(tstr, size, next);
				UInt re = KMP::index_kmp(m_data, tstr, m_length, size, next, pos);
				delete [] next;
				return re;
			}
		}
	}
	else
		return 0;
}

template <typename ttype>
UInt CharString<ttype>::index_kmp(const CharString<ttype>& tstr, UInt pos) const
{
	if(0 == tstr.m_length || m_length - pos < tstr.m_length - 1)
		return 0;
	else if(tstr.m_data == m_data)
		return (pos == 1);
	else
	{
		int next[] = new int [tstr.m_length];
		KMP::get_next(tstr.m_data, tstr.m_length, next);
		UInt re = KMP::index_kmp(m_data, tstr.m_data, m_length, tstr.m_length, next, pos);
		delete [] next;
		return re;
	}
}

template <typename ttype>
UInt CharString<ttype>::index_nokmp(const CharString<ttype>& tstr, UInt pos) const
{
	if(0 == tstr.m_length || m_length - pos < tstr.m_length - 1)
		return 0;
	else if(tstr.m_data == m_data)
		return (pos == 1);
	else
	{
		register UInt i = pos, j = 1, lens = m_length, lent = tstr.m_length;
		register ttype *s = m_data - 1, *t = tstr.m_data - 1;
		while(i <= lens && j <= lent)
		{
			if(s[i] == t[j]) {++i; ++j;}
			else { i = i - j + 2; j = 1;}
		}
		return (j > lent) ? (i - lent): 0;
	}
}


#endif