#ifndef GLOBALS_H
#define GLOBALS_H


#ifdef __linux__
    #include <wintypes.h>
#endif 

#include <string>
#include <cstring>


//#define UNICODE

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif

#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#endif

#if defined (UNICODE) && defined (_WIN32)
typedef std::wstring string_type;
typedef std::wifstream ifstream_type;
typedef std::wofstream ofstream_type;
typedef std::wfstream fstream_type;
typedef wchar_t char_type;
#define _CONV(x) L ##x
static FILE*(*type_fopen)(const char_type*, const char_type*) = _wfopen;
static size_t(*type_length)(const char_type*) = wcslen;
static char_type*(*type_copy)(char_type*, const char_type*) = wcscpy;
static int(*type_compare)(const char_type*, const char_type*) = wcscmp;
static long(*type_to_int)(const char_type*, char_type**, int) = wcstol;
static char_type*(*type_cat)(char_type*, const char_type*) = wcscat;
static errno_t(*type_path_split)(const char_type*, char_type*, size_t, char_type*, size_t, char_type*, size_t, char_type*, size_t) = _wsplitpath_s;
#else 
typedef std::string string_type;
typedef std::ifstream ifstream_type;
typedef std::ofstream ofstream_type;
typedef std::fstream fstream_type;
typedef char char_type;
#define _CONV(x) x
static FILE*(*type_fopen)(const char_type*, const char_type*) = fopen;
static  size_t(*type_length)(const char_type*) = strlen;
static char_type*(*type_copy)(char_type*, const char_type*) = strcpy;
static int(*type_compare)(const char_type*, const char_type*) = strcmp;
static long(*type_to_int)(const char_type*, char_type**, int) = strtol;
static char_type*(*type_cat)(char_type*, const char_type*) = strcat;
#if defined(_WIN32)
static errno_t(*type_path_split)(const char_type*, char_type*, size_t, char_type*, size_t, char_type*, size_t, char_type*, size_t) = _splitpath_s;
#endif
#endif

#ifndef HIGHBYTE
    #define HIGHBYTE(x)  x >> 8 
#endif

#ifndef LOWBYTE
    #define LOWBYTE(x)   x & 0xFF 
#endif

#define FILETIME_TO_SECOND                      10000000 

#define MAX_INI_VALUE_CHAR                      1000
#define MAX_INI_VALUE_CHAR_LARGE                1000000

#define FLAG_OS_WIN9X                           1
#define FLAG_OS_WINNT                           2
    
//#define _NO_AVAIL_CARD

#ifndef CARDAPDU_DEFINED
#define CARDAPDU_DEFINED

typedef struct _CARDAPDU {
	unsigned char   cla;
	unsigned char   ins;
	unsigned char   p1;
	unsigned char   p2;
	unsigned char   lc;
	unsigned char   le;
	unsigned char   DataIn[256];
	unsigned char   DataOut[256];
	unsigned short  sw;
	   
	_CARDAPDU(void) {
	    clear();
	}
	void clear() {
        sw = cla = ins = p1 = p2 = lc = le = (BYTE) 0;
        memset(DataIn, 0, 256);
        memset(DataOut, 0, 256);
	}
} CARDAPDU;
#endif

#if defined(_WIN32)
typedef struct _INT_DATA_BLOB {
    int*    pData;
    DWORD   dwMaxLen;
    DWORD   dwActLen;

    _INT_DATA_BLOB(void) {
        pData = NULL;
        dwMaxLen = 0;
        dwActLen = 0;
    }

} INT_DATA_BLOB;

typedef struct _INT64_DATA_BLOB {
    __int64*    pData;
    DWORD   dwMaxLen;
    DWORD   dwActLen;

    _INT64_DATA_BLOB(void) {
        pData = NULL;
        dwMaxLen = 0;
        dwActLen = 0;
    }

} INT64_DATA_BLOB;

typedef struct _BYTE_DATA_BLOB {
    BYTE*    pData;
    DWORD   dwMaxLen;
    DWORD   dwActLen;

    _BYTE_DATA_BLOB(void) {
        pData = NULL;
        dwMaxLen = 0;
        dwActLen = 0;
    }

} BYTE_DATA_BLOB;

typedef struct _FLOAT_DATA_BLOB {
    float*  pData;
    DWORD   dwMaxLen;
    DWORD   dwActLen;

    _FLOAT_DATA_BLOB(void) {
        pData = NULL;
        dwMaxLen = 0;
        dwActLen = 0;
    }

} FLOAT_DATA_BLOB;
#endif
#define SLOT_ANY_AVAILABLE	-1

#endif