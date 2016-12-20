#ifndef INTERFACE_H
#define INTERFACE_H
/*
This is of APDUPlay project for interception and manipulation of PC/SC APDU packets for smart cards.
See http://www.fi.muni.cz/~xsvenda/apduinspect.html for more information
Copyright (C) 2011  Petr Svenda <petr@svenda.com>

LICENSE TERMS

The free distribution and use of this software in both source and binary
form is allowed (with or without changes) provided that:

1. distributions of this source code include the above copyright
notice, this list of conditions and the following disclaimer;

2. distributions in binary form include the above copyright
notice, this list of conditions and the following disclaimer
in the documentation and/or other associated materials;

3. the copyright holder's name is not used to endorse products
built using this software without specific written permission.

ALTERNATIVELY, provided that this notice is retained in full, this product
may be distributed under the terms of the GNU General Public License (GPL),
in which case the provisions of the GPL apply INSTEAD OF those given above.

DISCLAIMER

This software is provided 'as is' with no explicit or implied warranties
in respect of its properties, including, but not limited to, correctness
and/or fitness for purpose.

Please, report any bugs to author <petr@svenda.com>
/**/


// Winscard.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <stdio.h>
#define WINSCARDDATA __declspec(dllexport)
#include <winscard.h>
#include "Winscard.h"
#include "CommonFnc.h"
#define STDCALL __stdcall
#include <time.h>
#include "socket.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef WINSCARDAPI
#define SCard WINSCARDAPI
#else 
#define SCard
#endif

#ifdef __linux__
#include <dlfcn.h>
#include "wintypes.h"
typedef wchar_t WCHAR;
typedef wchar_t* LPWSTR;
typedef const wchar_t* LPCWSTR;
#define TRUE 1
#define FALSE 0
#define NULL 0
#define STDCALL
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#endif

/*
BOOL    bAUTO_REQUEST_DATA = TRUE;           // DEFAULT: FALSE, SET TO TRUE IF APPLICATION IS NOT ABLE TO HANDLE GET DATA (00 0c 00 00 lc) COMMAND ON ITS OWN
BOOL    bFORCE_CONNECT_SHARED_MODE = TRUE;   // DEFAULT: FALSE, SET TO TRUE IF SOME APPLICATION IS BLOCKING CARD
BOOL    bFORCE_APDU_NONZERO_INPUT_DATA = TRUE;   // DEFAULT: FALSE, SET TO TRUE IF APPLET HAVE PROBLEM TO RETURN DATA (apdu.setOutgoingAndSend()) WHEN NO PREVIOUS setIncomingAndReceive() WAS CALLED.
BOOL    bLOG_EXCHANGED_APDU = TRUE;   // DEFAULT: FALSE, SET TO TRUE IF LOGGING OF APDU DATA IS REQUIRED
BOOL    bMODIFY_APDU_BY_RULES = TRUE;   // DEFAULT: FALSE, SET TO TRUE .
BOOL    bLOG_FUNCTIONS_CALLS = TRUE;   // DEFAULT: FALSE, SET TO TRUE .
/**/

// The one and only CWinscardApp object

CWinscardApp theApp;
/*
extern SCard (.*?) STDCALL.*?(SCard.*?)\((.*?)\);
static SCard \1 (STDCALL *Original_\2)
\3
);
/**/
#pragma warning(disable:4996)   

static string_type RULE_FILE = _CONV("winscard_rules.txt");
static string_type WINSCARD_RULES_LOG = _CONV("winscard_rules_log.txt");
static string_type WINSCARD_LOG = _CONV("winscard_log.txt");


#define SCSAT_SOCKET_TIMEOUT            5
#define SCSAT_SOCKET_LONG_TIMEOUT       20
static string_type SCSAT_SOCKET_ENDSEQ = _CONV("@@");

//#define HANDLE_VIRTUAL_CARD             0xABADBABE
#define HANDLE_VIRTUAL_CARD             0x1

//BYTE    START_APDU[] = {0xB0,  0x05, 0x01, 0x00, 0x01};
BYTE    START_APDU[] = { 0xB0, 0x01, 0x01, 0x00, 0x0C };

BYTE    PIN_COUNTER_APDU[] = { 0xB0,  0x05, 0x01, 0x00, 0x01 };
BYTE    GET_APDU1[] = { 0x00, 0xC0, 0x00, 0x00 };
BYTE    GET_APDU2[] = { 0xC0, 0xC0, 0x00, 0x00 };

//#define VIRT_READER_NAME        "VirtOpenPGP"
#define VIRT_READER_NAME        ""
#define VIRTUAL_READERS_LEN     strlen(VIRT_READER_NAME)

/* ******************************************************************************* */

#ifdef __linux__
    static void* hOriginal = NULL;
#else
    static HANDLE  hOut = 0;
    static HMODULE hOriginal = 0;
#endif


int apduCounter = 0;

/* ******************************************************************************* */

/* The following values variables MUST be defined here, but MUST NOT be referenced
in this or any other program module. The DEF file is set to forward their linkage
to the "original.dll". If we need the data that these variables should be pointing
to, we must GetProcAddress on "original.dll" and use the data there.
*/

#if defined(_WIN32)
const SCARD_IO_REQUEST g_rgSCardT0Pci, g_rgSCardT1Pci, g_rgSCardRawPci;

/* Just make sure we don't accidentally use the wrong global variable... */
#define g_rgSCardT0Pci   DONT_USE_ME_g_rgSCardT0Pci
#undef  SCARD_PCI_T0
#define SCARD_PCI_T0     DONT_USE_ME_SCARD_PCI_T0
#define g_rgSCardT1Pci   DONT_USE_ME_g_rgSCardT1Pci
#undef  SCARD_PCI_T1
#define SCARD_PCI_T1     DONT_USE_ME_SCARD_PCI_T1
#define g_rgSCardTRawPci DONT_USE_ME_g_rgSCardTRawPci
#undef  SCARD_PCI_RAW
#define SCARD_PCI_RAW    DONT_USE_ME_SCARD_PCI_RAW
#endif

/* ******************************************************************************* */

int compareWithNoCase(const char_type* str1,const char_type* str2) {
	char_type *str1_2 = new char_type[type_length(str1)];
	char_type *str2_2 = new char_type[type_length(str2)];
	type_copy(str1_2, str1);
	type_copy(str2_2, str2);
	toupper(*str1_2);
	toupper(*str2_2);
	int result = type_compare(str1_2, str2_2);
	delete[] str1_2;
	delete[] str2_2;
	return result;
}

/*int compareWithNoCase(const char_type* str1, const char_type* str2) {
	return 0;
}*/

void DumpMemory(LPCBYTE location, DWORD length) {
	/*
	DWORD i, written;
	char *hexDigit = "0123456789ABCDEF";
	char *space = " ", *crlf = "\r\n";
	char *delim = "#";

	//    WriteFile( hOut, space, lstrlen(space), &written, NULL );
	for ( i=0; i<length; i++ ) {
	if (i > 0) {
	WriteFile( hOut, space, lstrlen(space), &written, NULL );
	}
	WriteFile( hOut, (hexDigit+((location[i]>>4)&0x0F)), 1, &written, NULL );
	WriteFile( hOut, (hexDigit+((location[i]>>0)&0x0F)), 1, &written, NULL );
	}

	WriteFile( hOut, delim, lstrlen(delim), &written, NULL );
	WriteFile( hOut, crlf, lstrlen(crlf), &written, NULL );
	/**/
	string_type message;
	CCommonFnc::BYTE_ConvertFromArrayToHexString((BYTE*)location, length, &message);
	CCommonFnc::File_AppendString(WINSCARD_LOG, message);
	CCommonFnc::File_AppendString(WINSCARD_LOG, _CONV("\r\n"));

}

static SCard LONG(STDCALL *Original_SCardEstablishContext)(
	IN  DWORD dwScope,
	IN  LPCVOID pvReserved1,
	IN  LPCVOID pvReserved2,
	OUT LPSCARDCONTEXT phContext
	);

SCard LONG STDCALL SCardEstablishContext(
	IN  DWORD dwScope,
	IN  LPCVOID pvReserved1,
	IN  LPCVOID pvReserved2,
	OUT LPSCARDCONTEXT phContext
) {
	string_type message;
	message = string_format(_CONV("SCardEstablishContext() called\n"));
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	LONG status = (*Original_SCardEstablishContext)(dwScope, pvReserved1, pvReserved2, phContext);
	//message.Format("-> hContext:0x%x\n", *phContext);
	message = string_format(_CONV("-> hContext:0x%x\n"), *phContext);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	return status;
}


static SCard LONG(STDCALL *Original_SCardReleaseContext)(
	IN      SCARDCONTEXT hContext
	);

SCard LONG STDCALL SCardReleaseContext(
	IN      SCARDCONTEXT hContext
) {
	string_type message;
	message = string_format(_CONV("SCardReleaseContext(hContext:0x%x) called\n"), hContext);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	return (*Original_SCardReleaseContext)(hContext);
}


static SCard LONG(STDCALL *Original_SCardIsValidContext)(
	IN      SCARDCONTEXT hContext
	);

SCard LONG STDCALL SCardIsValidContext(
	IN      SCARDCONTEXT hContext
) {
	string_type message;
	message = string_format(_CONV("SCardIsValidContext(hContext:0x%x) called\n"), hContext);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	return (*Original_SCardIsValidContext)(hContext);
}

static SCard LONG(STDCALL *Original_SCardCancel)(
	IN      SCARDCONTEXT hContext
	);

SCard LONG STDCALL SCardCancel(
	IN      SCARDCONTEXT hContext
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardCancel called\n"));
	return (*Original_SCardCancel)(hContext);
}

static SCard LONG(STDCALL *Original_SCardReconnect)(
	IN      SCARDHANDLE hCard,
	IN      DWORD dwShareMode,
	IN      DWORD dwPreferredProtocols,
	IN      DWORD dwInitialization,
	OUT     LPDWORD pdwActiveProtocol
	);

SCard LONG STDCALL SCardReconnect(
	IN      SCARDHANDLE hCard,
	IN      DWORD dwShareMode,
	IN      DWORD dwPreferredProtocols,
	IN      DWORD dwInitialization,
	OUT     LPDWORD pdwActiveProtocol
) {
	string_type message;
	message = string_format(_CONV("SCardReconnect(hCard:0x%x) called\n"), hCard);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	return (*Original_SCardReconnect)(hCard, dwShareMode, dwPreferredProtocols, dwInitialization, pdwActiveProtocol);
}

static SCard LONG(STDCALL *Original_SCardBeginTransaction)(
	IN      SCARDHANDLE hCard
	);

SCard LONG STDCALL SCardBeginTransaction(
	IN      SCARDHANDLE hCard
) {
	CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardBeginTransaction called\n"));
	return (*Original_SCardBeginTransaction)(hCard);
}


static SCard LONG(STDCALL *Original_SCardEndTransaction)(
	IN      SCARDHANDLE hCard,
	IN      DWORD dwDisposition
	);

SCard LONG STDCALL SCardEndTransaction(
	IN      SCARDHANDLE hCard,
	IN      DWORD dwDisposition
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardEndTransaction called\n"));
	return (*Original_SCardEndTransaction)(hCard, dwDisposition);
}

static SCard LONG(STDCALL *Original_SCardControl)(
	IN      SCARDHANDLE hCard,
	IN      DWORD dwControlCode,
	IN      LPCVOID lpInBuffer,
	IN      DWORD nInBufferSize,
	OUT     LPVOID lpOutBuffer,
	IN      DWORD nOutBufferSize,
	OUT     LPDWORD lpBytesReturned
	);

SCard LONG STDCALL SCardControl(
	IN      SCARDHANDLE hCard,
	IN      DWORD dwControlCode,
	IN      LPCVOID lpInBuffer,
	IN      DWORD nInBufferSize,
	OUT     LPVOID lpOutBuffer,
	IN      DWORD nOutBufferSize,
	OUT     LPDWORD lpBytesReturned
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardControl called\n"));
	return (*Original_SCardControl)(hCard, dwControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned);
}


