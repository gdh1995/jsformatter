#ifndef _GDH_STRING_
#define _GDH_STRING_
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif
#include <malloc.h>

namespace KMP
{
	template <typename ttype>
	inline size_t length(ttype* str)
	{
		register size_t size = 0u;
		while(*str++)
			size++;
		return size;
	}

	template <typename ttype>
	void get_next(ttype* str, size_t len, int nextval[])
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
		// register size_t i = 1, j = 0;
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
	size_t index_kmp(ttype* s, ttype* t, size_t lens, size_t lent, int next[], size_t pos2)
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
		register size_t i = pos, j = 0;
		while(i < lens && j < lent)
		{
		if((0 == j) || (s[i] == t[j])) { ++i; ++j; }
		else j = next[j];
		}
		return (j > lent) ? (i - lent) : 0;*/
	}
};

// 所有pos的起点均为1
template <typename ttype>
class CharString
{
/*
private:
	void operator = (const CharString& ori);
/*/
public:
	// 一般使用 右值构造函数拷贝后右值赋值，它本身可以用成员函数copy_from(const CharString&);代替
	void operator = (const CharString& ori) {
		if(!mflag && mdata)
			free(mdata);
		mdata = ori.mdata;
		mlength = ori.mlength;
		mcapacity = ori.mcapacity;
		mflag = ori.mflag;
		type = ori.type;
		register CharString *const p = const_cast<CharString *>(&ori);
		p->mdata = NULL;
		p->mlength = 0;
		p->mcapacity = 0;
		p->mflag = 0;
	}
//*/

protected:
	ttype *mdata;
	size_t mlength;
	size_t mcapacity;
	short mflag;
	static const size_t sc_step_autoalloc = 4;

public:
	typedef ttype Char;
	short type;
	explicit inline CharString(): mdata(NULL), mlength(0), mcapacity(0), mflag(0), type(0) { }
	explicit inline CharString(ttype* ori, size_t len, size_t capa, int flag = 0): mdata(ori), mlength(len), mcapacity(capa), mflag(flag), type(0) { }
	explicit CharString(const ttype* ori);
	explicit CharString(const ttype* ori, const ttype* end);
	explicit CharString(const CharString& ori);
	explicit CharString(CharString&& ori);
	inline ~CharString() {
		if(!mflag && mdata)
			free(mdata);
	}
	inline bool isempty() const { return 0 == mlength; }
	// length() === size()
	inline size_t length() const { return mlength; }
	// size() === length()
	inline size_t size() const { return mlength; }
	// It is for changing it at another place that there's no 'const';
	inline ttype* c_str() const { return mdata; }
	inline void setLength(size_t len) { mlength = len; }
	inline void autoLength() { mlength = KMP::length(mdata); }
	inline void expand(size_t size) { if(size > mcapacity) { mdata = (ttype*)realloc(mdata, sizeof(ttype) * size); mcapacity = size; } }
	inline void reserve(size_t size) { expand(size); }

	inline ttype& operator[] (size_t pos) { return mdata[pos]; }

	bool operator == (const ttype* ori) const;
	inline bool operator!=(const ttype* ori) { return !(operator==(ori)); }
	inline bool operator==(const ttype ch) { return (mlength == 1) && ch == mdata[0]; }
	inline bool operator!=(const ttype ch) { return (mlength != 1) || ch != mdata[0]; }
	inline bool findIn(const ttype* str_to_find_in) const {
		if (mlength == 1) {
			for (register const char ch = mdata[0], *s = str_to_find_in; *s; s++) {
				if (*s == ch) {
					return true;
				}
			}
		}
		return false;
	}

	bool operator == (const CharString& ori) const;
	void operator = (CharString&& ori);
	// 使用“右值构造”和“右值赋值”还不能完全代替它，因为要有减少内存分配的考虑;
	void copyFrom(const CharString&);
	void copyFrom(const ttype* ori, size_t newlen);
	void operator += (const CharString& ori);

	inline void addOrDouble(const ttype ch)
	{
		if(mcapacity <= mlength + 2) {
			expand(mlength * 2 + 4);
		}
		register ttype *p1 = mdata + (mlength++);
		*p1 = ch;
		*++p1 = 0;
	}

	// 将begin指向的以end为结束位置的字符串复制到自身末尾;
	void addline(const ttype* begin, const ttype* end);

