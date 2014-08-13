#include "alldefs.h"
#include <stdio.h>
#include "FileCodes.h"
#include "realjsformatter.h"

#if defined UNICODE || defined _UNICODE
	DWORD out_codec = CP_UNICODE;
#else
	DWORD out_codec = CP_THREAD_ACP;
#endif

RealJSFormatter::FormatterOption g_options = {
	1, '\t',
	false, // bNotPutCR
	false, // bBracketAtNewLine
	false  // bEmpytLineIndent
};

int re = 0;
extern const char *err_str;
extern const void *err_msg;

int argn;
char **args;
int argIndex = 1;

CharString<char> file;
CharString<RealJSFormatter::Char> strJSFormat;

typedef void (voidfunc)();
voidfunc read, write, jsFormat;
voidfunc *p[] = {read, jsFormat, write};

#define print_help() printf("%s%s%s", "\t\
Javascript formatter ", JS_FORMATTER_VERSION_VALUE, " - by <gdh1995@qq.com>\n\
Thanks for & core code from: JSToolNpp (www.sunjw.us/jstoolnpp).\n")


int main(int n, char *s[]) {
	argn = n;
	args = s;
	if (argn < 3) {
		print_help();
		return 0;
	}

	int i = 0;
	do {
		p[i]();
	} while (re == 0 && ++i < sizeof(p) / sizeof(p[0]));
	if (re != 0) {
		if (err_str)
			fprintf(stderr, "ERROR: %s.\n%s\n", err_str, err_msg ? ((const char *) err_msg) : "");
		else
			fprintf(stderr, "ERROR: %d @ %d.\n", re, i);
	}
	else {
#if defined __TEST__ && __TEST__ > 0
		for (i = 0;  i < __TEST__; i++) jsFormat();
#endif
		printf("%s => %s\n", args[argIndex], args[argIndex + 1]);
	}
	return re;
}

void read() {
	re = FileAnaly::readAndEnsureCode(args[argIndex], &file, out_codec);
}

void write() {
	re = 0;
	FILE *fp = fopen(args[argIndex + 1], "wb");
	if (fp == NULL) {
		re = 0x11;
		err_str = "fail to open the file to write";
		err_msg = args[argIndex + 1];
	}
	else if (strJSFormat.size() > 0 &&
		fwrite(strJSFormat.str(), strJSFormat.size() * sizeof(RealJSFormatter::Char), 1, fp) != 1)
	{
		re = 0x12;
		err_str = "fail to write data into the file";
		err_msg = args[argIndex + 1];
	}
	else if (fflush(fp) != 0) {
		re = 0x13;
		err_str = "fail to save the data";
		err_msg = args[argIndex + 1];
	}
	else if (fclose(fp) != 0) {
		re = 0x14;
		err_str = "fail to save the data";
		err_msg = args[argIndex + 1];
	}
}

RealJSFormatter jsformat(NULL, 0, strJSFormat, g_options);
void jsFormat()
{
	re = 0;
	size_t jsLen = file.size();
	if (jsLen == 0)
		return;

	// clear old result (if any)
	strJSFormat.reset(((int) (jsLen * 1.1)) + 1024);
	strJSFormat.c_str()[0] = 0;
	jsformat.setInput(file.c_str(), file.size());
	jsformat.Go();
	re = 0;
	//catch(std::exception ex) {
	//	re = 1;
	//	err_str = "std::exception";
	//	err_msg = ex.what();
	//}
}