static SCard LONG(STDCALL *Original_SCardGetAttrib)(
	IN SCARDHANDLE hCard,
	IN DWORD dwAttrId,
	OUT LPBYTE pbAttr,
	IN OUT LPDWORD pcbAttrLen
	);

SCard LONG STDCALL SCardGetAttrib(
	IN SCARDHANDLE hCard,
	IN DWORD dwAttrId,
	OUT LPBYTE pbAttr,
	IN OUT LPDWORD pcbAttrLen
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardGetAttrib called\n"));
	return (*Original_SCardGetAttrib)(hCard, dwAttrId, pbAttr, pcbAttrLen);
}


static SCard LONG(STDCALL *Original_SCardSetAttrib)(
	IN SCARDHANDLE hCard,
	IN DWORD dwAttrId,
	IN LPCBYTE pbAttr,
	IN DWORD cbAttrLen
	);

SCard LONG STDCALL SCardSetAttrib(
	IN SCARDHANDLE hCard,
	IN DWORD dwAttrId,
	IN LPCBYTE pbAttr,
	IN DWORD cbAttrLen
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardSetAttrib called\n"));
	return (*Original_SCardSetAttrib)(hCard, dwAttrId, pbAttr, cbAttrLen);
}

/* ******************************************************************************* */

CWinscardApp::~CWinscardApp()
{
	#ifdef __linux
		dlclose(hOriginal);
	#else
	    FreeLibrary(hOriginal);
	#endif
	// Reference to WINSCARD_LOG will fail with access to 0xfeefee (global CString WINSCARD_LOG does not exists at the time of dll release (strange))
	//	if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) CCommonFnc::File_AppendString(WINSCARD_LOG, "[end]\r\n");

	if (m_scsat04Config.pSocket != NULL) delete m_scsat04Config.pSocket;

	lptr::iterator  iter;
	for (iter = m_charAllocatedMemoryList.begin(); iter != m_charAllocatedMemoryList.end(); iter++) {
		char* ptr = (char*)*iter;
		if (ptr != NULL) delete[] ptr;
	}
	m_charAllocatedMemoryList.clear();
	for (iter = m_wcharAllocatedMemoryList.begin(); iter != m_wcharAllocatedMemoryList.end(); iter++) {
		WCHAR* ptr = (WCHAR*)*iter;
		if (ptr != NULL) delete[] ptr;
	}
	m_wcharAllocatedMemoryList.clear();
}

static SCard LONG(STDCALL *Original_SCardFreeMemory)(
	IN SCARDCONTEXT hContext,
	IN LPCVOID pvMem
	);

SCard LONG STDCALL SCardFreeMemory(
	IN SCARDCONTEXT hContext,
	IN LPCVOID pvMem)
{
	string_type message;
	message = string_format(_CONV("SCardFreeMemory(hContext:0x%x) called\n"), hContext);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);

	LONG    status = SCARD_S_SUCCESS;

	// TRY TO FIND GIVEN MEMORY REFFERENCE IN LOCAL ALLOCATIONS
	BOOL            bFound = FALSE;
	lptr::iterator  iter;
	for (iter = theApp.m_charAllocatedMemoryList.begin(); iter != theApp.m_charAllocatedMemoryList.end(); iter++) {
		char* ptr = (char*)*iter;
		if (ptr != NULL && (ptr == pvMem)) {
			delete[] ptr;
			bFound = TRUE;

			theApp.m_charAllocatedMemoryList.erase(iter);
			break;
		}
	}
	for (iter = theApp.m_wcharAllocatedMemoryList.begin(); iter != theApp.m_wcharAllocatedMemoryList.end(); iter++) {
		WCHAR* ptr = (WCHAR*)*iter;
		if (ptr != NULL && (ptr == pvMem)) {
			delete[] ptr;
			bFound = TRUE;

			theApp.m_wcharAllocatedMemoryList.erase(iter);
			break;
		}
	}
	// IF NOT FOUND, PASS TO ORIGINAL LIBRARY
	if (!bFound) status = (*Original_SCardFreeMemory)(hContext, pvMem);

	return status;
}

static SCard LONG(STDCALL *Original_SCardDisconnect)(
	SCARDHANDLE hCard,
	DWORD dwDisposition
	);

SCard LONG STDCALL SCardDisconnect(
	SCARDHANDLE hCard,
	DWORD dwDisposition)
{
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardDisconnect called\n"));

	// DISCONNECT FROM CARD
	if (hCard == HANDLE_VIRTUAL_CARD) {
		// DO NOTHING
		return SCARD_S_SUCCESS;
	}
	else {
		return (*Original_SCardDisconnect)(hCard, dwDisposition);
	}
}

static SCard LONG(STDCALL *Original_SCardTransmit)(
	IN SCARDHANDLE hCard,
	IN LPCSCARD_IO_REQUEST pioSendPci,
	IN LPCBYTE pbSendBuffer,
	IN DWORD cbSendLength,
	IN OUT LPSCARD_IO_REQUEST pioRecvPci,
	OUT LPBYTE pbRecvBuffer,
	IN OUT LPDWORD pcbRecvLength
	);

SCard LONG STDCALL SCardTransmit(
	IN SCARDHANDLE hCard,
	IN LPCSCARD_IO_REQUEST pioSendPci,
	IN LPCBYTE pbSendBuffer,
	IN DWORD cbSendLength,
	IN OUT LPSCARD_IO_REQUEST pioRecvPci,
	OUT LPBYTE pbRecvBuffer,
	IN OUT LPDWORD pcbRecvLength
) {

	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardTransmit called\n"));

	LONG result = SCARD_S_SUCCESS;
	//    DWORD written;
	char_type *txMsg = _CONV("transmitted:");
	char_type *rxMsg = _CONV("received:");
	char_type *crlf = _CONV("\r\n");
	const int bufferLength = 1024;
	//char_type buffer[bufferLength];
	string_type buffer;
	char_type  sendBuffer[300];
	clock_t elapsedCard;
	clock_t elapsedLibrary;
	string_type     message;

	elapsedLibrary = -clock();
	if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) {
		//sprintf(buffer, "SCardTransmit (handle 0x%0.8X)#\r\n", hCard);
		buffer = string_format(_CONV("SCardTransmit (handle 0x%0.8X)#\r\n"), hCard);
		CCommonFnc::File_AppendString(WINSCARD_LOG, buffer);

		//sprintf(buffer, "apduCounter:%d#\r\n", apduCounter);
		buffer = string_format(_CONV("apduCounter:%d#\r\n"), apduCounter);
		CCommonFnc::File_AppendString(WINSCARD_LOG, buffer);

		//sprintf(buffer, "totalBytesINCounter:%d#\r\n", theApp.m_processedApduByteCounter + 1);
		buffer = string_format(_CONV("totalBytesINCounter:%d#\r\n"), theApp.m_processedApduByteCounter + 1);
		CCommonFnc::File_AppendString(WINSCARD_LOG, buffer);

		CCommonFnc::File_AppendString(WINSCARD_LOG, txMsg);

		DumpMemory(pbSendBuffer, cbSendLength);
	}


	// SAVE INCOMING APDU
	APDU_BUFFER     apduBuff;
	memset(&apduBuff, 0, sizeof(APDU_BUFFER));
	memcpy(&apduBuff, pbSendBuffer, cbSendLength);
	theApp.apduInList.push_front(apduBuff);

	if (theApp.m_winscardConfig.bMODIFY_APDU_BY_RULES) {
		message = string_format(_CONV("\nIncoming rules applied for apduCounter %d: \n"), apduCounter);
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
		CCommonFnc::BYTE_ConvertFromArrayToHexString((BYTE*)pbSendBuffer, cbSendLength, &message);
		//message.Insert(0, "   "); message += "\n";
		message.insert(0, _CONV("   ")); message += _CONV("\n");
		if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	}

	// COPY INPUT DATA
	memcpy(sendBuffer, pbSendBuffer, cbSendLength);

	// APPLY INCOMING RULES
	if (theApp.m_winscardConfig.bMODIFY_APDU_BY_RULES) {
		if (theApp.m_winscardConfig.bMODIFY_APDU_BY_RULES) theApp.ApplyRules((BYTE*)sendBuffer, &cbSendLength, INPUT_APDU);
		CCommonFnc::BYTE_ConvertFromArrayToHexString((BYTE*)sendBuffer, cbSendLength, &message);
		//message.Insert(0, "   "); message += "\n";
		message.insert(0, _CONV("   ")); message += _CONV("\n");
		if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	}

	elapsedCard = -clock();

	// INCREASE COUNTER OF THE BYTES SEND TO CARD - IS USED AS MEASUREMENT TRIGGER LATER
	theApp.m_processedApduByteCounter += cbSendLength;

	// SCSAT04
	if (theApp.m_scsat04Config.bRedirect) {
		// GET PIN COUNTER HACK
		if (memcmp(pbSendBuffer, PIN_COUNTER_APDU, sizeof(PIN_COUNTER_APDU)) == 0) {
			pbRecvBuffer[0] = 0x03; pbRecvBuffer[1] = 0x90; pbRecvBuffer[2] = 0x00;
			*pcbRecvLength = 3;

			//_sleep(1000);
		}
		else {

			// FORWARD TO SCSAT04 
			result = theApp.SCSAT_SCardTransmit(&(theApp.m_scsat04Config), (SCARD_IO_REQUEST *)pioSendPci, (LPCBYTE)sendBuffer, cbSendLength, pioRecvPci, pbRecvBuffer, pcbRecvLength);

			// APPEND 90 00 TO RETURN BUFFER IN CASE OF DATA_OUT RETRIEVE COMMAND (IF SCSAT IS NOT RETURNING IT)             
			if (memcmp(pbSendBuffer, GET_APDU1, sizeof(GET_APDU1)) == 0 || memcmp(pbSendBuffer, GET_APDU2, sizeof(GET_APDU2)) == 0) {
				if (result == SCARD_S_SUCCESS) {
					if ((pbRecvBuffer[*pcbRecvLength - 2] != 0x90) && (pbRecvBuffer[*pcbRecvLength - 1] != 0x00)) {
						// 0x90 0x00 IS MISSING
						pbRecvBuffer[*pcbRecvLength] = 0x90;
						pbRecvBuffer[*pcbRecvLength + 1] = 0x00;
						*pcbRecvLength += 2;
					}
				}
			}
		}
	}
	else {

		// If required, then ensure that at least one byte will be send to card
		if (theApp.m_winscardConfig.bFORCE_APDU_NONZERO_INPUT_DATA) {
			if (cbSendLength < 6) {
				// ADD ONE ZERO BYTE
				sendBuffer[4] = 1;
				sendBuffer[5] = 0;
				cbSendLength++;
			}
		}

		// SEND DIRECTLY TO LOCAL READER
		result = (*Original_SCardTransmit)(hCard, pioSendPci, (LPCBYTE)sendBuffer, cbSendLength, pioRecvPci, pbRecvBuffer, pcbRecvLength);
	}


	// HACK - if required, then perform transparently data readout on behalf of reader
	// RECEIVE RESPONSE DATA, IF ANY 
	if ((*pcbRecvLength == 2) && theApp.m_winscardConfig.bAUTO_REQUEST_DATA) {
		// READOUT ALL DATA
		DWORD   recvOffset = 0;
		while (((pbRecvBuffer[recvOffset]) == 0x61) || ((pbRecvBuffer[recvOffset]) == 0x6C)) { // 0x61 ... SW_BYTES_REMAINING_00, 0x6C ... SW_CORRECT_LENGTH_00
																							   // GET DATA APDU
			sendBuffer[0] = (BYTE)0x00;
			//sendBuffer[0] = (BYTE) 0xC0;
			//sendBuffer[0] = (BYTE) 0xA0;

			sendBuffer[1] = (BYTE)0xC0;
			sendBuffer[2] = (BYTE)0x00;
			sendBuffer[3] = (BYTE)0x00;

			// HACK TO DEAL WITH CARDS THAT CANNOT HANDLE 254B AND MORE APDUS - if 0x61 0x00 (SW_BYTES_REMAINING_00 with zero remaining bytes is detected, then ask for 254 bytes instead
			if ((pbRecvBuffer[*pcbRecvLength - 1] & 0xff) == 0)  sendBuffer[4] = (BYTE)254;
			else sendBuffer[4] = (BYTE)pbRecvBuffer[*pcbRecvLength - 1];

			cbSendLength = 5;

			int tmp = sendBuffer[4] & 0xff; tmp += 2; *pcbRecvLength = tmp;
			//*pcbRecvLength = sendBuffer[4] & 0xff + 2;
			result = (*Original_SCardTransmit)(hCard, pioSendPci, (LPCBYTE)sendBuffer, cbSendLength, pioRecvPci, pbRecvBuffer + recvOffset, pcbRecvLength);
			recvOffset = *pcbRecvLength - 2;
		}
	}


	// SAVE TIME OF CARD RESPONSE
	elapsedCard += clock();
	if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) {
		//sprintf(buffer, "responseTime:%d#\r\n", elapsedCard);
		buffer = string_format(_CONV("responseTime:%d#\r\n"), elapsedCard);
		CCommonFnc::File_AppendString(WINSCARD_LOG, buffer);

		//sprintf(buffer, "SCardTransmit result:0x%x#\r\n", result);
		buffer = string_format(_CONV("SCardTransmit result:0x%x#\r\n"), result);
		CCommonFnc::File_AppendString(WINSCARD_LOG, buffer);
	}

	if (result != SCARD_S_SUCCESS) {
		// CHANGE LENGTH OF RESPONSE TO PREVENT PARSING OF INCORRECT DATA
		*pcbRecvLength = 0;
	}

	if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) {
		CCommonFnc::File_AppendString(WINSCARD_LOG, rxMsg);
		DumpMemory(pbRecvBuffer, *pcbRecvLength);
		CCommonFnc::File_AppendString(WINSCARD_LOG, crlf);
	}

	// SAVE OUTGOING APDU
	memset(&apduBuff, 0, sizeof(APDU_BUFFER));
	memcpy(&apduBuff, pbRecvBuffer, *pcbRecvLength);
	theApp.apduOutList.push_front(apduBuff);

	if (theApp.m_winscardConfig.bMODIFY_APDU_BY_RULES) {
		message = string_format(_CONV("\nOutgoing rules applied for apduCounter %d: \n"), apduCounter);
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
		CCommonFnc::BYTE_ConvertFromArrayToHexString(pbRecvBuffer, *pcbRecvLength, &message);
		//message.Insert(0, "   "); message += "\n";
		message.insert(0, _CONV("   ")); message += _CONV("\n");
		if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
		// APPLY OUTGOING RULES
		if (theApp.m_winscardConfig.bMODIFY_APDU_BY_RULES) theApp.ApplyRules(pbRecvBuffer, pcbRecvLength, OUTPUT_APDU);
		CCommonFnc::BYTE_ConvertFromArrayToHexString(pbRecvBuffer, *pcbRecvLength, &message);
		//message.Insert(0, "   "); message += "\n";
		message.insert(0, _CONV("   ")); message += _CONV("\n");
		if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	}



	// SCSAT04 - READ MEASUREMENT SAMPLE FROM BOARD IF NOT READED YET AND TRIGGER APPDU WAS REACHED (NUMBER OF BYTES IN)
	if (theApp.m_scsat04Config.bRedirect && !(theApp.m_scsat04Config.sampleReaded) && (theApp.m_processedApduByteCounter >= theApp.m_scsat04Config.measureApduByteCounter)) {
		// DOWNLOAD DATA FROM MEASUREMENT (IF ANY) 
		if (theApp.m_scsat04Config.pSocket != NULL) {
			//string_type message;
			message = string_format(_CONV("get powertrace 0 %d"), theApp.m_scsat04Config.readRatio);
			theApp.m_scsat04Config.pSocket->SendLine(message);
			theApp.m_scsat04Config.baseReadOffset = 0;

			string_type sampleFilePath;
			theApp.SCSAT_CreateAndReceiveSamples(&(theApp.m_scsat04Config), &sampleFilePath);

			// PREVENT FUTHER READING
			theApp.m_scsat04Config.sampleReaded = TRUE;
		}
	}
	// SCSAT04 END

	// increase apdu counter	
	apduCounter++;

	elapsedLibrary += clock();
	if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) {
		message = string_format(_CONV("responseTimeLibrary:%d#\r\n"), elapsedLibrary);
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("...............................................\r\n"));
	}

	return result;
}

