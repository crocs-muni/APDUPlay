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
typedef struct _SCSAT04_CONFIG {
    BOOL            bRedirect;
	string_type     IP;
	string_type     port;
	string_type     cfgScript;
    BYTE            measureApdu[255];    
    BYTE            measureApduLen;     // number of used bytes from measureApdu array
    int             measureApduByteCounter;     // number of incoming apdu bytes before the measurement is run - part of get params 1 xx 0 command
    int             measureApduByteDelay;       // delay in ms after start apdu triggered    
    SocketClient*   pSocket;
    int             baseReadOffset;
    int             readRatio;
    DWORD           numSamples;
    BOOL            sampleReaded;
    
    _SCSAT04_CONFIG(void) {
        clear();
    }

    void clear() {
        bRedirect = FALSE;
        IP = _CONV("");
        port = _CONV("");
        cfgScript = _CONV("");
        memset(measureApdu, 0, sizeof(measureApdu));
        measureApduLen = 0;
        measureApduByteCounter = 0;
        measureApduByteDelay = 0;
        pSocket = NULL;
        baseReadOffset = 0;
        readRatio = 1;
        numSamples = 1000;
        sampleReaded = FALSE;
    }
} SCSAT04_CONFIG;
#endif

typedef struct _WINSCARD_CONFIG {
    BOOL    bAUTO_REQUEST_DATA = FALSE;
    BOOL    bFORCE_CONNECT_SHARED_MODE = FALSE;
    BOOL    bFORCE_APDU_NONZERO_INPUT_DATA = FALSE;
    BOOL    bLOG_EXCHANGED_APDU = FALSE;
    BOOL    bMODIFY_APDU_BY_RULES = FALSE;
    BOOL    bLOG_FUNCTIONS_CALLS = FALSE;
    BOOL    bLOG_WRITE_DESCRIPTION = FALSE;
    string_type sREADER_ORDERED_FIRST;
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
        bLOG_FUNCTIONS_CALLS = TRUE;   // DEFAULT: FALSE, SET TO TRUE . 
        sREADER_ORDERED_FIRST = _CONV("");
		sLOG_BASE_PATH = _CONV("");
#else
    // DEFAULT RELEASE VERSION FOR COMMON USAGE WITHOUT LOGGING
        bAUTO_REQUEST_DATA = FALSE;           // DEFAULT: FALSE, SET TO TRUE IF APPLICATION IS NOT ABLE TO HANDLE GET DATA (00 0c 00 00 lc) COMMAND ON ITS OWN 
        bFORCE_CONNECT_SHARED_MODE = FALSE;   // DEFAULT: FALSE, SET TO TRUE IF SOME APPLICATION IS BLOCKING CARD
        bFORCE_APDU_NONZERO_INPUT_DATA = FALSE;   // DEFAULT: FALSE, SET TO TRUE IF APPLET HAVE PROBLEM TO RETURN DATA (apdu.setOutgoingAndSend()) WHEN NO PREVIOUS setIncomingAndReceive() WAS CALLED. 
        bLOG_EXCHANGED_APDU = FALSE;   // DEFAULT: FALSE, SET TO TRUE IF LOGGING OF APDU DATA IS REQUIRED
        bMODIFY_APDU_BY_RULES = FALSE;   // DEFAULT: FALSE, SET TO TRUE . 
        bLOG_FUNCTIONS_CALLS = FALSE;   // DEFAULT: FALSE, SET TO TRUE . 
        sREADER_ORDERED_FIRST = "";
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

static string_type SCSAT_MEASURE_SECTION = _CONV("SCSAT_MEASURE_BASIC");
static string_type SCSAT_MEASURE_SECTION_EXT = _CONV("SCSAT_MEASURE_EXT");
static string_type SCSAT_MEASURE_SECTION_SAMPLES = _CONV("SCSAT_MEASURE_SAMPLES");
static string_type SCSAT_MEASURE_DATETIME = _CONV("DATETIME");
static string_type SCSAT_MEASURE_CARDATR = _CONV("CARDATR");
static string_type SCSAT_MEASURE_CARDNAME = _CONV("CARDNAME");
static string_type SCSAT_MEASURE_NOTE = _CONV("NOTE");
static string_type SCSAT_MEASURE_COMMLOG = _CONV("COMMLOG");
static string_type SCSAT_MEASURE_CFGSCRIPT = _CONV("CFGSCRIPT");
static string_type SCSAT_MEASURE_STARTAPDU = _CONV("STARTAPDU");
static string_type SCSAT_MEASURE_SAMPLINGFREQUENCY = _CONV("SAMPLINGFREQUENCY");
static string_type SCSAT_MEASURE_BASEOFFSET = _CONV("BASEOFFSET");
static string_type SCSAT_MEASURE_BASESHIFT = _CONV("BASESHIFT");
static string_type SCSAT_MEASURE_SYNDROMS = _CONV("SYNDROMS");
static string_type SCSAT_MEASURE_MICROSHIFTS = _CONV("MICROSHIFTS");
static string_type SCSAT_MEASURE_POWERTRACE = _CONV("POWERTRACE");
static string_type SCSAT_MEASURE_NUMSAMPLES = _CONV("NUMSAMPLES");

#define SCSAT_MAX_SAMPLING_FREQUENCY 100
#define SCSAT_SOCKET_SHORT_TIMEOUT 3

static string_type SCSAT_GET_APDU = _CONV("get apdu");
static string_type SCSAT_GET_APDU_FAIL = _CONV("get apdu fail");
static string_type SCSAT_GET_RESET = _CONV("get reset");

//static string_type SCSAT_MEASURE_SECTION = _CONV("SCSAT_MEASURE_BASIC");
//static string_type SCSAT_MEASURE_SECTION_EXT = _CONV("SCSAT_MEASURE_EXT");
static string_type SCSAT_MEASURE_SECTION_EXT1 = _CONV("SCSAT_MEASURE_EXT_1");
static string_type SCSAT_MEASURE_SECTION_EXT2 = _CONV("SCSAT_MEASURE_EXT_2");
static string_type SCSAT_MEASURE_SECTION_EXT3 = _CONV("SCSAT_MEASURE_EXT_3");
static string_type SCSAT_MEASURE_SECTION_EXT4 = _CONV("SCSAT_MEASURE_EXT_4");
static string_type SCSAT_MEASURE_SECTION_EXT5 = _CONV("SCSAT_MEASURE_EXT_5");
static string_type SCSAT_MEASURE_SECTION_EXT6 = _CONV("SCSAT_MEASURE_EXT_6");
static string_type SCSAT_MEASURE_SECTION_EXT7 = _CONV("SCSAT_MEASURE_EXT_7");
static string_type SCSAT_MEASURE_SECTION_EXT8 = _CONV("SCSAT_MEASURE_EXT_8");

//static string_type SCSAT_MEASURE_SECTION_SAMPLES = _CONV("SCSAT_MEASURE_SAMPLES");
//static string_type SCSAT_MEASURE_DATETIME = _CONV("DATETIME");
//static string_type SCSAT_MEASURE_CARDATR = _CONV("CARDATR");
//static string_type SCSAT_MEASURE_CARDNAME = _CONV("CARDNAME");
//static string_type SCSAT_MEASURE_NOTE = _CONV("NOTE");
//static string_type SCSAT_MEASURE_COMMLOG = _CONV("COMMLOG");
//static string_type SCSAT_MEASURE_CFGSCRIPT = _CONV("CFGSCRIPT");
//static string_type SCSAT_MEASURE_STARTAPDU = _CONV("STARTAPDU");
//static string_type SCSAT_MEASURE_SAMPLINGFREQUENCY = _CONV("SAMPLINGFREQUENCY");
//static string_type SCSAT_MEASURE_BASEOFFSET = _CONV("BASEOFFSET");
//static string_type SCSAT_MEASURE_BASESHIFT = _CONV("BASESHIFT");
//static string_type SCSAT_MEASURE_SYNDROMS = _CONV("SYNDROMS");
//static string_type SCSAT_MEASURE_MICROSHIFTS = _CONV("MICROSHIFTS");
//static string_type SCSAT_MEASURE_POWERTRACE = _CONV("POWERTRACE");
//static string_type SCSAT_MEASURE_NUMSAMPLES = _CONV("NUMSAMPLES");
static string_type SCSAT_MEASURE_SAMPLEUNIQUEID = _CONV("SAMPLEUNIQUEID");
static string_type SCSAT_MEASURE_APDUDATA = _CONV("APDUDATA");
static string_type SCSAT_MEASURE_SAVEBINARY = _CONV("SAVEBINARY");

#if defined (_WIN32)
typedef struct _SCSAT_MEASURE_INFO {
	string_type     dateTime;
	string_type     cardATR;
	string_type     cardName;
	string_type     cfgScript;
	string_type     commLog;
	string_type     syndroms;       // classified syndroms format into single line string 
	string_type     microShifts;    // synchronization microShifts for subparts of trace format into single line string
	string_type     note;           // user supplied note
	string_type     startAPDU;
	string_type     apduData;       // substring of startAPDU
    int         frequency;
    int         baseOffset;
    int         baseShift;
    int         baseLevel;
    int         numSamples;
	__int64     sampleUniqueID;
	BOOL		bSaveBinary;
    
    _SCSAT_MEASURE_INFO() {
        clear();
    }
    void clear() {
        cardATR = _CONV("");
        cardName = _CONV("");
        clearMeasure();
		sampleUniqueID=0;
    }
    void clearMeasure() {
        dateTime = _CONV("");
        cfgScript = _CONV("");
        startAPDU = _CONV("");
		apduData = _CONV("");
        commLog = _CONV("");
        syndroms = _CONV("");
        microShifts = _CONV("");
        note = _CONV("");
        frequency = 0;
        baseOffset = 0;
        baseShift = 0;
        numSamples = 0;
        baseLevel = -1;
    }

int formatToString(string_type* pResult) {
		string_type value;
		// remove all endlines and replace by ';' in multiline strings
		
		cfgScript.erase(remove(cfgScript.begin(), cfgScript.end(), '\r'), cfgScript.end());
		replace(cfgScript.begin(), cfgScript.end(), '\n', ';');
		commLog.erase(remove(commLog.begin(), commLog.end(), '\r'), commLog.end());
		replace(commLog.begin(), commLog.end(), '\n', ';');

		//cfgScript.Replace("\r", ""); cfgScript.Replace("\n", ";");
		//commLog.Replace("\r", ""); commLog.Replace("\n", ";");

		//create INI style section
		*pResult = string_format(_CONV("[%s]\r\n"), SCSAT_MEASURE_SECTION);
		*pResult += string_format(_CONV("%s=%lld\r\n"), SCSAT_MEASURE_SAMPLEUNIQUEID, sampleUniqueID);
		*pResult += string_format(_CONV("%s=%s\r\n"), SCSAT_MEASURE_DATETIME, dateTime);
		*pResult += string_format(_CONV("%s=%s\r\n"), SCSAT_MEASURE_CARDATR, cardATR);
		*pResult += string_format(_CONV("%s=%s\r\n"), SCSAT_MEASURE_CARDNAME, cardName);
		*pResult += string_format(_CONV("%s=%s\r\n"), SCSAT_MEASURE_NOTE, note);
		*pResult += string_format(_CONV("%s=%s\r\n"), SCSAT_MEASURE_CFGSCRIPT, cfgScript);
		*pResult += string_format(_CONV("%s=%s\r\n"), SCSAT_MEASURE_STARTAPDU, startAPDU);
		*pResult += string_format(_CONV("%s=%s\r\n"), SCSAT_MEASURE_COMMLOG, commLog);
		*pResult += string_format(_CONV("%s=%d\r\n"), SCSAT_MEASURE_SAMPLINGFREQUENCY, frequency);
		*pResult += string_format(_CONV("%s=%d\r\n"), SCSAT_MEASURE_BASEOFFSET, baseOffset);
		*pResult += string_format(_CONV("%s=%d\r\n"), SCSAT_MEASURE_BASESHIFT, baseShift);
		*pResult += string_format(_CONV("%s=%s\r\n"), SCSAT_MEASURE_SYNDROMS, syndroms);
		*pResult += string_format(_CONV("%s=%s\r\n"), SCSAT_MEASURE_MICROSHIFTS, microShifts);
		*pResult += string_format(_CONV("%s=%d\r\n"), SCSAT_MEASURE_NUMSAMPLES, numSamples);
		*pResult += string_format(_CONV("[%s_1]\r\n%s=\r\n"), SCSAT_MEASURE_SECTION_EXT, SCSAT_MEASURE_NOTE);
		*pResult += string_format(_CONV("%s=%s\r\n"), SCSAT_MEASURE_APDUDATA, apduData);
		*pResult += string_format(_CONV("%s=%s\r\n"), SCSAT_MEASURE_SAVEBINARY, bSaveBinary ? "1" : "0");
		*pResult += string_format(_CONV("[%s_2]\r\n%s=\r\n"), SCSAT_MEASURE_SECTION_EXT, SCSAT_MEASURE_NOTE);
		*pResult += string_format(_CONV("[%s_3]\r\n%s=\r\n"), SCSAT_MEASURE_SECTION_EXT, SCSAT_MEASURE_NOTE);
		*pResult += string_format(_CONV("[%s_4]\r\n%s=\r\n"), SCSAT_MEASURE_SECTION_EXT, SCSAT_MEASURE_NOTE);
		*pResult += string_format(_CONV("[%s_5]\r\n%s=\r\n"), SCSAT_MEASURE_SECTION_EXT, SCSAT_MEASURE_NOTE);
		*pResult += string_format(_CONV("[%s_6]\r\n%s=\r\n"), SCSAT_MEASURE_SECTION_EXT, SCSAT_MEASURE_NOTE);
		*pResult += string_format(_CONV("[%s_7]\r\n%s=\r\n"), SCSAT_MEASURE_SECTION_EXT, SCSAT_MEASURE_NOTE);
		*pResult += string_format(_CONV("[%s_8]\r\n%s=\r\n"), SCSAT_MEASURE_SECTION_EXT, SCSAT_MEASURE_NOTE);
		*pResult += string_format(_CONV("[%s_9]\r\n%s=\r\n"), SCSAT_MEASURE_SECTION_EXT, SCSAT_MEASURE_NOTE);
		*pResult += string_format(_CONV("[%s_9]\r\n%s=\r\n"), SCSAT_MEASURE_SECTION_EXT, SCSAT_MEASURE_NOTE);
		*pResult += string_format(_CONV("[%s]\r\n%s=\r\n"), SCSAT_MEASURE_SECTION_SAMPLES, SCSAT_MEASURE_POWERTRACE);
		return 0;
	}
void copy(_SCSAT_MEASURE_INFO* pTemplate) {
		cardATR = pTemplate->cardATR;
		cardName = pTemplate->cardName;
		dateTime = pTemplate->dateTime;
		cfgScript = pTemplate->cfgScript;
		startAPDU = pTemplate->startAPDU;
		apduData = pTemplate->apduData;
		commLog = pTemplate->commLog;
		syndroms = pTemplate->syndroms;
		microShifts = pTemplate->microShifts;
		note = pTemplate->note;
		frequency = pTemplate->frequency;
		baseOffset = pTemplate->baseOffset;
		baseShift = pTemplate->baseShift;
		numSamples = pTemplate->numSamples;
		sampleUniqueID = pTemplate->sampleUniqueID;
		bSaveBinary = pTemplate->bSaveBinary;
	}
} SCSAT_MEASURE_INFO;

typedef struct _SCSAT04_INFO {
    int     baseOffset;
    int     readRatio;
    
    _SCSAT04_INFO(void) {
        baseOffset = 0; 
        readRatio = 1;   
    }
} SCSAT04_INFO;

class SAMPLE_PLOT {
public:
    INT_DATA_BLOB   dataBlob;
    INT_DATA_BLOB   microshiftsDataBlob;
	string_type     dataFilePath;
    INT_DATA_BLOB   busBlob;
	string_type     busFilePath;
    int             sampleFileIndex;  // used only when data/busFilePath is constructed as 'dataoutXX.dat'
    DWORD           flags;
    BOOL            bShow;
    SCSAT04_INFO    scsat04Info;
    SCSAT_MEASURE_INFO  measureInfo;

public:
    SAMPLE_PLOT(void) {
        dataFilePath = _CONV("");
        busFilePath = _CONV("");
        bShow = TRUE;
        flags = 0;
        sampleFileIndex = -1;
        measureInfo.clear();
    }
};
#endif
