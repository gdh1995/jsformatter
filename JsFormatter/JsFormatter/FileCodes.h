#ifndef _GDH_FILE_CODES_
#define _GDH_FILE_CODES_
#include <stdio.h>
#include <windows.h>
#include "CharString.h"

namespace FileAnaly
{
	// "CP_UNICODE" := little-endian
	#define CP_UNICODE ((DWORD)(-1))
	#define CP_UNKNOWN ((DWORD)(-2))

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

	inline void strMToWide(CharString<char>& dest, const CharString<char>& ori, DWORD codepage);

	inline void strWideToM(CharString<char>& dest, const CharString<char>& ori,DWORD codepage);

	int readAndEnsureCode(const char* file, CharString<char>* dest, DWORD const codepage);

};

//extern "C" _declspec(dllexport) int _stdcall writeUnicode(const wchar_t *file, const CharString<wchar_t>* data);

#endif