// Here are declared only windows specific functions
#if defined (WIN32)

static SCard LONG(STDCALL *Original_SCardStatusA)(
	SCARDHANDLE hCard,
	LPSTR szReaderName,
	LPDWORD pcchReaderLen,
	LPDWORD pdwState,
	LPDWORD pdwProtocol,
	LPBYTE pbAtr,
	LPDWORD pcbAtrLen
	);

SCard LONG STDCALL SCardStatusA(
	SCARDHANDLE hCard,
	LPSTR szReaderName,
	LPDWORD pcchReaderLen,
	LPDWORD pdwState,
	LPDWORD pdwProtocol,
	LPBYTE pbAtr,
	LPDWORD pcbAtrLen)
{
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardStatusA called\n"));
	return (*Original_SCardStatusA)(hCard, szReaderName, pcchReaderLen, pdwState, pdwProtocol, pbAtr, pcbAtrLen);
}

static SCard LONG(STDCALL *Original_SCardConnectW)(
	IN      SCARDCONTEXT hContext,
	IN      LPCWSTR szReader,
	IN      DWORD dwShareMode,
	IN      DWORD dwPreferredProtocols,
	OUT     LPSCARDHANDLE phCard,
	OUT     LPDWORD pdwActiveProtocol
	);

SCard LONG STDCALL SCardConnectW(
	SCARDCONTEXT hContext,
	LPCWSTR szReader,
	DWORD dwShareMode,
	DWORD dwPreferredProtocols,
	LPSCARDHANDLE phCard,
	LPDWORD pdwActiveProtocol)
{
	LONG    status = SCARD_S_SUCCESS;
	string_type message;
	message = string_format(_CONV("SCardConnectW(hContext:0x%x, %S) called\n"), hContext, szReader);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);

	// RESET APDU IN BYTE COUNTER
	theApp.m_processedApduByteCounter = 0;

	// RESET CARD
	if (theApp.m_scsat04Config.bRedirect && (theApp.m_scsat04Config.pSocket != NULL)) {
		string_type message;
		theApp.m_scsat04Config.pSocket->SendLine(_CONV("get reset 1000"));
		string_type l = theApp.m_scsat04Config.pSocket->ReceiveResponse(SCSAT_SOCKET_ENDSEQ, SCSAT_SOCKET_TIMEOUT);
		message = string_format(_CONV("\n:: %s"), l.c_str());
		//message.Replace("\n", " ");
		message.erase(remove(message.begin(), message.end(), '\r'), message.end());
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);

		// PREPARE FOR MEASUREMENT
		message = string_format(_CONV("get params 1 %d %d"), theApp.m_scsat04Config.measureApduByteCounter, theApp.m_scsat04Config.measureApduByteDelay);
		theApp.m_scsat04Config.pSocket->SendLine(message);
		l = theApp.m_scsat04Config.pSocket->ReceiveResponse(SCSAT_SOCKET_ENDSEQ, SCSAT_SOCKET_TIMEOUT);
		message = string_format(_CONV(":: %s"), l.c_str());
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);

		message = string_format(_CONV("post sampling %d"), theApp.m_scsat04Config.numSamples);
		theApp.m_scsat04Config.pSocket->SendLine(message);
		l = theApp.m_scsat04Config.pSocket->ReceiveResponse(SCSAT_SOCKET_ENDSEQ, SCSAT_SOCKET_TIMEOUT);
		message = string_format(_CONV(":: %s"), l.c_str());
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);

		// PREPARE FOR MEASUREMENT READING IN FUTURE
		theApp.m_scsat04Config.sampleReaded = FALSE;

		// CREATE VIRTUAL CARD HANDLE
		*phCard = HANDLE_VIRTUAL_CARD;
		*pdwActiveProtocol = SCARD_PROTOCOL_T0;

		status = SCARD_S_SUCCESS;
	}
	else {
		status = (*Original_SCardConnectW)(hContext, szReader, dwShareMode, dwPreferredProtocols, phCard, pdwActiveProtocol);
	}

	message = string_format(_CONV("-> hCard:0x%x\n"), *phCard);
	CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);

	return status;
}

static SCard LONG(STDCALL *Original_SCardListReadersW)(
	IN      SCARDCONTEXT hContext,
	IN      LPCWSTR mszGroups,
	OUT     LPWSTR mszReaders,
	IN OUT  LPDWORD pcchReaders
	);