	int subString(size_t pos, size_t len, CharString& target) const;
	CharString subString(size_t pos, size_t pos2) const;
	inline size_t index(ttype tch, size_t pos) const;
	size_t index_kmp(ttype *tstr, size_t pos) const;
	size_t index_kmp(const CharString& T, size_t pos) const;
	size_t index_nokmp(const CharString& tstr, size_t pos) const;
	static CharString Concat(const CharString& s1, const CharString& s2);
};


//#include <CharString.h>

//template <typename ttype>
//void CharString<ttype>::operator = (const CharString& ori)

template <typename ttype>
CharString<ttype>::CharString(const ttype* ori): mflag(0), type(0)
{
	register size_t size = 0u;
	register const ttype* end = ori;
	while(*end++)
		size++;
	mlength = size;
	size++;
	if(size < sc_step_autoalloc)
		size = sc_step_autoalloc;
	register ttype *p = mdata = (ttype*)malloc(sizeof(ttype) * size);
	register const ttype *pori = ori;
	for(; end > pori; )
		*p++ = *pori++;
	mcapacity = size;
}

template <typename ttype>
CharString<ttype>::CharString(const ttype* ori, const ttype* end): mflag(0), type(0)
{
	if(end < ori)
	{
		mdata = NULL;
		mlength = 0;
		mcapacity = 0;
	}
	else
	{
		mlength = end - ori;
		mcapacity = mlength + 1;
		mdata = (ttype*)malloc(sizeof(ttype) * mcapacity);
		ttype* po = mdata;
		for(; end > ori; )
			*po++ = *ori++;
		*po = 0;
	}
}

template <typename ttype>
CharString<ttype>::CharString(const CharString& ori)
	: mdata(ori.mdata), mcapacity(ori.mcapacity), mlength(ori.mlength)
	, mflag(0), type(ori.type)
{
	// register const ttype *end = ori.mdata + ori.mlength, *pori = ori.mdata;
	// register ttype *p = mdata = (ttype*)malloc(sizeof(ttype) * (ori.mlength + 1));
	// for(; end >= pori; )
	// 	*p++ = *pori++;
	register CharString *const p = const_cast<CharString *>(&ori);
	p->mdata = NULL;
	p->mlength = 0;
	p->mcapacity = 0;
	p->mflag = 0;
}

template <typename ttype>
CharString<ttype>::CharString(CharString&& ori)
	: mdata(ori.mdata), mlength(ori.mlength), mcapacity(ori.mcapacity)
	, mflag(ori.mflag), type(ori.type)
{
	ori.mdata = NULL;
	ori.mlength = 0;
	ori.mcapacity = 0;
	ori.mflag = 0;
}

template <typename ttype>
void CharString<ttype>::copyFrom(const CharString& ori)
{
	if(mdata != &ori.mdata)
	{
		expand(ori.mcapacity);
		register const ttype *end = ori.mdata + ori.mlength, *pori = ori.mdata;
		register ttype *p = mdata;
		for(; end >= pori; )
			*p++ = *pori++;
		mlength = ori.mlength;
	}
}

template <typename ttype>
void CharString<ttype>::copyFrom(const ttype* ori, size_t newlen)
{
	if(mdata != ori)
	{
		expand(newlen + 1);
		register const ttype *end = ori + newlen, *pori = ori;
		register ttype *p = mdata;
		for(; end >= pori; )
			*p++ = *pori++;
		mlength = newlen;
	}
}

template <typename ttype>
void CharString<ttype>::operator = (CharString&& ori)
{
	if(!mflag)
		free(mdata);
	mdata = ori.mdata;
	ori.mdata = NULL;
	mlength = ori.mlength;
	ori.length = 0;
	mcapacity = ori.mcapacity;
	ori.mcapacity = 0;
	mflag = ori.mflag;
	ori.mflag = 0;
	type = ori.type;
}

template <typename ttype>
bool CharString<ttype>::operator == (const ttype* ori) const
{
	if(NULL == ori || 0 == *ori)
		return (0 == mlength);
	else if(0 == mlength)
		return false;
	else if(mdata != ori)
	{
		register const ttype *p1 = mdata - 1, *p2 = ori - 1, *const end = mdata + mlength;
		while(*++p1 == *++p2)
		{
			if(p1 == end)
				break;
		}
		return (*p1 == *p2);
	}
	else
		return true;
}

