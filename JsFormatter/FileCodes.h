#ifndef _GDH_FILE_CODES_
#define _GDH_FILE_CODES_
#include <stdio.h>
#include <windows.h>
#include <locale.h>
#include "CharString.h"

namespace FileAnaly
{
#define CP_UNICODE ((DWORD)(-1))
#define CP_UNKNOWN 0

	const char*const locale = setlocale(LC_ALL, "");

	inline long getFileSize(FILE*fp)
	{
		long back = ftell(fp);
		fseek(fp, 0, SEEK_END);
		long len = ftell(fp);
		fseek(fp, back, SEEK_SET);
		return len;
	}

	inline DWORD getCode(FILE *fp)
	{
		long back = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		DWORD re = CP_UNKNOWN;
		switch(fgetc(fp))
		{
		case 0xff:
			if(fgetc(fp) == 0xfe)
				re = CP_UNICODE; // My flag of Unicode
			break;
		case 0xef:
			if(fgetc(fp) == 0xbb && fgetc(fp) == 0xbf)
				re =  CP_UTF8;
			break;
		default: break; // think as current ASCII;
		}
		fseek(fp, back, SEEK_SET);
		return re;
	}

	inline void MToWide(CharString<char>& dest, const CharString<char>& ori, DWORD codepage)
	{
		int len = ori.length();
		dest.expand(len * 2 + 4);
		dest.setLength( MultiByteToWideChar(codepage, 0, ori.c_str(), len, (wchar_t*) dest.c_str(), len + 2) * 2 );
		char * str = dest.c_str() + dest.length();
		str[1] = str[0] = '\0';
	}

	inline void WideToM(CharString<char>& dest, const CharString<char>& ori,DWORD codepage)
	{
		int len = (ori.length() + 1) / 2;
		dest.expand(len * 3 + 4);
		dest.setLength(WideCharToMultiByte(codepage, 0, (wchar_t*) ori.c_str(), len, dest.c_str(), len * 3 + 2, NULL, NULL) );
		char * str = dest.c_str() + dest.length();
		str[0] = 0;
	}

	int readAndEnsureCode(const char* file, CharString<char>* dest, DWORD const codepage);

};

//extern "C" _declspec(dllexport) int _stdcall writeUnicode(const wchar_t *file, const CharString<wchar_t>* data);

#endif