SCard LONG STDCALL SCardListReadersW(
	IN      SCARDCONTEXT hContext,
	IN      LPCWSTR mszGroups,
	OUT     LPWSTR mszReaders,
	IN OUT  LPDWORD pcchReaders)
{
	string_type message;
	message = string_format(_CONV("SCardListReadersW(hContext:0x%x) called\n"), hContext);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);

	LONG    status = SCARD_S_SUCCESS;
	lcs     readersList;

	if (*pcchReaders == SCARD_AUTOALLOCATE) {
		// NO BUFFER IS SUPPLIED

		// OBTAIN REQUIRED LENGTH FOR REAL READERS
		if ((status = (*Original_SCardListReadersW)(hContext, mszGroups, NULL, pcchReaders)) == SCARD_S_SUCCESS) {
			// ALLOCATE OWN BUFFER FOR REAL AND VIRTUAL READERS
			int     newLen = *pcchReaders + VIRTUAL_READERS_LEN;
			WCHAR*   readers = new WCHAR[newLen];
			memset(readers, 0, newLen * sizeof(WCHAR));
			*pcchReaders = newLen;
			if ((status = (*Original_SCardListReadersW)(hContext, mszGroups, readers, pcchReaders)) == SCARD_S_SUCCESS) {
				// COPY NAME OF VIRTUAL READERS TO END
				for (DWORD i = 0; i < strlen(VIRT_READER_NAME) + 1; i++) {
					readers[i + *pcchReaders] = VIRT_READER_NAME[i];
				}
				// ADD TRAILING ZERO
				*pcchReaders += (DWORD)strlen(VIRT_READER_NAME) + 1;
				readers[*pcchReaders - 1] = 0;
				// CAST mszReaders TO char** IS NECESSARY TO CORRECTLY PROPAGATE ALLOCATED BUFFER              
				WCHAR**  temp = (WCHAR**)mszReaders;
				*temp = readers;
				CCommonFnc::String_ParseNullSeparatedArray(readers, *pcchReaders, &readersList);
				// ADD ALLOCATED MEMORY TO LIST FOR FUTURE DEALLOCATION
				theApp.m_wcharAllocatedMemoryList.push_back(readers);
			}
		}
	}
	else {
		// BUFFER SUPPLIED
		// OBTAIN REQUIRED LENGTH FOR REAL READERS
		DWORD     realLen = *pcchReaders;
		if ((status = (*Original_SCardListReadersW)(hContext, mszGroups, NULL, &realLen)) == SCARD_S_SUCCESS) {
			if ((realLen + VIRTUAL_READERS_LEN > *pcchReaders) || (mszReaders == NULL)) {
				// SUPPLIED BUFFER IS NOT LARGE ENOUGHT
				*pcchReaders = realLen + VIRTUAL_READERS_LEN;
				if (mszReaders != NULL) status = SCARD_E_INSUFFICIENT_BUFFER;
			}
			else {
				// SUPPLIED BUFFER IS OK, COPY REAL AND VIRTUAL READERS
				realLen = *pcchReaders;
				memset(mszReaders, 0, *pcchReaders * sizeof(WCHAR));
				if ((status = (*Original_SCardListReadersW)(hContext, mszGroups, mszReaders, &realLen)) == SCARD_S_SUCCESS) {
					// COPY NAME OF VIRTUAL READERS TO END (IF USED)
					if (strlen(VIRT_READER_NAME) > 0) {
						for (DWORD i = 0; i < strlen(VIRT_READER_NAME) + 1; i++) {
							mszReaders[i + realLen] = VIRT_READER_NAME[i];
						}
						*pcchReaders = realLen + strlen(VIRT_READER_NAME) + 1;
					}
					else { *pcchReaders = realLen; }
					// ADD TRAILING ZERO
					mszReaders[*pcchReaders - 1] = 0;

					CCommonFnc::String_ParseNullSeparatedArray(mszReaders, *pcchReaders, &readersList);
				}
			}
		}
	}

	if (status == STAT_OK && mszReaders != NULL) {
		if (theApp.m_winscardConfig.sREADER_ORDERED_FIRST != _CONV("")) {
			// REODERING OF READERS WILL BE PERFORMED

			// TRY TO FIND POSITION OF PREFFERED READER IN BUFFER
			for (DWORD i = 0; i < *pcchReaders - theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length(); i++) {
				if (memcmp((LPCTSTR)theApp.m_winscardConfig.sREADER_ORDERED_FIRST.c_str(), mszReaders + i, theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length() * sizeof(WCHAR)) == 0) {
					// PREFFERED READER FOUND

					WCHAR*   readers = new WCHAR[*pcchReaders];
					memset(readers, 0, *pcchReaders * sizeof(WCHAR));
					memcpy(readers, mszReaders, *pcchReaders * sizeof(WCHAR));

					DWORD   offset = 0;
					// PREFFERED FIRST
					memcpy(readers, mszReaders + i, theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length() * sizeof(WCHAR));
					readers[theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length()] = 0;
					offset += theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length() + 1;
					// ORIGINAL PREDECESOR SECOND
					memcpy(readers + offset, mszReaders, i * sizeof(WCHAR));
					offset += i;
					// ORIGINAL SUCCESSOR THIRD - IS THERE FROM INITIAL MEMCPY

					// COPY BACK
					memcpy(mszReaders, readers, *pcchReaders * sizeof(WCHAR));
					delete[] readers;

					break;
				}
			}
		}
	}

	lcs::iterator   iter;
	string_type         availableReaders = _CONV("-> Found readers: ");
	for (iter = readersList.begin(); iter != readersList.end(); iter++) {
		availableReaders += *iter;
		availableReaders += _CONV(", ");
	}
	availableReaders += _CONV("\n");
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, availableReaders);

	return status;
}

static SCard LONG(STDCALL *Original_SCardListReadersA)(
	IN      SCARDCONTEXT hContext,
	IN      LPCSTR mszGroups,
	OUT     LPSTR mszReaders,
	IN OUT  LPDWORD pcchReaders
	);

SCard LONG STDCALL SCardListReadersA(
	IN      SCARDCONTEXT hContext,
	IN      LPCSTR mszGroups,
	OUT     LPSTR mszReaders,
	IN OUT  LPDWORD pcchReaders)
{
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardListReadersA called\n"));

	int  status = SCARD_S_SUCCESS;
	lcs     readersList;

	if (*pcchReaders == SCARD_AUTOALLOCATE) {
		// NO BUFFER IS SUPPLIED

		// OBTAIN REQUIRED LENGTH FOR REAL READERS
		if ((status = (*Original_SCardListReadersA)(hContext, mszGroups, NULL, pcchReaders)) == SCARD_S_SUCCESS) {
			// ALLOCATE OWN BUFFER FOR REAL AND VIRTUAL READERS
			int     newLen = *pcchReaders + VIRTUAL_READERS_LEN + 2;
			char*   readers = new char[newLen];
			memset(readers, 0, newLen);
			*pcchReaders = newLen;
			if ((status = (*Original_SCardListReadersA)(hContext, mszGroups, readers, pcchReaders)) == SCARD_S_SUCCESS) {
				// COPY NAME OF VIRTUAL READERS TO END
				memcpy(readers + *pcchReaders, VIRT_READER_NAME, strlen(VIRT_READER_NAME));
				// ADD TRAILING ZERO
				*pcchReaders += (DWORD)strlen(VIRT_READER_NAME) + 1;
				readers[*pcchReaders - 1] = 0;
				// CAST mszReaders TO char** IS NECESSARY TO CORRECTLY PROPAGATE ALLOCATED BUFFER              
				char**  temp = (char**)mszReaders;
				*temp = readers;
				CCommonFnc::String_ParseNullSeparatedArray((BYTE*)readers, *pcchReaders - 1, &readersList);
				// ADD ALLOCATED MEMORY TO LIST FOR FUTURE DEALLOCATION
				theApp.m_charAllocatedMemoryList.push_back(readers);
			}
		}
	}
	else {
		// BUFFER SUPPLIED
		// OBTAIN REQUIRED LENGTH FOR REAL READERS
		DWORD     realLen = *pcchReaders;
		if ((status = (*Original_SCardListReadersA)(hContext, mszGroups, NULL, &realLen)) == SCARD_S_SUCCESS) {
			if ((realLen + VIRTUAL_READERS_LEN > *pcchReaders) || (mszReaders == NULL)) {
				// SUPPLIED BUFFER IS NOT LARGE ENOUGHT
				*pcchReaders = realLen + VIRTUAL_READERS_LEN;
				if (mszReaders != NULL) status = SCARD_E_INSUFFICIENT_BUFFER;
			}
			else {
				// SUPPLIED BUFFER IS OK, COPY REAL AND VIRTUAL READERS
				realLen = *pcchReaders;
				if ((status = (*Original_SCardListReadersA)(hContext, mszGroups, mszReaders, &realLen)) == SCARD_S_SUCCESS) {
					*pcchReaders = realLen;

					// ADD VIRTUAL READER
					// COPY NAME OF VIRTUAL READERS TO END
					memcpy(mszReaders + realLen, VIRT_READER_NAME, strlen(VIRT_READER_NAME));
					*pcchReaders = realLen + strlen(VIRT_READER_NAME) + 1;
					// ADD TRAILING ZERO
					mszReaders[*pcchReaders - 1] = 0;
					/**/
					CCommonFnc::String_ParseNullSeparatedArray((BYTE*)mszReaders, *pcchReaders - 1, &readersList);
				}
			}
		}
	}

	if (status == STAT_OK && mszReaders != NULL) {
		if (theApp.m_winscardConfig.sREADER_ORDERED_FIRST != _CONV("")) {
			// REODERING OF READERS WILL BE PERFORMED

			// TRY TO FIND POSITION OF PREFFERED READER IN BUFFER
			for (DWORD i = 0; i < *pcchReaders - theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length(); i++) {
				if (memcmp((LPCTSTR)theApp.m_winscardConfig.sREADER_ORDERED_FIRST.c_str(), mszReaders + i, theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length()) == 0) {
					// PREFFERED READER FOUND

					char*   readers = new char[*pcchReaders];
					memset(readers, 0, *pcchReaders);
					memcpy(readers, mszReaders, *pcchReaders);

					DWORD   offset = 0;
					// PREFFERED FIRST
					memcpy(readers, mszReaders + i, theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length());
					readers[theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length()] = 0;
					offset += theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length() + 1;
					// ORIGINAL PREDECESOR SECOND
					memcpy(readers + offset, mszReaders, i);
					offset += i;
					// ORIGINAL SUCCESSOR THIRD - IS THERE FROM INITIAL MEMCPY

					// COPY BACK
					memcpy(mszReaders, readers, *pcchReaders);
					delete[] readers;


					break;
				}
			}
		}
	}

	lcs::iterator   iter;
	string_type         availableReaders = _CONV("-> Found readers: ");
	for (iter = readersList.begin(); iter != readersList.end(); iter++) {
		availableReaders += *iter;
		availableReaders += _CONV(", ");
	}
	availableReaders += _CONV("\n");
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, availableReaders);

	return status;
}

static SCard LONG(STDCALL *Original_SCardListReaderGroupsA)(
	IN      SCARDCONTEXT hContext,
	OUT     LPSTR mszGroups,
	IN OUT  LPDWORD pcchGroups
	);

SCard LONG STDCALL SCardListReaderGroupsA(
	IN      SCARDCONTEXT hContext,
	OUT     LPSTR mszGroups,
	IN OUT  LPDWORD pcchGroups
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardListReaderGroupsA called\n"));
	return (*Original_SCardListReaderGroupsA)(hContext, mszGroups, pcchGroups);
}


static SCard LONG(STDCALL *Original_SCardListReaderGroupsW)(
	IN      SCARDCONTEXT hContext,
	OUT     LPWSTR mszGroups,
	IN OUT  LPDWORD pcchGroups
	);

SCard LONG STDCALL SCardListReaderGroupsW(
	IN      SCARDCONTEXT hContext,
	OUT     LPWSTR mszGroups,
	IN OUT  LPDWORD pcchGroups
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardListReaderGroupsW called\n"));
	return (*Original_SCardListReaderGroupsW)(hContext, mszGroups, pcchGroups);
}


static SCard LONG(STDCALL *Original_SCardListCardsA)(
	IN      SCARDCONTEXT hContext,
	IN      LPCBYTE pbAtr,
	IN      LPCGUID rgquidInterfaces,
	IN      DWORD cguidInterfaceCount,
	OUT     LPSTR mszCards,
	IN OUT  LPDWORD pcchCards
	);

SCard LONG STDCALL SCardListCardsA(
	IN      SCARDCONTEXT hContext,
	IN      LPCBYTE pbAtr,
	IN      LPCGUID rgquidInterfaces,
	IN      DWORD cguidInterfaceCount,
	OUT     LPSTR mszCards,
	IN OUT  LPDWORD pcchCards
) {
	string_type message;
	message = string_format(_CONV("SCardListCardsA(hContext:0x%x) called\n"), hContext);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	return (*Original_SCardListCardsA)(hContext, pbAtr, rgquidInterfaces, cguidInterfaceCount, mszCards, pcchCards);
}


static SCard LONG(STDCALL *Original_SCardListCardsW)(
	IN      SCARDCONTEXT hContext,
	IN      LPCBYTE pbAtr,
	IN      LPCGUID rgquidInterfaces,
	IN      DWORD cguidInterfaceCount,
	OUT     LPWSTR mszCards,
	IN OUT  LPDWORD pcchCards
	);

SCard LONG STDCALL SCardListCardsW(
	IN      SCARDCONTEXT hContext,
	IN      LPCBYTE pbAtr,
	IN      LPCGUID rgquidInterfaces,
	IN      DWORD cguidInterfaceCount,
	OUT     LPWSTR mszCards,
	IN OUT  LPDWORD pcchCards
) {
	string_type message;
	message = string_format(_CONV("SCardListCardsW(hContext:0x%x) called\n"), hContext);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	return (*Original_SCardListCardsW)(hContext, pbAtr, rgquidInterfaces, cguidInterfaceCount, mszCards, pcchCards);
}


static SCard LONG(STDCALL *Original_SCardListInterfacesA)(
	IN      SCARDCONTEXT hContext,
	IN      LPCSTR szCard,
	OUT     LPGUID pguidInterfaces,
	IN OUT  LPDWORD pcguidInterfaces
	);

