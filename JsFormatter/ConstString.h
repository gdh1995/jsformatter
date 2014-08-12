#ifndef _GDH_CONST_STRING_
#define _GDH_CONST_STRING_

#ifndef	NULL
#ifdef __cplusplus
#define	NULL	0
#else
#define	NULL	((void *)0)
#endif
#endif

template <typename ttype>
struct StringStruct {
	/* public: */	size_t	mlength;
	/* public: */	ttype*	mdata;
	/* public: */	short	mflag;
	/* public: */	short	more;
};

template <typename ttype>
class ConstString: protected StringStruct<ttype> {
public:
	using StringStruct::more;

	inline void setData(ttype *start) { mdata = start; }
	inline void setLength(size_t len) { mlength = len; }
	inline void autoLength() { mlength = lengthOf(mdata); }
	inline void setFlag0() { mflag = 0; }
	inline void setFlag1() { mflag = 1; }

protected:
	// 不建议同以'\0'结尾的字符串进行直接比较
			bool operator == (const ttype* ori) const;
	inline  bool operator != (const ttype* ori) const { return !(this->operator==(ori)); }

public:
	typedef ttype Char;
	typedef StringStruct<ttype> BaseString;

	inline StringStruct& raw() { return *this; }
	inline const StringStruct& raw() const { return *this; }
	inline ttype* c_str() { return mdata; }
	inline const ttype* c_str() const { return mdata; }

	inline bool nempty() const { return 0 != mlength; }
	inline bool empty() const { return 0 == mlength; }
	
	// length() === size()
	inline size_t length() const { return mlength; }
	// size() === length()
	inline size_t size() const { return mlength; }
	static size_t lengthOf(const ttype *str);
	
	inline ttype operator [] (size_t pos) const { return mdata[pos]; }
	inline ttype& get(size_t pos) { return mdata[pos]; }
	inline const ttype& get(size_t pos) const { return mdata[pos]; }
	
	inline bool  equals(const ttype ch1, const ttype ch2) const { return (mlength == 2) && mdata[0] == ch1 && mdata[1] == ch2; };
	inline bool nequals(const ttype ch1, const ttype ch2) const { return (mlength != 2) || mdata[0] != ch1 || mdata[1] != ch2; };
	bool  equals(const ttype* str2, size_t len2) const;
	bool nequals(const ttype* str2, size_t len2) const;
	inline bool operator == (const ConstString& ori) const { return this->equals(ori.mdata, ori.mlength); }
	inline bool operator != (const ConstString& ori) const { return this->nequals(ori.mdata, ori.mlength); }
	inline bool operator == (const ttype ch) const { return (mlength == 1) && ch == mdata[0]; }
	inline bool operator != (const ttype ch) const { return (mlength != 1) || ch != mdata[0]; }
	
	inline bool findIn(const ttype* const str_to_find_in) const { return mlength == 1 && findIn(mdata[0], str_to_find_in); }
	static bool findIn(const ttype ch, const ttype* const str_to_find_in);
	// 返回值和pos均从1开始; 返回0表示未找到
	size_t index(ttype tch, size_t pos) const;
	
	// return its start position
	// * the result means an empty string if return == null or pend <= return
	const  ttype* trim(int* const plen) const;
	// return its start position
	// * the result means an empty string if return == null or pend <= return
	inline ttype* trim(int* const plen) { return (ttype*)((const ConstString*) this)->trim(plen); };
	// return the offset of its end
	int trimRight() const;
};

template <typename ttype>
bool ConstString<ttype>::operator == (const ttype* ori) const
{
	if(NULL == ori)
		return (0 == mlength);
	register const ttype *p1 = mdata, *p2 = ori, *const end = mdata + mlength;
	while(p1 < end) {
		if(*p1++ != *p2++)
			return false;
	}
	return (0 == *p2);
}

template <typename ttype>
bool ConstString<ttype>::equals(const ttype* str2, size_t len2) const
{
	if(len2 != mlength)
		return false;
	register const ttype *p1 = mdata, *p2 = str2, *const end = mdata + mlength;
	while(p1 < end) {
		if(*p1++ != *p2++)
			return false;
	}
	return true;
}

template <typename ttype>
bool ConstString<ttype>::nequals(const ttype* str2, size_t len2) const
{
	if(len2 != mlength)
		return true;
	register const ttype *p1 = mdata, *p2 = str2, *const end = mdata + mlength;
	while(p1 < end) {
		if(*p1++ != *p2++)
			return true;
	}
	return false;
}

template <typename ttype>
bool ConstString<ttype>::findIn(const ttype ch, const ttype* const str_to_find_in) {
	for (register const ttype *s = str_to_find_in; *s; ) {
		if (*s++ == ch) {
			return true;
		}
	}
	return false;
}

template <typename ttype>
size_t ConstString<ttype>::index(ttype tch, size_t pos) const
{
	// 为防止 pos < 0 造成 p 溢出后小于end, 有必要直接判断pos和mlength
	if (pos <= mlength) {
		register ttype *p = mdata + pos - 1, *end = mdata + mlength;
		while (p < end) {
			if (tch == *p++)
				return (p - mdata);
		}
	}
	return 0;
}

template <typename ttype>
const ttype* ConstString<ttype>::trim(int* const pend) const {
	register const ttype * end = mdata;
	register const ttype * start = end;
	if (start != NULL) {
		end += mlength;
		for(; start < end; start++) {
			if (*start != ' ' && *start != '\t' && *start != '\n' && *start != '\r')
				break;
		}
		while(--end >= start) {
			if (*end != ' ' && *end != '\t' && *end != '\n' && *end != '\r')
				break;
		}
		++end;
	}
	*pend = end - start;
	return start;
}

template <typename ttype>
int ConstString<ttype>::trimRight() const {
	register const Char *end = mdata;
	if (end != NULL) {
		register const Char *const start = end;
		end += mlength;
		while(--end >= start) {
			if (*end != ' ' && *end != '\t' && *end != '\n' && *end != '\r')
				break;
			end = end;
		}
		++end;
	}
	return end - mdata;
}

template <typename ttype>
size_t ConstString<ttype>::lengthOf(const ttype* str) {
	register size_t size = 0u;
	while(*str++)
		size++;
	return size;
}

#endif