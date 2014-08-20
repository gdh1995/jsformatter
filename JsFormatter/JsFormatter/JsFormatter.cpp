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
	_T('\t'), 1,
	false, // bNotPutCR
	false, // bBracketAtNewLine
	false  // bEmpytLineIndent
};

int re = 0;
extern const char *err_str;
extern const void *err_msg;

int argn;
const char **args;
int argIndex;

CharString<char> file;
CharString<RealJSFormatter::Char> strJSFormat;

typedef void (voidfunc)();
voidfunc read, write, jsFormat, print_help;
voidfunc *p[] = {read, jsFormat, write};


int main(int n, const char *s[]) {
	if (n < 2) {
		print_help();
		return 0;
	}
	if (n == 2) {
		file.copyFrom(s[1], ConstString<char>::lengthOf(s[1]));
		UInt len = file.size();
		while (len) {
			register const char ch = file[--len];
			if (ch == '.' || ch == '/' || ch == '\\')
				break;
		}
		if (len == 0)
			len = file.size();
		file.setLength(len);
		file.addStr(".fmt.js", 8);
		s[0] = s[1];
		s[1] = file.c_str();
		file.setData(NULL);
		argn = 2;
		args = s;
	}
	else {
		argn = n - 1;
		args = s + 1;
	}
	argIndex = 0;

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

void print_help() {
	printf("%s%s%s", "\t\
Javascript formatter ", JS_FORMATTER_VERSION_VALUE, " - by <gdh1995@qq.com>\n\
Thanks for & core code from: JSToolNpp (www.sunjw.us/jstoolnpp).\n");
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
	UInt jsLen = file.size();
	if (jsLen == 0)
		return;

	// clear old result (if any)
	strJSFormat.reset(jsLen + jsLen / 2 + 1024);
	strJSFormat.c_str()[0] = 0;
	jsformat.setInput(file, file.size());
	jsformat.go();
	re = 0;
	//catch(std::exception ex) {
	//	re = 1;
	//	err_str = "std::exception";
	//	err_msg = ex.what();
	//}
}