SCard LONG STDCALL SCardListInterfacesA(
	IN      SCARDCONTEXT hContext,
	IN      LPCSTR szCard,
	OUT     LPGUID pguidInterfaces,
	IN OUT  LPDWORD pcguidInterfaces
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardListInterfacesA called\n"));
	return (*Original_SCardListInterfacesA)(hContext, szCard, pguidInterfaces, pcguidInterfaces);
}


static SCard LONG(STDCALL *Original_SCardListInterfacesW)(
	IN      SCARDCONTEXT hContext,
	IN      LPCWSTR szCard,
	OUT     LPGUID pguidInterfaces,
	IN OUT  LPDWORD pcguidInterfaces
	);

SCard LONG STDCALL SCardListInterfacesW(
	IN      SCARDCONTEXT hContext,
	IN      LPCWSTR szCard,
	OUT     LPGUID pguidInterfaces,
	IN OUT  LPDWORD pcguidInterfaces
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardListInterfacesW called\n"));
	return (*Original_SCardListInterfacesW)(hContext, szCard, pguidInterfaces, pcguidInterfaces);
}


static SCard LONG(STDCALL *Original_SCardGetProviderIdA)(
	IN      SCARDCONTEXT hContext,
	IN      LPCSTR szCard,
	OUT     LPGUID pguidProviderId
	);

SCard LONG STDCALL SCardGetProviderIdA(
	IN      SCARDCONTEXT hContext,
	IN      LPCSTR szCard,
	OUT     LPGUID pguidProviderId
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardGetProviderIdA called\n"));
	return (*Original_SCardGetProviderIdA)(hContext, szCard, pguidProviderId);
}


static SCard LONG(STDCALL *Original_SCardGetProviderIdW)(
	IN      SCARDCONTEXT hContext,
	IN      LPCWSTR szCard,
	OUT     LPGUID pguidProviderId
	);

SCard LONG STDCALL SCardGetProviderIdW(
	IN      SCARDCONTEXT hContext,
	IN      LPCWSTR szCard,
	OUT     LPGUID pguidProviderId
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardGetProviderIdW called\n"));
	return (*Original_SCardGetProviderIdW)(hContext, szCard, pguidProviderId);
}


static SCard LONG(STDCALL *Original_SCardGetCardTypeProviderNameA)(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szCardName,
	IN DWORD dwProviderId,
	OUT LPSTR szProvider,
	IN OUT LPDWORD pcchProvider
	);

SCard LONG STDCALL SCardGetCardTypeProviderNameA(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szCardName,
	IN DWORD dwProviderId,
	OUT LPSTR szProvider,
	IN OUT LPDWORD pcchProvider
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardGetCardTypeProviderNameA called\n"));
	return (*Original_SCardGetCardTypeProviderNameA)(hContext, szCardName, dwProviderId, szProvider, pcchProvider);
}


static SCard LONG(STDCALL *Original_SCardGetCardTypeProviderNameW)(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szCardName,
	IN DWORD dwProviderId,
	OUT LPWSTR szProvider,
	IN OUT LPDWORD pcchProvider
	);

SCard LONG STDCALL SCardGetCardTypeProviderNameW(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szCardName,
	IN DWORD dwProviderId,
	OUT LPWSTR szProvider,
	IN OUT LPDWORD pcchProvider
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardGetCardTypeProviderNameW called\n"));
	return (*Original_SCardGetCardTypeProviderNameW)(hContext, szCardName, dwProviderId, szProvider, pcchProvider);
}


static SCard LONG(STDCALL *Original_SCardIntroduceReaderGroupA)(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szGroupName
	);

SCard LONG STDCALL SCardIntroduceReaderGroupA(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szGroupName
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardIntroduceReaderGroupA called\n"));
	return (*Original_SCardIntroduceReaderGroupA)(hContext, szGroupName);
}


static SCard LONG(STDCALL *Original_SCardIntroduceReaderGroupW)(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szGroupName
	);

SCard LONG STDCALL SCardIntroduceReaderGroupW(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szGroupName
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardIntroduceReaderGroupW called\n"));
	return (*Original_SCardIntroduceReaderGroupW)(hContext, szGroupName);
}


static SCard LONG(STDCALL *Original_SCardForgetReaderGroupA)(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szGroupName
	);

SCard LONG STDCALL SCardForgetReaderGroupA(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szGroupName
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardForgetReaderGroupA called\n"));
	return (*Original_SCardForgetReaderGroupA)(hContext, szGroupName);
}


static SCard LONG(STDCALL *Original_SCardForgetReaderGroupW)(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szGroupName
	);

SCard LONG STDCALL SCardForgetReaderGroupW(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szGroupName
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardForgetReaderGroupW called\n"));
	return (*Original_SCardForgetReaderGroupW)(hContext, szGroupName);
}


static SCard LONG(STDCALL *Original_SCardIntroduceReaderA)(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szReaderName,
	IN LPCSTR szDeviceName
	);

SCard LONG STDCALL SCardIntroduceReaderA(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szReaderName,
	IN LPCSTR szDeviceName
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardIntroduceReaderA called\n"));
	return (*Original_SCardIntroduceReaderA)(hContext, szReaderName, szDeviceName);
}


static SCard LONG(STDCALL *Original_SCardIntroduceReaderW)(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szReaderName,
	IN LPCWSTR szDeviceName
	);

SCard LONG STDCALL SCardIntroduceReaderW(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szReaderName,
	IN LPCWSTR szDeviceName
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardIntroduceReaderW called\n"));
	return (*Original_SCardIntroduceReaderW)(hContext, szReaderName, szDeviceName);
}


static SCard LONG(STDCALL *Original_SCardForgetReaderA)(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szReaderName
	);

SCard LONG STDCALL SCardForgetReaderA(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szReaderName
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardForgetReaderA called\n"));
	return (*Original_SCardForgetReaderA)(hContext, szReaderName);
}


static SCard LONG(STDCALL *Original_SCardForgetReaderW)(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szReaderName
	);

SCard LONG STDCALL SCardForgetReaderW(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szReaderName
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardForgetReaderW called\n"));
	return (*Original_SCardForgetReaderW)(hContext, szReaderName);
}


static SCard LONG(STDCALL *Original_SCardAddReaderToGroupA)(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szReaderName,
	IN LPCSTR szGroupName
	);

SCard LONG STDCALL SCardAddReaderToGroupA(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szReaderName,
	IN LPCSTR szGroupName
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardAddReaderToGroupA called\n"));
	return (*Original_SCardAddReaderToGroupA)(hContext, szReaderName, szGroupName);
}


static SCard LONG(STDCALL *Original_SCardAddReaderToGroupW)(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szReaderName,
	IN LPCWSTR szGroupName
	);

SCard LONG STDCALL SCardAddReaderToGroupW(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szReaderName,
	IN LPCWSTR szGroupName
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardAddReaderToGroupW called\n"));
	return (*Original_SCardAddReaderToGroupW)(hContext, szReaderName, szGroupName);
}


static SCard LONG(STDCALL *Original_SCardRemoveReaderFromGroupA)(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szReaderName,
	IN LPCSTR szGroupName
	);

SCard LONG STDCALL SCardRemoveReaderFromGroupA(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szReaderName,
	IN LPCSTR szGroupName
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardRemoveReaderFromGroupA called\n"));
	return (*Original_SCardRemoveReaderFromGroupA)(hContext, szReaderName, szGroupName);
}


static SCard LONG(STDCALL *Original_SCardRemoveReaderFromGroupW)(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szReaderName,
	IN LPCWSTR szGroupName
	);

SCard LONG STDCALL SCardRemoveReaderFromGroupW(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szReaderName,
	IN LPCWSTR szGroupName
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardRemoveReaderFromGroupW called\n"));
	return (*Original_SCardRemoveReaderFromGroupW)(hContext, szReaderName, szGroupName);
}


static SCard LONG(STDCALL *Original_SCardIntroduceCardTypeA)(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szCardName,
	IN LPCGUID pguidPrimaryProvider,
	IN LPCGUID rgguidInterfaces,
	IN DWORD dwInterfaceCount,
	IN LPCBYTE pbAtr,
	IN LPCBYTE pbAtrMask,
	IN DWORD cbAtrLen
	);

SCard LONG STDCALL SCardIntroduceCardTypeA(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szCardName,
	IN LPCGUID pguidPrimaryProvider,
	IN LPCGUID rgguidInterfaces,
	IN DWORD dwInterfaceCount,
	IN LPCBYTE pbAtr,
	IN LPCBYTE pbAtrMask,
	IN DWORD cbAtrLen
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardIntroduceCardTypeA called\n"));
	return (*Original_SCardIntroduceCardTypeA)(hContext, szCardName, pguidPrimaryProvider, rgguidInterfaces, dwInterfaceCount, pbAtr, pbAtrMask, cbAtrLen);
}


static SCard LONG(STDCALL *Original_SCardIntroduceCardTypeW)(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szCardName,
	IN LPCGUID pguidPrimaryProvider,
	IN LPCGUID rgguidInterfaces,
	IN DWORD dwInterfaceCount,
	IN LPCBYTE pbAtr,
	IN LPCBYTE pbAtrMask,
	IN DWORD cbAtrLen
	);

SCard LONG STDCALL SCardIntroduceCardTypeW(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szCardName,
	IN LPCGUID pguidPrimaryProvider,
	IN LPCGUID rgguidInterfaces,
	IN DWORD dwInterfaceCount,
	IN LPCBYTE pbAtr,
	IN LPCBYTE pbAtrMask,
	IN DWORD cbAtrLen
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardIntroduceCardTypeW called\n"));
	return (*Original_SCardIntroduceCardTypeW)(hContext, szCardName, pguidPrimaryProvider, rgguidInterfaces, dwInterfaceCount, pbAtr, pbAtrMask, cbAtrLen);
}


static SCard LONG(STDCALL *Original_SCardSetCardTypeProviderNameA)(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szCardName,
	IN DWORD dwProviderId,
	IN LPCSTR szProvider
	);

SCard LONG STDCALL SCardSetCardTypeProviderNameA(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szCardName,
	IN DWORD dwProviderId,
	IN LPCSTR szProvider
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardSetCardTypeProviderNameA called\n"));
	return (*Original_SCardSetCardTypeProviderNameA)(hContext, szCardName, dwProviderId, szProvider);
}


static SCard LONG(STDCALL *Original_SCardSetCardTypeProviderNameW)(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szCardName,
	IN DWORD dwProviderId,
	IN LPCWSTR szProvider
	);

SCard LONG STDCALL SCardSetCardTypeProviderNameW(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szCardName,
	IN DWORD dwProviderId,
	IN LPCWSTR szProvider
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardSetCardTypeProviderNameW called\n"));
	return (*Original_SCardSetCardTypeProviderNameW)(hContext, szCardName, dwProviderId, szProvider);
}


static SCard LONG(STDCALL *Original_SCardForgetCardTypeA)(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szCardName
	);

SCard LONG STDCALL SCardForgetCardTypeA(
	IN SCARDCONTEXT hContext,
	IN LPCSTR szCardName
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardForgetCardTypeA called\n"));
	return (*Original_SCardForgetCardTypeA)(hContext, szCardName);
}


static SCard LONG(STDCALL *Original_SCardForgetCardTypeW)(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szCardName
	);

SCard LONG STDCALL SCardForgetCardTypeW(
	IN SCARDCONTEXT hContext,
	IN LPCWSTR szCardName
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardForgetCardTypeW called\n"));
	return (*Original_SCardForgetCardTypeW)(hContext, szCardName);
}

static SCard LONG(STDCALL *Original_SCardLocateCardsA)(
	IN      SCARDCONTEXT hContext,
	IN      LPCSTR mszCards,
	IN OUT  LPSCARD_READERSTATEA rgReaderStates,
	IN      DWORD cReaders
	);