template <typename ttype>
bool CharString<ttype>::operator == (const CharString& ori) const
{
	if(this == &ori)
		return true;
	else if(0 == mlength)
		return (0 == ori.mlength);
	else if(0 == ori.mlength)
		return false;
	else if(mlength == ori.mlength)
	{
		register ttype *p1 = mdata - 1, *p2 = ori.mdata - 1, *end = mdata + mlength;
		while(*++p1 == *++p2)
		{
			if(p1 == end)
				break;
		}
		return (*p1 == *p2);
	}
	else
		return false;
}

template <typename ttype>
void CharString<ttype>::operator += (const CharString& ori)
{
	if(0 != ori.mlength)
	{
		size_t x = ori.mlength + mlength;
		if(x + 1 > mcapacity)
			expand(x + 1);
		register ttype *p1 = mdata + mlength;
		for(register ttype *p2 = ori.mdata, *const end = p2 + ori.mlength; end > p2; )
			*p1++ = *p2++;
		*p1 = 0;
		mlength = x;
	}
}


template <typename ttype>
void CharString<ttype>::addline(const ttype* begin, const ttype*const end)
{
	if(end > begin)
	{
		if((end - begin) + 2 >= (mcapacity - mlength))
			expand(mlength + (end - begin) + 3);
		register ttype *p1 = mdata + mlength;
		for(register const ttype *p2 = begin, *const end2 = end; end2 > p2; )
			*p1++ = *p2++;
		*p1++ = '\r';
		*p1++ = '\n';
		*p1 = 0;
		mlength = p1 - mdata;
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
int CharString<ttype>::subString(size_t pos, size_t len, CharString<ttype>& target) const
{
	if(pos > 0 && len > 0 && (--pos) + len <= mlength)
	{
		target.expand(len + 1);
		strncpy(target.c_str(), mdata + pos, len);
		target.c_str()[len] = 0;
		return 0;
	}
	else
		return -1;
}

// 偏特化未能实现，只好暂时只用strncpy函数
template <typename ttype>
CharString<ttype> CharString<ttype>::subString(size_t pos, size_t pos2) const
{
	if(pos2 > pos && --pos > 0 && pos2 <= mlength)
	{
		register size_t size = pos2 - pos;
		ttype* t1 = (ttype*)malloc(sizeof(ttype) * size);
		strncpy(t1, mdata + pos, --size);
		t1[size] = 0;
		return CharString(t1, size , 1 + size, 0);
	}
	else
		return CharString();
}

template <typename ttype>
size_t CharString<ttype>::index_kmp(ttype *tstr, size_t pos) const
{
	if(*tstr && pos <= mlength)
	{
		if(tstr == mdata)
			return (pos == 1);
		else
		{
			register size_t size = 0u;
			register const ttype* end = tstr;
			while(*end++)
				size++;
			if(mlength + 1 < size + pos)
				return 0;
			else
			{
				int next[] = new int [size];
				KMP::get_next(tstr, size, next);
				size_t re = KMP::index_kmp(mdata, tstr, mlength, size, next, pos);
				delete [] next;
				return re;
			}
		}
	}
	else
		return 0;
}

template <typename ttype>
size_t CharString<ttype>::index_kmp(const CharString<ttype>& tstr, size_t pos) const
{
	if(0 == tstr.mlength || mlength - pos < tstr.mlength - 1)
		return 0;
	else if(tstr.mdata == mdata)
		return (pos == 1);
	else
	{
		int next[] = new int [tstr.mlength];
		KMP::get_next(tstr.mdata, tstr.mlength, next);
		size_t re = KMP::index_kmp(mdata, tstr.mdata, mlength, tstr.mlength, next, pos);
		delete [] next;
		return re;
	}
}

template <typename ttype>
size_t CharString<ttype>::index_nokmp(const CharString<ttype>& tstr, size_t pos) const
{
	if(0 == tstr.mlength || mlength - pos < tstr.mlength - 1)
		return 0;
	else if(tstr.mdata == mdata)
		return (pos == 1);
	else
	{
		register size_t i = pos, j = 1, lens = mlength, lent = tstr.mlength;
		register ttype *s = mdata - 1, *t = tstr.mdata - 1;
		while(i <= lens && j <= lent)
		{
			if(s[i] == t[j]) {++i; ++j;}
			else { i = i - j + 2; j = 1;}
		}
		return (j > lent) ? (i - lent): 0;
	}
}

template <typename ttype>
size_t CharString<ttype>::index(ttype tch, size_t pos) const
{
	register ttype *p = mdata + pos - 1, *end = mdata + mlength;
	while(end > p && tch != *p++) { }
	return (end > p) ? (p - mdata) : 0;
}

#endif