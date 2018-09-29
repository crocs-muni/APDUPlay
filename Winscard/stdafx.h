// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#include <memory>
#include <algorithm>
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

//#include <afxwin.h>         // MFC core and standard components
//#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
//#include <afxole.h>         // MFC OLE classes
//#include <afxodlgs.h>       // MFC OLE dialog classes
//#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
//#include <afxdb.h>			// MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
//#include <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#ifdef _WIN32
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif
#else
#define FALSE 0
#endif

#ifndef _AFX_NO_AFXCMN_SUPPORT
//#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "../Shared/globals.h"
#include "../Shared/status.H"
#if defined(_WIN32)
#include "socket.h"
#endif

#include <list>
#include <unordered_map>
//using namespace std;

typedef void* PTR;

#if defined (UNICODE) && defined (_WIN32)
typedef int(*s)(char_type*, size_t,const char_type*, ...);
static s type_snprint = _snwprintf;
#else 
typedef int(*s)(char_type*, size_t, const char_type*, ...);
static s type_snprint = snprintf;
#endif


#pragma warning(disable : 4996) // for Visual Studio: Microsoft renamed std::snprintf to _snprintf
                                // this should remove the warnings

//This function is used to format std::string with arguments like with printf
template<typename ... Args>
string_type string_format(const string_type& format, Args ... args) {
	size_t size = type_snprint(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	std::unique_ptr<char_type[]> buf(new char_type[size]);
	type_snprint(buf.get(), size, format.c_str(), args ...);
	return string_type(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

typedef std::list<std::string>       ls;
typedef std::list<std::wstring>      lws;

#define MAX_APDU_LENGTH     300 
#define OFFSET_CDATA        5

typedef struct _APDU_BUFFER {
    BYTE    buffer[MAX_APDU_LENGTH];
} APDU_BUFFER;

typedef std::list<APDU_BUFFER>    lab;


#define INPUT_APDU      1
#define OUTPUT_APDU     0

#define CLA_ELEM        0
#define INS_ELEM        1
#define P1_ELEM         2
#define P2_ELEM         3
#define LC_ELEM         4
#define LE_ELEM         299

typedef struct _APDU_SINGLE_RULE {
    bool    valid = false;          // true when used
    int     history;        // number of target apdu in history (current == 0)
    int     apduDirection;  // 1 .. input, 0... output
    int     element;        // type of refered element, also equal to ordinary number of byte in apdu stream
    BYTE    value;          // expected value of given element
    
    _APDU_SINGLE_RULE() {
        clear();
    }
    void clear() {
        valid = false;
        history = 0;
        element = 0;
        value = 0x00;
    }
} APDU_SINGLE_RULE;

typedef std::list<APDU_SINGLE_RULE>   lasr;


typedef struct _APDU_RULE {
	string_type ruleName;
    int     usage;
    int     direction;      // apdu direction, 1... input apdu, 0...output apdu 
    lasr    matchRules;    
    lasr    actionRules;    
    int     msDelay;        // delay in ms before continue     
    
    _APDU_RULE() {
        clear();
    }
    void clear() {
        msDelay = 0;
        ruleName = _CONV("");
        usage = 0;
        direction = INPUT_APDU;
        lasr::iterator   iter;
        for (iter = matchRules.begin(); iter != matchRules.end(); iter++) {
            iter->clear();    
        }
        for (iter = actionRules.begin(); iter != actionRules.end(); iter++) {
            iter->clear();    
        }
    }
} APDU_RULE;

typedef std::list<APDU_RULE>     lar;
   
typedef std::list<PTR>           lptr;


#if defined (_WIN32)
#define REMOTE_READER_PREFIX_DEFAULT	"Simona"

typedef struct _REMOTE_CONFIG {
    BOOL            bRedirect;
	string_type     IP;
	string_type     port;
	string_type     cfgScript;
	string_type		remoteReaderPrefix; // prefix of reader name signalizing remote reader. If matches, all calls are redirected to remote proxy
	string_type		remoteTag;			// personalization remote tag to be interpreted by remote proxy for personalization of proxy actions 
    BYTE            measureApdu[255];    
    BYTE            measureApduLen;     // number of used bytes from measureApdu array
    int             measureApduByteCounter;     // number of incoming apdu bytes before the measurement is run - part of get params 1 xx 0 command
    int             measureApduByteDelay;       // delay in ms after start apdu triggered    
    SocketClient*   pSocket;
    int             baseReadOffset;
    int             readRatio;
    DWORD           numSamples;
    BOOL            sampleReaded;
	DWORD			nextCommandID;		// next unique command ID (checked on response)
    
    _REMOTE_CONFIG(void) {
        clear();
    }

    void clear() {
        bRedirect = FALSE;
        IP = _CONV("");
        port = _CONV("");
        cfgScript = _CONV("");
		remoteReaderPrefix = _CONV(REMOTE_READER_PREFIX_DEFAULT);
        memset(measureApdu, 0, sizeof(measureApdu));
        measureApduLen = 0;
        measureApduByteCounter = 0;
        measureApduByteDelay = 0;
        pSocket = NULL;
        baseReadOffset = 0;
        readRatio = 1;
        numSamples = 1000;
        sampleReaded = FALSE;
		nextCommandID = 1;
    }
} REMOTE_CONFIG;
#endif

typedef struct _WINSCARD_CONFIG {
    BOOL    bAUTO_REQUEST_DATA = FALSE;
    BOOL    bFORCE_CONNECT_SHARED_MODE = FALSE;
    BOOL    bFORCE_APDU_NONZERO_INPUT_DATA = FALSE;
    BOOL    bLOG_EXCHANGED_APDU = FALSE;
    BOOL    bMODIFY_APDU_BY_RULES = FALSE;
    BOOL    bLOG_WRITE_DESCRIPTION = FALSE;
    string_type sREADER_ORDERED_FIRST;
	string_type sVIRTUAL_READERS_STATIC;	// virtual readers loaded from configuration file (always shown)
	string_type sVIRTUAL_READERS;			// Virtual readers loaded from cfg file + dynamically from remote proxy
	string_type sLOG_BASE_PATH;
    
    _WINSCARD_CONFIG(void) {
        clear();
    }

    void clear() {
    
#ifdef TRAFFIC_LOGGING    
    // DEFAULT STANDARD LOGGING VERSION
    // NOTE: SENSITIVE DATA MAY BE LOGGED, DO NOT USE FOR RELEASE WORK
        bAUTO_REQUEST_DATA = FALSE;           // DEFAULT: FALSE, SET TO TRUE IF APPLICATION IS NOT ABLE TO HANDLE GET DATA (00 0c 00 00 lc) COMMAND ON ITS OWN 
        bFORCE_CONNECT_SHARED_MODE = FALSE;   // DEFAULT: FALSE, SET TO TRUE IF SOME APPLICATION IS BLOCKING CARD
        bFORCE_APDU_NONZERO_INPUT_DATA = FALSE;   // DEFAULT: FALSE, SET TO TRUE IF APPLET HAVE PROBLEM TO RETURN DATA (apdu.setOutgoingAndSend()) WHEN NO PREVIOUS setIncomingAndReceive() WAS CALLED. 
        bLOG_EXCHANGED_APDU = TRUE;   // DEFAULT: FALSE, SET TO TRUE IF LOGGING OF APDU DATA IS REQUIRED
        bMODIFY_APDU_BY_RULES = TRUE;   // DEFAULT: FALSE, SET TO TRUE . 
        sREADER_ORDERED_FIRST = _CONV("");
		sLOG_BASE_PATH = _CONV("");
#else
    // DEFAULT RELEASE VERSION FOR COMMON USAGE WITHOUT LOGGING
        bAUTO_REQUEST_DATA = FALSE;           // DEFAULT: FALSE, SET TO TRUE IF APPLICATION IS NOT ABLE TO HANDLE GET DATA (00 0c 00 00 lc) COMMAND ON ITS OWN 
        bFORCE_CONNECT_SHARED_MODE = FALSE;   // DEFAULT: FALSE, SET TO TRUE IF SOME APPLICATION IS BLOCKING CARD
        bFORCE_APDU_NONZERO_INPUT_DATA = FALSE;   // DEFAULT: FALSE, SET TO TRUE IF APPLET HAVE PROBLEM TO RETURN DATA (apdu.setOutgoingAndSend()) WHEN NO PREVIOUS setIncomingAndReceive() WAS CALLED. 
        bLOG_EXCHANGED_APDU = FALSE;   // DEFAULT: FALSE, SET TO TRUE IF LOGGING OF APDU DATA IS REQUIRED
        bMODIFY_APDU_BY_RULES = FALSE;   // DEFAULT: FALSE, SET TO TRUE . 
        sREADER_ORDERED_FIRST = "";
		sVIRTUAL_READERS = "";
		sLOG_BASE_PATH = "";
#endif
    }
} WINSCARD_CONFIG;


#if defined(_WIN32)

#ifndef RDTSC_H
#define RDTSC_H

#if defined( _MSC_VER )
#if defined( _WIN64 )
#include <intrin.h>
#pragma intrinsic( __rdtsc )
__inline volatile unsigned long long read_tsc(void)
{
    return __rdtsc();
}
#elif defined(_WIN32)
__inline volatile unsigned long long read_tsc(void)
{   unsigned long lo, hi;
    __asm cpuid __asm rdtsc __asm mov [lo],eax __asm mov [hi],edx ; 
    return (((unsigned long long)hi) << 32) + lo;
}
#endif

#else
#error A high resolution timer is not available
#endif
#endif
#endif