SCard LONG STDCALL SCardLocateCardsA(
	IN      SCARDCONTEXT hContext,
	IN      LPCSTR mszCards,
	IN OUT  LPSCARD_READERSTATEA rgReaderStates,
	IN      DWORD cReaders
) {
	string_type message;
	message = string_format(_CONV("SCardLocateCardsA(%s,0x%x) called\n"), mszCards, hContext);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	return (*Original_SCardLocateCardsA)(hContext, mszCards, rgReaderStates, cReaders);
}


static SCard LONG(STDCALL *Original_SCardLocateCardsW)(
	IN      SCARDCONTEXT hContext,
	IN      LPCWSTR mszCards,
	IN OUT  LPSCARD_READERSTATEW rgReaderStates,
	IN      DWORD cReaders
	);

SCard LONG STDCALL SCardLocateCardsW(
	IN      SCARDCONTEXT hContext,
	IN      LPCWSTR mszCards,
	IN OUT  LPSCARD_READERSTATEW rgReaderStates,
	IN      DWORD cReaders
) {
	string_type message;
	message = string_format(_CONV("SCardLocateCardsW(%S,0x%x) called\n"), mszCards, hContext);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	return (*Original_SCardLocateCardsW)(hContext, mszCards, rgReaderStates, cReaders);
}


static SCard LONG(STDCALL *Original_SCardLocateCardsByATRA)(
	IN      SCARDCONTEXT hContext,
	IN      LPSCARD_ATRMASK rgAtrMasks,
	IN      DWORD cAtrs,
	IN OUT  LPSCARD_READERSTATEA rgReaderStates,
	IN      DWORD cReaders
	);

SCard LONG STDCALL SCardLocateCardsByATRA(
	IN      SCARDCONTEXT hContext,
	IN      LPSCARD_ATRMASK rgAtrMasks,
	IN      DWORD cAtrs,
	IN OUT  LPSCARD_READERSTATEA rgReaderStates,
	IN      DWORD cReaders
) {
	string_type message;
	message = string_format(_CONV("SCardLocateCardsByATRA(hContext:0x%x) called\n"), hContext);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	return (*Original_SCardLocateCardsByATRA)(hContext, rgAtrMasks, cAtrs, rgReaderStates, cReaders);
}


static SCard LONG(STDCALL *Original_SCardLocateCardsByATRW)(
	IN      SCARDCONTEXT hContext,
	IN      LPSCARD_ATRMASK rgAtrMasks,
	IN      DWORD cAtrs,
	IN OUT  LPSCARD_READERSTATEW rgReaderStates,
	IN      DWORD cReaders
	);

SCard LONG STDCALL SCardLocateCardsByATRW(
	IN      SCARDCONTEXT hContext,
	IN      LPSCARD_ATRMASK rgAtrMasks,
	IN      DWORD cAtrs,
	IN OUT  LPSCARD_READERSTATEW rgReaderStates,
	IN      DWORD cReaders
) {
	string_type message;
	message = string_format(_CONV("SCardLocateCardsByATRW(hContext:0x%x) called\n"), hContext);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	return (*Original_SCardLocateCardsByATRW)(hContext, rgAtrMasks, cAtrs, rgReaderStates, cReaders);
}


static SCard LONG(STDCALL *Original_SCardGetStatusChangeA)(
	IN      SCARDCONTEXT hContext,
	IN      DWORD dwTimeout,
	IN OUT  LPSCARD_READERSTATEA rgReaderStates,
	IN      DWORD cReaders
	);

SCard LONG STDCALL SCardGetStatusChangeA(
	IN      SCARDCONTEXT hContext,
	IN      DWORD dwTimeout,
	IN OUT  LPSCARD_READERSTATEA rgReaderStates,
	IN      DWORD cReaders
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardGetStatusChangeA called\n"));
	return (*Original_SCardGetStatusChangeA)(hContext, dwTimeout, rgReaderStates, cReaders);
}


static SCard LONG(STDCALL *Original_SCardGetStatusChangeW)(
	IN      SCARDCONTEXT hContext,
	IN      DWORD dwTimeout,
	IN OUT  LPSCARD_READERSTATEW rgReaderStates,
	IN      DWORD cReaders
	);

SCard LONG STDCALL SCardGetStatusChangeW(
	IN      SCARDCONTEXT hContext,
	IN      DWORD dwTimeout,
	IN OUT  LPSCARD_READERSTATEW rgReaderStates,
	IN      DWORD cReaders
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardGetStatusChangeW called\n"));
	return (*Original_SCardGetStatusChangeW)(hContext, dwTimeout, rgReaderStates, cReaders);
}

static SCard LONG(STDCALL *Original_SCardConnectA)(
	IN      SCARDCONTEXT hContext,
	IN      LPCSTR szReader,
	IN      DWORD dwShareMode,
	IN      DWORD dwPreferredProtocols,
	OUT     LPSCARDHANDLE phCard,
	OUT     LPDWORD pdwActiveProtocol
	);

SCard LONG STDCALL SCardConnectA(
	IN      SCARDCONTEXT hContext,
	IN      LPCSTR szReader,
	IN      DWORD dwShareMode,
	IN      DWORD dwPreferredProtocols,
	OUT     LPSCARDHANDLE phCard,
	OUT     LPDWORD pdwActiveProtocol
) {

	if (theApp.m_winscardConfig.bFORCE_CONNECT_SHARED_MODE) {
		// we will always set mode to shared, if required
		dwShareMode = SCARD_SHARE_SHARED;
	}


	LONG status = (*Original_SCardConnectA)(hContext, szReader, dwShareMode, dwPreferredProtocols, phCard, pdwActiveProtocol);
	string_type message;
	message = string_format(_CONV("SCardConnectA(hContext:0x%x,%s,hCard:0x%x) called\n"), hContext, szReader, *phCard);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	return status;
}

static SCard LONG(STDCALL *Original_SCardUIDlgSelectCardA)(
	LPOPENCARDNAMEA_EX
	);

SCard LONG STDCALL SCardUIDlgSelectCardA(
	LPOPENCARDNAMEA_EX a
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardUIDlgSelectCardA called\n"));
	return (*Original_SCardUIDlgSelectCardA)(a);
}


static SCard LONG(STDCALL *Original_SCardUIDlgSelectCardW)(
	LPOPENCARDNAMEW_EX
	);

SCard LONG STDCALL SCardUIDlgSelectCardW(
	LPOPENCARDNAMEW_EX a
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardUIDlgSelectCardW called\n"));
	return (*Original_SCardUIDlgSelectCardW)(a);
}

static SCard LONG(STDCALL *Original_GetOpenCardNameA)(
	LPOPENCARDNAMEA
	);

SCard LONG STDCALL GetOpenCardNameA(
	LPOPENCARDNAMEA a
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("GetOpenCardNameA called\n"));
	return (*Original_GetOpenCardNameA)(a);
}

static SCard LONG(STDCALL *Original_GetOpenCardNameW)(
	LPOPENCARDNAMEW
	);

SCard LONG STDCALL GetOpenCardNameW(
	LPOPENCARDNAMEW a
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("GetOpenCardNameW called\n"));
	return (*Original_GetOpenCardNameW)(a);
}


static SCard LONG(STDCALL *Original_SCardDlgExtendedError)(void);

SCard LONG STDCALL SCardDlgExtendedError(void) {
	CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardDlgExtendedError called\n"));
	return (*Original_SCardDlgExtendedError)();
}

static SCard LONG(STDCALL *Original_SCardCancelTransaction)(
	IN      SCARDHANDLE hCard
	);

SCard LONG STDCALL SCardCancelTransaction(
	IN      SCARDHANDLE hCard
) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardCancelTransaction called\n"));
	return (*Original_SCardCancelTransaction)(hCard);
}

static SCard LONG(STDCALL *Original_SCardState)(
	IN SCARDHANDLE hCard,
	OUT LPDWORD pdwState,
	OUT LPDWORD pdwProtocol,
	OUT LPBYTE pbAtr,
	IN OUT LPDWORD pcbAtrLen
	);

SCard LONG STDCALL SCardState(
	IN SCARDHANDLE hCard,
	OUT LPDWORD pdwState,
	OUT LPDWORD pdwProtocol,
	OUT LPBYTE pbAtr,
	IN OUT LPDWORD pcbAtrLen
) {
	string_type message;
	message = string_format(_CONV("SCardState(hCard:0x%x) called\n"), hCard);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	return (*Original_SCardState)(hCard, pdwState, pdwProtocol, pbAtr, pcbAtrLen);
}


static SCard LONG(STDCALL *Original_SCardStatusW)(
	IN SCARDHANDLE hCard,
	OUT LPWSTR szReaderName,
	IN OUT LPDWORD pcchReaderLen,
	OUT LPDWORD pdwState,
	OUT LPDWORD pdwProtocol,
	OUT LPBYTE pbAtr,
	IN OUT LPDWORD pcbAtrLen
	);

SCard LONG STDCALL SCardStatusW(
	IN SCARDHANDLE hCard,
	OUT LPWSTR szReaderName,
	IN OUT LPDWORD pcchReaderLen,
	OUT LPDWORD pdwState,
	OUT LPDWORD pdwProtocol,
	OUT LPBYTE pbAtr,
	IN OUT LPDWORD pcbAtrLen
) {
	string_type message;
	message = string_format(_CONV("SCardStatusW(hCard:0x%x) called\n"), hCard);
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
	return (*Original_SCardStatusW)(hCard, szReaderName, pcchReaderLen, pdwState, pdwProtocol, pbAtr, pcbAtrLen);
}

static SCard HANDLE(STDCALL *Original_SCardAccessStartedEvent)(void);

SCard HANDLE STDCALL SCardAccessStartedEvent(void) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardAccessStartedEvent called\n"));
	return (*Original_SCardAccessStartedEvent)();
}


static SCard void(STDCALL *Original_SCardReleaseStartedEvent)(void);

SCard void STDCALL SCardReleaseStartedEvent(void) {
	if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("SCardReleaseStartedEvent called\n"));
	return (*Original_SCardReleaseStartedEvent)();
}
#endif

#if __linux__
	typedef void (*q)(void *restrict, const char *restrict);
	static q load_func = dlsym;
#else 
	typedef FARPROC(STDCALL *q) (HMODULE, LPCSTR);
	static q load_func = GetProcAddress;
#endif

