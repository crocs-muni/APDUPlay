#ifndef GLOBALS_H
#define GLOBALS_H


#ifdef __linux__
    #include <wintypes.h>
#endif 

#include <cstring>

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

#define SLOT_ANY_AVAILABLE	-1

#endif