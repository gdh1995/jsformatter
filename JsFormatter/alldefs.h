#ifndef _JSFORMATTER_ALL_DEFS_

#define VERSION_VALUE "1.16.5.0"
#define VERSION_DIGITALVALUE 1, 16, 5, 0

#define JS_FORMATTER_VERSION_VALUE "0.9"
#define JS_FORMATTER_VERSION_DIGITALVALUE 0, 9, 0, 0

//*/
#ifdef UNICODE
#undef UNICODE
#endif
#ifdef _UNICODE
#undef _UNICODE
#endif
#ifndef _MBCS
#define _MBCS
#endif
/*/
#ifdef _MBCS
#undef _MBCS
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
// */

#define __TEST__ 0


#endif
