#include "alldefs.h"
#include <stdio.h>
#include "FileCodes.h"
#include "realjsformatter.h"

DWORD out_codec = CP_UTF8;

RealJSFormatter::FormatterOption g_options = {
	_T('\t'), 1,
	false, // bNotPutCR
	false, // bBracketAtNewLine
	false  // bEmpytLineIndent
};

int re = 0;
extern const char *err_str;
extern const void *err_msg;

bool put_result_to_std_out = false;
int argn;
const char **args;
int argIndex;

CharString<char> file;
RealJSFormatter::OutputString strJSFormat;

typedef void (voidfunc)();
extern "C" __declspec(dllexport) int real_main();
voidfunc read, write, jsFormat, print_help;
voidfunc *p[] = {read, jsFormat, write};

RealJSFormatter jsformat(NULL, 0, strJSFormat, g_options);

// release
//extern "C" __declspec(dllexport)
int main(int n, const char *s[]) {
  if (n < 2) {
    print_help();
    return 0;
  }
  if (n == 2) {
    if (!strcmp(s[1], "-")) {
      s[0] = s[1];
      argn = 2;
      args = s;
    }
    else {
      CharString<char> dest;
      dest.copyFrom(s[1], ConstString<char>::lengthOf(s[1]));
      UInt len = dest.size();
      while (len) {
        register const char ch = dest[--len];
        if (ch == '.' || ch == '/' || ch == '\\')
          break;
      }
      if (len == 0 || dest[len] != '.')
        len = dest.size();
      dest.setLength(len);
      dest.addStr(".fmt.js", 8);
      s[0] = s[1];
      s[1] = dest.c_str();
      dest.setData(NULL);
      argn = 2;
      args = s;
    }
  }
  else {
    argn = n - 1;
    args = s + 1;
  }
  return real_main();
}

// extern "C" __declspec(dllexport)
int __declspec(noinline) test_target(char *input_path, char *argv_0) {
  static char dest[512], *(local_args[3]);
  strcpy(dest, input_path);
  strcat(dest, ".out");
  local_args[0] = input_path;
  local_args[1] = dest;
  local_args[2] = NULL;
  args = (const char**)local_args;
  argn = 2;
  jsformat.init();
  return real_main();
}

// test main
//extern "C" __declspec(dllexport)
int main1(int argc, char** argv)
{
  if (argc < 2) {
    printf("Usage: %s <input file>\n", argv[0]);
    return 0;
  }

  if (argc == 3 && !strcmp(argv[2], "loop"))
  {
    //loop inside application and call target infinitey
    while (true)
    {
      test_target(argv[1], argv[0]);
    }
  }
  else
  {
    //regular single target call
    return test_target(argv[1], argv[0]);
  }
}


//extern "C" __declspec(dllexport)
int real_main() {
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
	}
  if (!put_result_to_std_out && re == 0) {
    printf("%s => %s\n", args[argIndex], args[argIndex + 1]);
  }
  /* {
    FILE* fp = fopen("R:\\working\\log.txt", "a+");
    if (re != 0) {
      if (err_str)
        fprintf(fp, "ERROR: %s.\n%s\n", err_str, err_msg ? ((const char *)err_msg) : "");
      else
        fprintf(fp, "ERROR: %d @ %d.\n", re, i);
    }
    else {
#if defined __TEST__ && __TEST__ > 0
      for (i = 0; i < __TEST__; i++) jsFormat();
#endif
      fprintf(fp, "%s => %s\n", args[argIndex], args[argIndex + 1]);
    }
    fclose(fp);
  } //*/
	return re;
}

void print_help() {
	printf("%s%s%s", "\t\
Javascript formatter ", JS_FORMATTER_VERSION_VALUE, " - by <gdh1995@qq.com>\n\
Thanks for & core code from: JSToolNpp (www.sunjw.us/jstoolnpp).\n");
}

void read() {
  if (!strcmp(args[argIndex], "-")) {
    int offset = 0, line = 0;
    do {
      offset += line;
      file.expand(offset / sizeof(char) + 202);
      line = fread(((char*)file.c_str()) + offset, 1, 200, stdin);
    } while (line > 0);
    file.setLength(offset / sizeof(char));
    re = 0;
    return;
  }
	re = FileAnaly::readAndEnsureCode(args[argIndex], &file, out_codec);
  /* FILE* fp = fopen("R:\\working\\log.txt", "a+");
  fprintf(fp, "read %s and len=%d, re=%d\n", args[argIndex], file.length(), re);
  fclose(fp); //*/
}

void write() {
	re = 0;
  put_result_to_std_out = !strcmp(args[argIndex + 1], "-");
  if (put_result_to_std_out) {
    if (strJSFormat.size() > 0) {
      fwrite(strJSFormat.str(), strJSFormat.size() * sizeof(RealJSFormatter::Char), 1, stdout);
      if (strJSFormat[strJSFormat.len() - 1] != '\r' && strJSFormat[strJSFormat.len() - 1] != '\n') {
        puts("");
      }
    }
    return;
  }
	FILE *fp = fopen(args[argIndex + 1], "wb");
	if (fp == NULL || fseek(fp, 0, SEEK_SET) != 0) {
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
  /* {
    FILE* fp = fopen("R:\\working\\log.txt", "a+");
    fprintf(fp, "write %s and len=%d, re=%d\n", args[argIndex + 1], strJSFormat.size() * sizeof(RealJSFormatter::Char), re);
    fclose(fp);
  } //*/
}

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
