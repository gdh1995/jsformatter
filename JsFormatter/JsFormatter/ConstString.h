#ifndef _GDH_CONST_STRING_
#define _GDH_CONST_STRING_

#ifndef	NULL
#ifdef __cplusplus
#define	NULL	0
#else
#define	NULL	((void *)0)
#endif
#endif

typedef unsigned int UInt;

template <typename ttype>
struct StringStruct {
	/* public: */
		UInt	m_length;
	/* public: */
		ttype*	m_data;
	/* public: */
	union {
	struct {
		char	flag;
		char	flag2;
	};
		short	m_flag;
	};
	/* public: */	
	union {
	struct {
		char	more;
		char	more2;
	};
		short	m_more;
	};
};

template <typename ttype>
class ConstString: protected StringStruct<ttype> {
public:
	inline void setData(ttype *const start) { m_data = start; }
	inline void setLength(const UInt len) { m_length = len; }
	inline void autoLength() { m_length = lengthOf(m_data); }
	
	inline void setFlag (const short flag) { m_flag  = flag ; }
	inline void setFlag1(const char byte1) {   flag  = byte1; }
	inline void setFlag2(const char byte2) {   flag2 = byte2; }
	inline short getFlag() const { return m_flag; }
	inline char getFlag1() const { return  flag ; }
	inline char getFlag2() const { return  flag2; }
	
protected:
	// 不建议同以'\0'结尾的字符串进行直接比较
			bool operator == (const ttype* const ori) const;
	inline  bool operator != (const ttype* const ori) const { return !(this->operator==(ori)); }

public:
	typedef StringStruct<ttype> BaseString;
	using BaseString::more;
	using BaseString::more2;
	using BaseString::m_more;

	typedef ttype Char;
	
public:
	inline const ttype* c_str() const { return m_data; }
	inline const ttype* c_end() const { return m_data + m_length; }
	inline const ttype*   str() const { return m_data; }
	inline operator const ttype * () const { return m_data; }
	inline const BaseString& raw() const { return *this; }
	
	inline bool nempty() const { return 0 != m_length; }
	inline bool empty() const { return 0 == m_length; }
	
	// size() === length()
	inline UInt size  () const { return m_length; }
	// length() === size()
	inline UInt length() const { return m_length; }
	inline UInt len   () const { return m_length; }
	static UInt lengthOf(const ttype * const str);
	
	// inline ttype& get(const UInt pos) { return m_data[pos]; }
	// inline ttype&  at(const UInt pos) { return pos < m_length ? m_data[pos] : 0; }
	
	inline const ttype& get(const UInt pos) const { return m_data[pos]; }
	inline const ttype&  at(const UInt pos) const { return pos < m_length ? m_data[pos] : 0; }
	

	bool  equals0(const ttype* const str2) const;
	bool nequals0(const ttype* const str2) const;
	inline bool  equals(const ttype* const str2, const UInt len2) const { return m_length == len2 &&  equals0(str2); }
	inline bool nequals(const ttype* const str2, const UInt len2) const { return m_length != len2 || nequals0(str2); }
	inline bool  equals(const ttype ch1, const ttype ch2) const { return (m_length == 2) && m_data[0] == ch1 && m_data[1] == ch2; };
	inline bool nequals(const ttype ch1, const ttype ch2) const { return (m_length != 2) || m_data[0] != ch1 || m_data[1] != ch2; };
	inline bool operator == (const ConstString& ori) const { return this->equals(ori.m_data, ori.m_length); }
	inline bool operator != (const ConstString& ori) const { return this->nequals(ori.m_data, ori.m_length); }
	inline bool operator == (const ttype ch) const { return (m_length == 1) && ch == m_data[0]; }
	inline bool operator != (const ttype ch) const { return (m_length != 1) || ch != m_data[0]; }
	
	inline bool findIn(const ttype* const str_to_find_in) const { return m_length == 1 && findIn(m_data[0], str_to_find_in); }
	static bool findIn(const ttype ch, const ttype* const str_to_find_in);
	// 返回值和pos均从1开始; 返回0表示未找到
	UInt index(const ttype tch, const UInt pos) const;
	
	// return its start position
	// * the result m_eans an empty string if return == null or pend <= return
	const  ttype* trim(int* const plen) const;
	// return its start position
	// * the result m_eans an empty string if return == null or pend <= return
	inline ttype* trim(int* const plen) { return (ttype*)((const ConstString*) this)->trim(plen); };
	// return the offset of its end
	int trimRight() const;
};

template <typename ttype>
bool ConstString<ttype>::operator == (const ttype* const ori) const
{
	if(NULL == ori)
		return (0 == m_length);
	register const ttype *p1 = m_data, *p2 = ori, *const end = m_data + m_length;
	while(p1 < end) {
		if(*p1++ != *p2++)
			return false;
	}
	return (0 == *p2);
}

template <typename ttype>
bool ConstString<ttype>::equals0(const ttype* const str2) const
{
	register const ttype *p1 = m_data, *p2 = str2, *const end = m_data + m_length;
	while(p1 < end) {
		if(*p1++ != *p2++)
			return false;
	}
	return true;
}

template <typename ttype>
bool ConstString<ttype>::nequals0(const ttype* const str2) const
{
	register const ttype *p1 = m_data, *p2 = str2, *const end = m_data + m_length;
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
UInt ConstString<ttype>::index(const ttype tch, const UInt pos) const
{
	// 为防止 pos < 0 造成 p 溢出后小于end, 有必要直接判断pos和mlength
	if (pos <= m_length) {
		register ttype *p = m_data + pos - 1, *end = m_data + m_length;
		while (p < end) {
			if (tch == *p++)
				return (p - m_data);
		}
	}
	return 0;
}

template <typename ttype>
const ttype* ConstString<ttype>::trim(int* const pend) const {
	register const ttype * end = m_data;
	register const ttype * start = end;
	if (start != NULL) {
		end += m_length;
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
	register const Char *end = m_data;
	if (end != NULL) {
		register const Char *const start = end;
		end += m_length;
		while(--end >= start) {
			if (*end != ' ' && *end != '\t' && *end != '\n' && *end != '\r')
				break;
			end = end;
		}
		++end;
	}
	return end - m_data;
}

template <typename ttype>
UInt ConstString<ttype>::lengthOf(const ttype* const str) {
	register UInt size = 0u;
	for(register const ttype* s = str; *s++; ) { size++; }
	return size;
}

#endif