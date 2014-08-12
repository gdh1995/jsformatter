#include "FileCodes.h"

const char *err_str = NULL;
const void *err_msg = NULL;

int codec(CharString<char> *file, DWORD oldpage, DWORD const newpage) {
	int re = 0x8;
	if (oldpage == CP_UNKNOWN) {
		register const char *str = file->c_str();
		const char *const end = str + file->length();
		--str;
		while (*++str) {
			register char ch = *str;
			if (ch > 0) {
				continue;
			}
			if ((ch & '\xC0') != '\xC0') {
				break;
			}

			static const char temp1[] = {'\xC0', '\xC0', '\xE0', '\xF0', '\xF8'};
			int next10 = sizeof(temp1) / sizeof(temp1[0]) - 1;
			do {
				ch &= temp1[next10];
			} while (ch != temp1[--next10]);
			if (!next10) {
				break;
			}

			++next10;
			const char *const str1 = str;
			while (--next10 && (*++str & '\xC0') == '\x80') {
			}
			if (next10) {
				str = str1;
				break;
			}
			oldpage = CP_UTF8;
		}
		if (oldpage == CP_UTF8 && str >= end) {
			oldpage = CP_UTF8;
		} else {
			oldpage = CP_THREAD_ACP;
			for (; str < end; str++) {
				if (*str == '\0') {
					oldpage = CP_UNICODE;
					break;
				}
			}
		}
	}
	if (oldpage == newpage) {
		re = 0;
	} else if (1) {
		CharString<char> temp;
		if (oldpage != CP_UNICODE) {
			FileAnaly::MToWide(temp, *file, oldpage);
		} else {
			temp = (CharString<char> &&) *file;
		}
		if (newpage != CP_UNICODE) {
			FileAnaly::WideToM(*file, temp, newpage);
		} else {
			*file = (CharString<char> &&) temp;
		}
		re = 0;
	}
	return re;
}

int FileAnaly::readAndEnsureCode(const char* const file, CharString<char>* dest, DWORD const codepage)
{
	int re = 0;  
	if(NULL != dest && NULL != file && L'\0' != *file)
	{
		long len;
		FILE *fp = fopen(file, "rb");
		if(NULL == fp)
		{
			err_str = "fail to open the file to read";
			err_msg = file;
			return -1;
		}
		else if((len = FileAnaly::getFileSize(fp)) < 0)
		{
			re = -2; // an error;
			err_str = "negative size of the file to be read";
			err_msg = file;
		}
		else
		{
			DWORD codep = FileAnaly::getCode(fp);
			if(codep == CP_UNICODE)
			{
				fseek(fp, 2, SEEK_SET);
				len -= 2;
			}
			else if(codep == CP_UTF8)
			{
				fseek(fp, 3, SEEK_SET);
				len -= 3;
			}
			size_t size = (size_t)len;
			dest->expand(size + 4);
			if(len > 0 && fread(dest->c_str(), size, 1, fp) != 1)
			{
				re = -3;
				err_str = "fail to read all data of the file";
				err_msg = file;
			}
			else
			{
				char *s1 = dest->c_str() + size;
				s1[2] = s1[1] = s1[0] = '\0';
				dest->setLength(size);
				if (codep == CP_UNKNOWN || codep != codepage) {
					re = codec(dest, codep, codepage);
				}
			}
		}
		fclose(fp);
	}
	return re;
}