/*na linux chyba: 
 *SCardConnect
 *SCardStatus4
 *SCardGetStatusChange
 *SCardListReaders
 *SCardListReaderGroups
*/
int initialize()
{
	void *handle;
	char *error = "";
	#ifdef __linux__ 
	    hOriginal = dlopen("libpcsclite.so", RTLD_LAZY);
    #else 
	    hOriginal = LoadLibrary(_CONV("original.dll"));
    #endif 
	
	if (!hOriginal) {
        #if __linux__
		    error = dlerror()
        #endif
		fprintf(stderr, "Failed to load original library %s\n", error);
		return FALSE;
	}
	
    #if __linux__
	    dlerror();    // Clear any existing errorz
    #endif

	Original_SCardTransmit =
		(long(STDCALL *)(unsigned long, const struct _SCARD_IO_REQUEST *, const unsigned char *, unsigned long, struct _SCARD_IO_REQUEST *, unsigned char *, unsigned long *))
		load_func(hOriginal, "SCardTransmit");
	if ((!Original_SCardTransmit)) {
        #if __linux__
		error = dlerror();
        #endif
		fprintf(stderr, "Could not find SCardTransmit procedure address %s\n", error);
		return FALSE;
	}
	
	Original_SCardDisconnect =
		(long(STDCALL *)(SCARDHANDLE hCard, DWORD dwDisposition))
		load_func(hOriginal, "SCardDisconnect");
	if ((!Original_SCardDisconnect)) {
		#if __linux__
		error = dlerror();
		#endif
		fprintf(stderr, "Could not find SCardDisconnect procedure address %s\n", error);
		return FALSE;
	}

	Original_SCardFreeMemory =
		(long(STDCALL *)(SCARDCONTEXT hContext, LPCVOID pvMem))
		load_func(hOriginal, "SCardFreeMemory");
	if ((!Original_SCardFreeMemory)) {
		#if __linux__
		error = dlerror();
		#endif
		fprintf(stderr, "Could not find SCardFreeMemory procedure address %s\n", error);
		return FALSE;
	}

	Original_SCardEstablishContext = (LONG(STDCALL *)(
		IN  DWORD dwScope,
		IN  LPCVOID pvReserved1,
		IN  LPCVOID pvReserved2,
		OUT LPSCARDCONTEXT phContext)) load_func(hOriginal, "SCardEstablishContext");
	if (!Original_SCardEstablishContext) {
        #if __linux__
		error = dlerror();
        #endif
		fprintf(stderr, "Could not find SCardEstablishContext procedure address %s\n", error);
		return FALSE;
	}

	Original_SCardReleaseContext =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext))
		load_func(hOriginal, "SCardReleaseContext");
	if (!Original_SCardReleaseContext) {
		#if __linux__
		error = dlerror();
		#endif
		fprintf(stderr, "Could not find SCardReleaseContext procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardIsValidContext =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext))
		load_func(hOriginal, "SCardIsValidContext");
	if (!Original_SCardIsValidContext) {
		#if __linux__
		error = dlerror();
		#endif
		fprintf(stderr, "Could not find SCardIsValidContext procedure address:  %s\n", error);
		return FALSE;
	}
	
	Original_SCardFreeMemory =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCVOID pvMem))
		load_func(hOriginal, "SCardFreeMemory");
	if (!Original_SCardFreeMemory) {
		#if __linux__
		error = dlerror();
		#endif
		fprintf(stderr, "Could not find SCardFreeMemory procedure address:  %s\n", error);
		return FALSE;
	}
	
	Original_SCardCancel =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext))
		load_func(hOriginal, "SCardCancel");
	if (!Original_SCardCancel) {
		#if __linux__
		error = dlerror();
		#endif
		fprintf(stderr, "Could not find SCardCancel procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardReconnect =
		(LONG(STDCALL *)(
			IN      SCARDHANDLE hCard,
			IN      DWORD dwShareMode,
			IN      DWORD dwPreferredProtocols,
			IN      DWORD dwInitialization,
			OUT     LPDWORD pdwActiveProtocol))
		load_func(hOriginal, "SCardReconnect");
	if (!Original_SCardReconnect) {
		#if __linux__
		error = dlerror();
		#endif
		fprintf(stderr, "Could not find SCardReconnect procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardDisconnect =
		(LONG(STDCALL *)(
			IN      SCARDHANDLE hCard,
			IN      DWORD dwDisposition))
		load_func(hOriginal, "SCardDisconnect");
	if (!Original_SCardDisconnect) {
		#if __linux__
		error = dlerror();
		#endif
		fprintf(stderr, "Could not find SCardDisconnect procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardBeginTransaction =
		(LONG(STDCALL *)(
			IN      SCARDHANDLE hCard))
		load_func(hOriginal, "SCardBeginTransaction");
	if (!Original_SCardBeginTransaction) {
		#if __linux__
		error = dlerror();
		#endif
		fprintf(stderr, "Could not find SCardBeginTransaction procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardEndTransaction =
		(LONG(STDCALL *)(
			IN      SCARDHANDLE hCard,
			IN      DWORD dwDisposition))
		load_func(hOriginal, "SCardEndTransaction");
	if (!Original_SCardEndTransaction) {
		#if __linux__
		error = dlerror();
		#endif
		fprintf(stderr, "Could not find SCardEndTransaction procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardTransmit =
		(LONG(STDCALL *)(
			IN SCARDHANDLE hCard,
			IN LPCSCARD_IO_REQUEST pioSendPci,
			IN LPCBYTE pbSendBuffer,
			IN DWORD cbSendLength,
			IN OUT LPSCARD_IO_REQUEST pioRecvPci,
			OUT LPBYTE pbRecvBuffer,
			IN OUT LPDWORD pcbRecvLength))
		load_func(hOriginal, "SCardTransmit");
	if (!Original_SCardTransmit) {
		#if __linux__
		error = dlerror();
		#endif
		fprintf(stderr, "Could not find SCardTransmit procedure address:  %s\n", error);
		return FALSE;
	}

	/*LINUX*/
	Original_SCardControl =
		(LONG(STDCALL *)(
			IN      SCARDHANDLE hCard,
			IN      DWORD dwControlCode,
			IN      LPCVOID lpInBuffer,
			IN      DWORD nInBufferSize,
			OUT     LPVOID lpOutBuffer,
			IN      DWORD nOutBufferSize,
			OUT     LPDWORD lpBytesReturned))
		load_func(hOriginal, "SCardControl");
	if (!Original_SCardControl) {
		#if __linux__
				error = dlerror()
		#endif
		fprintf(stderr, "Could not find SCardControl procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetAttrib =
		(LONG(STDCALL *)(
			IN SCARDHANDLE hCard,
			IN DWORD dwAttrId,
			OUT LPBYTE pbAttr,
			IN OUT LPDWORD pcbAttrLen))
		load_func(hOriginal, "SCardGetAttrib");
	if (!Original_SCardGetAttrib) {
		#if __linux__
		error = dlerror();
		#endif
		fprintf(stderr, "Could not find SCardGetAttrib procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardSetAttrib =
		(LONG(STDCALL *)(
			IN SCARDHANDLE hCard,
			IN DWORD dwAttrId,
			IN LPCBYTE pbAttr,
			IN DWORD cbAttrLen))
		load_func(hOriginal, "SCardSetAttrib");
	if (!Original_SCardSetAttrib) {
		#if __linux__
		error = dlerror();
		#endif
		fprintf(stderr, "Could not find SCardSetAttrib procedure address:  %s\n", error);
		return FALSE;
	}

	//Initialization of windows specific funcions
	#if defined (WIN32)
	Original_SCardStatusA =
		(long(STDCALL *)(SCARDHANDLE hCard, LPSTR szReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen))
		load_func(hOriginal, "SCardStatusA");
	if ((!Original_SCardStatusA)) {
		fprintf(stderr, "Could not find SCardStatusA procedure address %s\n");
		return FALSE;
	}

	Original_SCardConnectW =
		(long(STDCALL *)(SCARDCONTEXT hContext, LPCWSTR szReader, DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol))
		load_func(hOriginal, "SCardConnectW");
	if ((!Original_SCardConnectW)) {
		fprintf(stderr, "Could not find SCardConnectW procedure address %s\n");
		return FALSE;
	}

	Original_SCardListReadersW =
		(long(STDCALL *)(SCARDCONTEXT hContext, LPCWSTR mszGroups, LPWSTR mszReaders, LPDWORD pcchReaders))
		load_func(hOriginal, "SCardListReadersW");
	if ((!Original_SCardListReadersW)) {
		fprintf(stderr, "Could not find SCardListReadersW procedure address %s\n", error);
		return FALSE;
	}

	Original_SCardListReadersA =
		(long(STDCALL *)(SCARDCONTEXT hContext, LPCSTR mszGroups, LPSTR mszReaders, LPDWORD pcchReaders))
		load_func(hOriginal, "SCardListReadersA");
	if ((!Original_SCardListReadersA)) {
		fprintf(stderr, "Could not find SCardListReadersA procedure address %s\n", error);
		return FALSE;
	}

	Original_SCardListReaderGroupsA =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			OUT     LPSTR mszGroups,
			IN OUT  LPDWORD pcchGroups))
		load_func(hOriginal, "SCardListReaderGroupsA");
	if (!Original_SCardListReaderGroupsA) {
		fprintf(stderr, "Could not find SCardListReaderGroupsA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListReaderGroupsW =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			OUT     LPWSTR mszGroups,
			IN OUT  LPDWORD pcchGroups))
		load_func(hOriginal, "SCardListReaderGroupsW");
	if (!Original_SCardListReaderGroupsW) {
		fprintf(stderr, "Could not find SCardListReaderGroupsW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListReadersA =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCSTR mszGroups,
			OUT     LPSTR mszReaders,
			IN OUT  LPDWORD pcchReaders))
		load_func(hOriginal, "SCardListReadersA");
	if (!Original_SCardListReadersA) {
		fprintf(stderr, "Could not find SCardListReadersA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListReadersW =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCWSTR mszGroups,
			OUT     LPWSTR mszReaders,
			IN OUT  LPDWORD pcchReaders))
		load_func(hOriginal, "SCardListReadersW");
	if (!Original_SCardListReadersW) {
		fprintf(stderr, "Could not find SCardListReadersW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListCardsA =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCBYTE pbAtr,
			IN      LPCGUID rgquidInterfaces,
			IN      DWORD cguidInterfaceCount,
			OUT     LPSTR mszCards,
			IN OUT  LPDWORD pcchCards))
		load_func(hOriginal, "SCardListCardsA");
	if (!Original_SCardListCardsA) {
		fprintf(stderr, "Could not find SCardListCardsA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListCardsW =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCBYTE pbAtr,
			IN      LPCGUID rgquidInterfaces,
			IN      DWORD cguidInterfaceCount,
			OUT     LPWSTR mszCards,
			IN OUT  LPDWORD pcchCards))
		load_func(hOriginal, "SCardListCardsW");
	if (!Original_SCardListCardsW) {
		fprintf(stderr, "Could not find SCardListCardsW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListInterfacesA =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCSTR szCard,
			OUT     LPGUID pguidInterfaces,
			IN OUT  LPDWORD pcguidInterfaces))
		load_func(hOriginal, "SCardListInterfacesA");
	if (!Original_SCardListInterfacesA) {
		fprintf(stderr, "Could not find SCardListInterfacesA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListInterfacesW =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCWSTR szCard,
			OUT     LPGUID pguidInterfaces,
			IN OUT  LPDWORD pcguidInterfaces))
		load_func(hOriginal, "SCardListInterfacesW");
	if (!Original_SCardListInterfacesW) {
		fprintf(stderr, "Could not find SCardListInterfacesW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetProviderIdA =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCSTR szCard,
			OUT     LPGUID pguidProviderId))
		load_func(hOriginal, "SCardGetProviderIdA");
	if (!Original_SCardGetProviderIdA) {
		fprintf(stderr, "Could not find SCardGetProviderIdA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetProviderIdW =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCWSTR szCard,
			OUT     LPGUID pguidProviderId))
		load_func(hOriginal, "SCardGetProviderIdW");
	if (!Original_SCardGetProviderIdW) {
		fprintf(stderr, "Could not find SCardGetProviderIdW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetCardTypeProviderNameA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szCardName,
			IN DWORD dwProviderId,
			OUT LPSTR szProvider,
			IN OUT LPDWORD pcchProvider))
		load_func(hOriginal, "SCardGetCardTypeProviderNameA");
	if (!Original_SCardGetCardTypeProviderNameA) {
		fprintf(stderr, "Could not find SCardGetCardTypeProviderNameA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetCardTypeProviderNameW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szCardName,
			IN DWORD dwProviderId,
			OUT LPWSTR szProvider,
			IN OUT LPDWORD pcchProvider))
		load_func(hOriginal, "SCardGetCardTypeProviderNameW");
	if (!Original_SCardGetCardTypeProviderNameW) {
		fprintf(stderr, "Could not find SCardGetCardTypeProviderNameW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardIntroduceReaderGroupA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szGroupName))
		load_func(hOriginal, "SCardIntroduceReaderGroupA");
	if (!Original_SCardIntroduceReaderGroupA) {
		fprintf(stderr, "Could not find SCardIntroduceReaderGroupA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardIntroduceReaderGroupW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szGroupName))
		load_func(hOriginal, "SCardIntroduceReaderGroupW");
	if (!Original_SCardIntroduceReaderGroupW) {
		fprintf(stderr, "Could not find SCardIntroduceReaderGroupW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardForgetReaderGroupA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szGroupName))
		load_func(hOriginal, "SCardForgetReaderGroupA");
	if (!Original_SCardForgetReaderGroupA) {
		fprintf(stderr, "Could not find SCardForgetReaderGroupA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardForgetReaderGroupW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szGroupName))
		load_func(hOriginal, "SCardForgetReaderGroupW");
	if (!Original_SCardForgetReaderGroupW) {
		fprintf(stderr, "Could not find SCardForgetReaderGroupW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardIntroduceReaderA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szReaderName,
			IN LPCSTR szDeviceName))
		load_func(hOriginal, "SCardIntroduceReaderA");
	if (!Original_SCardIntroduceReaderA) {
		fprintf(stderr, "Could not find SCardIntroduceReaderA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardIntroduceReaderW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szReaderName,
			IN LPCWSTR szDeviceName))
		load_func(hOriginal, "SCardIntroduceReaderW");
	if (!Original_SCardIntroduceReaderW) {
		fprintf(stderr, "Could not find SCardIntroduceReaderW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardForgetReaderA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szReaderName))
		load_func(hOriginal, "SCardForgetReaderA");
	if (!Original_SCardForgetReaderA) {
		fprintf(stderr, "Could not find SCardForgetReaderA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardForgetReaderW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szReaderName))
		load_func(hOriginal, "SCardForgetReaderW");
	if (!Original_SCardForgetReaderW) {
		fprintf(stderr, "Could not find SCardForgetReaderW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardAddReaderToGroupA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szReaderName,
			IN LPCSTR szGroupName))
		load_func(hOriginal, "SCardAddReaderToGroupA");
	if (!Original_SCardAddReaderToGroupA) {
		fprintf(stderr, "Could not find SCardAddReaderToGroupA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardAddReaderToGroupW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szReaderName,
			IN LPCWSTR szGroupName))
		load_func(hOriginal, "SCardAddReaderToGroupW");
	if (!Original_SCardAddReaderToGroupW) {
		fprintf(stderr, "Could not find SCardAddReaderToGroupW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardRemoveReaderFromGroupA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szReaderName,
			IN LPCSTR szGroupName))
		load_func(hOriginal, "SCardRemoveReaderFromGroupA");
	if (!Original_SCardRemoveReaderFromGroupA) {
		fprintf(stderr, "Could not find SCardRemoveReaderFromGroupA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardRemoveReaderFromGroupW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szReaderName,
			IN LPCWSTR szGroupName))
		load_func(hOriginal, "SCardRemoveReaderFromGroupW");
	if (!Original_SCardRemoveReaderFromGroupW) {
		fprintf(stderr, "Could not find SCardRemoveReaderFromGroupW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardIntroduceCardTypeA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szCardName,
			IN LPCGUID pguidPrimaryProvider,
			IN LPCGUID rgguidInterfaces,
			IN DWORD dwInterfaceCount,
			IN LPCBYTE pbAtr,
			IN LPCBYTE pbAtrMask,
			IN DWORD cbAtrLen))
		load_func(hOriginal, "SCardIntroduceCardTypeA");
	if (!Original_SCardIntroduceCardTypeA) {
		fprintf(stderr, "Could not find SCardIntroduceCardTypeA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardIntroduceCardTypeW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szCardName,
			IN LPCGUID pguidPrimaryProvider,
			IN LPCGUID rgguidInterfaces,
			IN DWORD dwInterfaceCount,
			IN LPCBYTE pbAtr,
			IN LPCBYTE pbAtrMask,
			IN DWORD cbAtrLen))
		load_func(hOriginal, "SCardIntroduceCardTypeW");
	if (!Original_SCardIntroduceCardTypeW) {
		fprintf(stderr, "Could not find SCardIntroduceCardTypeW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardSetCardTypeProviderNameA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szCardName,
			IN DWORD dwProviderId,
			IN LPCSTR szProvider))
		load_func(hOriginal, "SCardSetCardTypeProviderNameA");
	if (!Original_SCardSetCardTypeProviderNameA) {
		fprintf(stderr, "Could not find SCardSetCardTypeProviderNameA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardSetCardTypeProviderNameW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szCardName,
			IN DWORD dwProviderId,
			IN LPCWSTR szProvider))
		load_func(hOriginal, "SCardSetCardTypeProviderNameW");
	if (!Original_SCardSetCardTypeProviderNameW) {
		fprintf(stderr, "Could not find SCardSetCardTypeProviderNameW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardForgetCardTypeA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szCardName))
		load_func(hOriginal, "SCardForgetCardTypeA");
	if (!Original_SCardForgetCardTypeA) {
		fprintf(stderr, "Could not find SCardForgetCardTypeA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardForgetCardTypeW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szCardName))
		load_func(hOriginal, "SCardForgetCardTypeW");
	if (!Original_SCardForgetCardTypeW) {
		fprintf(stderr, "Could not find SCardForgetCardTypeW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardAccessStartedEvent =
		(HANDLE(STDCALL *)(void))
		load_func(hOriginal, "SCardAccessStartedEvent");
	if (!Original_SCardAccessStartedEvent) {
		fprintf(stderr, "Could not find SCardAccessStartedEvent procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardReleaseStartedEvent =
		(void(STDCALL *)(void))
		load_func(hOriginal, "SCardReleaseStartedEvent");
	if (!Original_SCardReleaseStartedEvent) {
		fprintf(stderr, "Could not find SCardReleaseStartedEvent procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardLocateCardsA =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCSTR mszCards,
			IN OUT  LPSCARD_READERSTATEA rgReaderStates,
			IN      DWORD cReaders))
		load_func(hOriginal, "SCardLocateCardsA");
	if (!Original_SCardLocateCardsA) {
		fprintf(stderr, "Could not find SCardLocateCardsA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardLocateCardsW =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCWSTR mszCards,
			IN OUT  LPSCARD_READERSTATEW rgReaderStates,
			IN      DWORD cReaders))
		load_func(hOriginal, "SCardLocateCardsW");
	if (!Original_SCardLocateCardsW) {
		fprintf(stderr, "Could not find SCardLocateCardsW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardLocateCardsByATRA =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPSCARD_ATRMASK rgAtrMasks,
			IN      DWORD cAtrs,
			IN OUT  LPSCARD_READERSTATEA rgReaderStates,
			IN      DWORD cReaders))
		load_func(hOriginal, "SCardLocateCardsByATRA");
	if (!Original_SCardLocateCardsByATRA) {
		fprintf(stderr, "Could not find SCardLocateCardsByATRA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardLocateCardsByATRW =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPSCARD_ATRMASK rgAtrMasks,
			IN      DWORD cAtrs,
			IN OUT  LPSCARD_READERSTATEW rgReaderStates,
			IN      DWORD cReaders))
		load_func(hOriginal, "SCardLocateCardsByATRW");
	if (!Original_SCardLocateCardsByATRW) {
		fprintf(stderr, "Could not find SCardLocateCardsByATRW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetStatusChangeA =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      DWORD dwTimeout,
			IN OUT  LPSCARD_READERSTATEA rgReaderStates,
			IN      DWORD cReaders))
		load_func(hOriginal, "SCardGetStatusChangeA");
	if (!Original_SCardGetStatusChangeA) {
		fprintf(stderr, "Could not find SCardGetStatusChangeA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetStatusChangeW =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      DWORD dwTimeout,
			IN OUT  LPSCARD_READERSTATEW rgReaderStates,
			IN      DWORD cReaders))
		load_func(hOriginal, "SCardGetStatusChangeW");
	if (!Original_SCardGetStatusChangeW) {
		fprintf(stderr, "Could not find SCardGetStatusChangeW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardConnectA =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCSTR szReader,
			IN      DWORD dwShareMode,
			IN      DWORD dwPreferredProtocols,
			OUT     LPSCARDHANDLE phCard,
			OUT     LPDWORD pdwActiveProtocol))
		load_func(hOriginal, "SCardConnectA");
	if (!Original_SCardConnectA) {
		fprintf(stderr, "Could not find SCardConnectA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardConnectW =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCWSTR szReader,
			IN      DWORD dwShareMode,
			IN      DWORD dwPreferredProtocols,
			OUT     LPSCARDHANDLE phCard,
			OUT     LPDWORD pdwActiveProtocol))
		load_func(hOriginal, "SCardConnectW");
	if (!Original_SCardConnectW) {
		fprintf(stderr, "Could not find SCardConnectW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardCancelTransaction =
		(LONG(STDCALL *)(
			IN      SCARDHANDLE hCard))
		load_func(hOriginal, "SCardCancelTransaction");
	if (!Original_SCardCancelTransaction) {
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("Could not find SCardCancelTransaction procedure address\n"));
	}

	Original_SCardState =
		(LONG(STDCALL *)(
			IN SCARDHANDLE hCard,
			OUT LPDWORD pdwState,
			OUT LPDWORD pdwProtocol,
			OUT LPBYTE pbAtr,
			IN OUT LPDWORD pcbAtrLen))
		load_func(hOriginal, "SCardState");
	if (!Original_SCardState) {
		fprintf(stderr, "Could not find SCardState procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardStatusA =
		(LONG(STDCALL *)(
			IN SCARDHANDLE hCard,
			OUT LPSTR szReaderName,
			IN OUT LPDWORD pcchReaderLen,
			OUT LPDWORD pdwState,
			OUT LPDWORD pdwProtocol,
			OUT LPBYTE pbAtr,
			IN OUT LPDWORD pcbAtrLen))
		load_func(hOriginal, "SCardStatusA");
	if (!Original_SCardStatusA) {
		fprintf(stderr, "Could not find SCardStatusA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardStatusW =
		(LONG(STDCALL *)(
			IN SCARDHANDLE hCard,
			OUT LPWSTR szReaderName,
			IN OUT LPDWORD pcchReaderLen,
			OUT LPDWORD pdwState,
			OUT LPDWORD pdwProtocol,
			OUT LPBYTE pbAtr,
			IN OUT LPDWORD pcbAtrLen))
		load_func(hOriginal, "SCardStatusW");
	if (!Original_SCardStatusW) {
		fprintf(stderr, "Could not find SCardStatusW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardUIDlgSelectCardA =
		(LONG(STDCALL *)(
			LPOPENCARDNAMEA_EX))
		load_func(hOriginal, "SCardUIDlgSelectCardA");
	if (!Original_SCardUIDlgSelectCardA) {
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("Could not find SCardUIDlgSelectCardA procedure address\n"));
	}

	Original_SCardUIDlgSelectCardW =
		(LONG(STDCALL *)(
			LPOPENCARDNAMEW_EX))
		load_func(hOriginal, "SCardUIDlgSelectCardW");
	if (!Original_SCardUIDlgSelectCardW) {
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("Could not find SCardUIDlgSelectCardW procedure address\n"));
	}

	Original_GetOpenCardNameA =
		(LONG(STDCALL *)(LPOPENCARDNAMEA))
		load_func(hOriginal, "GetOpenCardNameA");
	if (!Original_GetOpenCardNameA) {
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("Could not find GetOpenCardNameA procedure address\n"));
	}

	Original_GetOpenCardNameW =
		(LONG(STDCALL *)(LPOPENCARDNAMEW))
		load_func(hOriginal, "GetOpenCardNameW");
	if (!Original_GetOpenCardNameW) {
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("Could not find GetOpenCardNameW procedure address\n"));
	}

	Original_SCardDlgExtendedError =
		(LONG(STDCALL *)(void))
		load_func(hOriginal, "SCardDlgExtendedError ");
	if (!Original_SCardDlgExtendedError) {
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("Could not find SCardDlgExtendedError procedure address\n"));
	}

	#endif

	return TRUE;
}


#endif