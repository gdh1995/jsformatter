//this file a source file of JSMinNpp
//Copyright (C) 2007 Don HO <donho@altern.org>
//Copyright (C) 2010-2010 Sun Junwen
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include <stdio.h>

#include "realjsformatter.h"
#include "FileCodes.h"
#include "version.h"


DWORD out_codec = CP_UTF8;

RealJSFormatter::FormatterOption g_options = {
	'\t', 1,
	RealJSFormatter::SKIP_READ_CR,
	true ? RealJSFormatter::PUT_CR : RealJSFormatter::NOT_PUT_CR,
	false ? RealJSFormatter::NEWLINE_BRAC : RealJSFormatter::NO_NEWLINE_BRAC,
	false ? RealJSFormatter::INDENT_IN_EMPTYLINE : RealJSFormatter::NO_INDENT_IN_EMPTYLINE
};

int re = 0;

int argn;
char **args;
int argIndex = 1;

CharString<char> file;
CharString<char> strJSFormat;

typedef void (voidfunc)();
voidfunc read, write, jsFormat;
voidfunc *p[] = {read, jsFormat, write};

int main(int n, char *s[])
{
	argn = n;
	args = s;
	if (argn < 3) {
		printf("%s%s%s", "\t\
Javascript formatter ", JS_FORMATTER_VERSION_VALUE, " - by <gdh1995@qq.com>\n\
Thanks for & core code from: JSToolNpp (www.sunjw.us/jstoolnpp).\n");
		return 0;
	}

	for (int i0 = 0; i0 < 1; i0++) {
		int i = 0;
		for (; re == 0 && i < sizeof(p) / sizeof(p[0]); i++) {
			p[i]();
		}
		if (re != 0) {
			fprintf(stderr, "ERROR: %d @ %d.\n", re, i);
		} else {
			printf("%s => %s\n", args[argIndex], args[argIndex + 1]);
		}
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
	} else if (fwrite(strJSFormat.c_str(), strJSFormat.length(), 1, fp) != 1) {
		re = 0x12;
	} else if (fflush(fp) != 0) {
		re = 0x13;
	} else if (fclose(fp) != 0) {
		re = 0x14;
	}
}

void jsFormat()
{
	re = 0;
	size_t jsLen = file.length();
	if (jsLen == 0) {
		return;
	}

	// clear old result (if any)
	{
		strJSFormat.expand(jsLen + 1024);
		strJSFormat.setLength(0);
		strJSFormat.c_str()[0] = 0;
	}
	try
	{	
		RealJSFormatter jsformat(file, strJSFormat, g_options);
		jsformat.Go();
	}
	catch(std::exception ex)
	{
		fprintf(stderr, "%s%s\n", "Exception: ", ex.what());
		re = 1;
	}
}
