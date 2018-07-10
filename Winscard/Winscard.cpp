#include "stdafx.h"
#include <chrono>
#include <thread>
#include <fstream>
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

#include <winscard.h>
#include "Winscard.h"
#include "CommonFnc.h"

#include <time.h>
#if defined(_WIN32)
#include "socket.h"
#define SCardConnect SCardConnectA
#define SCardStatus SCardStatusA
#define SCardListReaders SCardListReadersA
#define SCardListReaderGroups SCardListReaderGroupsA
#endif
#ifdef __linux__
#include <dlfcn.h>
#include <pwd.h>
#include "wintypes.h"
#endif

/*
extern SCard (.*?) STDCALL.*?(SCard.*?)\((.*?)\);
static SCard \1 (STDCALL *Original_\2)
\3
);
/**/
#pragma warning(disable:4996)   

static string_type LIBRARY_VERSION = _CONV("2.0.1");

static string_type ENV_APDUPLAY_WINSCARD_RULES_PATH = _CONV("APDUPLAY");
static string_type ENV_APDUPLAY_DEBUG_PATH = _CONV("APDUPLAY_DEBUG");
static string_type APDUPLAY_DEBUG_FILE = _CONV("c:\\Temp\\apduplay_debug.txt");

static string_type RULE_FILE = _CONV("winscard_rules.txt");
static string_type WINSCARD_RULES_LOG = _CONV("winscard_rules_log.txt");
static string_type WINSCARD_LOG = _CONV("winscard_log.txt");
static std::string INSTRUCTION_FILE = "Instructions.txt";

// The one and only CWinscardApp object
CWinscardApp theApp;

#define REMOTE_SOCKET_TIMEOUT            5
#define REMOTE_SOCKET_LONG_TIMEOUT       20
static string_type REMOTE_SOCKET_ENDSEQ = _CONV("@@");

//#define HANDLE_VIRTUAL_CARD             0xABADBABE
#define HANDLE_VIRTUAL_CARD             0x1

//BYTE    START_APDU[] = {0xB0,  0x05, 0x01, 0x00, 0x01};
BYTE    START_APDU[] = { 0xB0, 0x01, 0x01, 0x00, 0x0C };

BYTE    PIN_COUNTER_APDU[] = { 0xB0,  0x05, 0x01, 0x00, 0x01 };
BYTE    GET_APDU1[] = { 0x00, 0xC0, 0x00, 0x00 };
BYTE    GET_APDU2[] = { 0xC0, 0xC0, 0x00, 0x00 };

//#define VIRT_READER_NAME        "VirtOpenPGP"
#define VIRT_READER_NAME        "Simona /111.222.123.033@07"
#define VIRTUAL_READERS_LEN     strlen(VIRT_READER_NAME)

#define REMOTE_READER_PREFIX	"Simona"
//#define REMOTE_READER_PREFIX	"Generic EMV Smartcard"


#define CMD_APDU				"APDU"
#define CMD_RESET				"RESET"
#define CMD_ENUM				"ENUM"
#define CMD_LINE_SEPARATOR		"|"	
#define CMD_SEPARATOR			":"	
#define CMD_RESPONSE_FAIL		"FAIL"	


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
to the "originalxx.dll". If we need the data that these variables should be pointing
to, we must GetProcAddress on "originalxx.dll" and use the data there.
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

void LogDebugString(string_type message, bool bInsertTime = true) {
	string_type logLine;

	if (bInsertTime) {
		string_type date_and_time = getCurrentTimeString();
		CCommonFnc::File_AppendString(APDUPLAY_DEBUG_FILE, string_format(_CONV("%s: %s"), date_and_time.c_str(), message.c_str()));
	}
	else {
		CCommonFnc::File_AppendString(APDUPLAY_DEBUG_FILE, message);
	}
}

void LogWinscardRules(string_type message) {
	LogDebugString(message);
	CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
}

void DumpMemory(LPCBYTE location, DWORD length) {
	string_type message;
	CCommonFnc::BYTE_ConvertFromArrayToHexString((BYTE*)location, length, &message);
	CCommonFnc::File_AppendString(WINSCARD_LOG, message);
	CCommonFnc::File_AppendString(WINSCARD_LOG, _CONV("\n"));
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
	LogWinscardRules(message);
	LONG status = (*Original_SCardEstablishContext)(dwScope, pvReserved1, pvReserved2, phContext);
	message = string_format(_CONV("-> hContext:0x%x\n"), *phContext);
	LogWinscardRules(message);
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
	LogWinscardRules(message);
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
	LogWinscardRules(message);
	return (*Original_SCardIsValidContext)(hContext);
}

static SCard LONG(STDCALL *Original_SCardCancel)(
	IN      SCARDCONTEXT hContext
	);

SCard LONG STDCALL SCardCancel(
	IN      SCARDCONTEXT hContext
) {
	LogWinscardRules(_CONV("SCardCancel called\n"));
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
	LogWinscardRules(message);
	if (theApp.IsRemoteCard(hCard)) {
		return SCARD_S_SUCCESS;
		*pdwActiveProtocol = SCARD_PROTOCOL_T1;
	}
	else {
		return (*Original_SCardReconnect)(hCard, dwShareMode, dwPreferredProtocols, dwInitialization, pdwActiveProtocol);
	}
}

static SCard LONG(STDCALL *Original_SCardBeginTransaction)(
	IN      SCARDHANDLE hCard
	);

SCard LONG STDCALL SCardBeginTransaction(
	IN      SCARDHANDLE hCard
) {
	LogWinscardRules(_CONV("SCardBeginTransaction called\n"));
	if (theApp.IsRemoteCard(hCard)) {
		return SCARD_S_SUCCESS;
	}
	else {
		return (*Original_SCardBeginTransaction)(hCard);
	}
}


static SCard LONG(STDCALL *Original_SCardEndTransaction)(
	IN      SCARDHANDLE hCard,
	IN      DWORD dwDisposition
	);

SCard LONG STDCALL SCardEndTransaction(
	IN      SCARDHANDLE hCard,
	IN      DWORD dwDisposition
) {
	LogWinscardRules(_CONV("SCardEndTransaction called\n"));
	if (theApp.IsRemoteCard(hCard)) {
		return SCARD_S_SUCCESS;
	}
	else {
		return (*Original_SCardEndTransaction)(hCard, dwDisposition);
	}
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
	LogWinscardRules(_CONV("SCardControl called\n"));
	if (theApp.IsRemoteCard(hCard)) {
		return SCARD_S_SUCCESS;
	}
	else {
		return (*Original_SCardControl)(hCard, dwControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned);
	}
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
	LogWinscardRules(_CONV("SCardGetAttrib called\n"));
	if (theApp.IsRemoteCard(hCard)) {
		return SCARD_S_SUCCESS;
	}
	else {
		return (*Original_SCardGetAttrib)(hCard, dwAttrId, pbAttr, pcbAttrLen);
	}
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
	LogWinscardRules(_CONV("SCardSetAttrib called\n"));
	if (theApp.IsRemoteCard(hCard)) {
		return SCARD_S_SUCCESS;
	}
	else {
		return (*Original_SCardSetAttrib)(hCard, dwAttrId, pbAttr, cbAttrLen);
	}
}

/* ******************************************************************************* */

CWinscardApp::~CWinscardApp()
{

	iniparser_freedict(instructionDict);

#ifdef __linux
	dlclose(hOriginal);
#else
	FreeLibrary(hOriginal);
#endif
	// Reference to WINSCARD_LOG will fail with access to 0xfeefee (global CString WINSCARD_LOG does not exists at the time of dll release (strange))
	//	if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) CCommonFnc::File_AppendString(WINSCARD_LOG, "[end]\n");

#if defined(_WIN32)
	if (m_remoteConfig.pSocket != NULL) delete m_remoteConfig.pSocket;
#endif
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


void ConvertOuterParenthessis(std::string& description)
{
	int count = 0;
	for (unsigned int i = 0; i < description.length(); ++i)
	{
		if (description[i] == _CONV('('))
		{
			if (count == 0)
			{
				description[i] = _CONV('[');
			}
			++count;
		}
		else if (description[i] == _CONV(')'))
		{
			if (count == 1)
			{
				description[i] = _CONV(']');
			}
			--count;
		}
	}
}

void CWinscardApp::WriteDescription(BYTE insByte)
{
	char_type sec_and_key[256];
	string_type hexNum = string_format(_CONV("%.2x"), insByte);
	char_type* section_name = _CONV("instructions:");
	const char* description;
	string_type prefix = _CONV("instruction info: ");

	type_copy(sec_and_key, section_name);
	type_cat(sec_and_key, hexNum.c_str());

#ifdef UNICODE
	char sec_and_key_char[256];
	wcstombs(sec_and_key_char, sec_and_key, type_length(sec_and_key));
	description = iniparser_getstring(instructionDict, sec_and_key_char, "");

	std::string tmp(description);

	if (strlen(description) != 0)
	{
		ConvertOuterParenthessis(tmp);
		CCommonFnc::File_AppendString(WINSCARD_LOG, prefix + std::wstring(tmp.begin(), tmp.end()));
		CCommonFnc::File_AppendString(WINSCARD_LOG, _CONV("\n"));
	}

#else
	description = iniparser_getstring(instructionDict, sec_and_key, "");
	std::string tmp(description);

	if (strlen(description) != 0)
	{
		ConvertOuterParenthessis(tmp);
		CCommonFnc::File_AppendString(WINSCARD_LOG, prefix + tmp);
		CCommonFnc::File_AppendString(WINSCARD_LOG, _CONV("\n"));
	}
#endif
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
	LogWinscardRules(message);

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
	LogWinscardRules(_CONV("SCardDisconnect called\n"));

	// DISCONNECT FROM CARD
	if (theApp.IsRemoteCard(hCard)) {
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

	LogWinscardRules(_CONV("SCardTransmit called\n"));

	LONG result = SCARD_S_SUCCESS;
	//    DWORD written;
	char_type *txMsg = _CONV("transmitted:");
	char_type *rxMsg = _CONV("received:");
	char_type *crlf = _CONV("\n");
	const int bufferLength = 1024;
	//char_type buffer[bufferLength];
	string_type buffer;
	char_type  sendBuffer[300];
	//clock_t elapsedCard;
	//clock_t elapsedLibrary;
	typedef std::chrono::high_resolution_clock Clock;

	
	string_type     message;

	//elapsedLibrary = -clock();
	auto lib_timestamp1 = Clock::now();
	if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) {
		//sprintf(buffer, "SCardTransmit (handle 0x%0.8X)#\n", hCard);
		buffer = string_format(_CONV("SCardTransmit (handle 0x%0.8X)#\n"), hCard);
		CCommonFnc::File_AppendString(WINSCARD_LOG, buffer);

		//sprintf(buffer, "apduCounter:%d#\n", apduCounter);
		buffer = string_format(_CONV("apduCounter:%d#\n"), apduCounter);
		CCommonFnc::File_AppendString(WINSCARD_LOG, buffer);

		//sprintf(buffer, "totalBytesINCounter:%d#\n", theApp.m_processedApduByteCounter + 1);
		buffer = string_format(_CONV("totalBytesINCounter:%d#\n"), theApp.m_processedApduByteCounter + 1);
		CCommonFnc::File_AppendString(WINSCARD_LOG, buffer);

        if(theApp.m_winscardConfig.bLOG_WRITE_DESCRIPTION)
        {
            theApp.WriteDescription((BYTE)pbSendBuffer[1]); // Send the INS BYTE to the function
        }

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
		LogWinscardRules(message);
		CCommonFnc::BYTE_ConvertFromArrayToHexString((BYTE*)pbSendBuffer, cbSendLength, &message);
		//message.Insert(0, "   "); message += "\n";
		message.insert(0, _CONV("   ")); message += _CONV("\n");
		if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) LogWinscardRules(message);
	}

	// COPY INPUT DATA
	memcpy(sendBuffer, pbSendBuffer, cbSendLength);

	// APPLY INCOMING RULES
	if (theApp.m_winscardConfig.bMODIFY_APDU_BY_RULES) {
        theApp.ApplyRules((BYTE*)sendBuffer, &cbSendLength, INPUT_APDU);
		CCommonFnc::BYTE_ConvertFromArrayToHexString((BYTE*)sendBuffer, cbSendLength, &message);
		//message.Insert(0, "   "); message += "\n";
		message.insert(0, _CONV("   ")); message += _CONV("\n");
		if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) LogWinscardRules(message);
	}

	//elapsedCard = -clock();
	auto card_timestamp1 = Clock::now();

	// INCREASE COUNTER OF THE BYTES SEND TO CARD - IS USED AS MEASUREMENT TRIGGER LATER
	theApp.m_processedApduByteCounter += cbSendLength;

#if defined(_WIN32)
	// Check if redirection is required
	if (theApp.m_remoteConfig.bRedirect) {
	// Check if provided card handle is remote card
		if (theApp.IsRemoteCard(hCard)) {
			// FORWARD TO REMOTE SOCKET 
			result = theApp.Remote_SCardTransmit(&(theApp.m_remoteConfig), theApp.GetReaderName(hCard), (SCARD_IO_REQUEST *)pioSendPci, (LPCBYTE)sendBuffer, cbSendLength, pioRecvPci, pbRecvBuffer, pcbRecvLength);
		}
	}
	else {
#endif
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
#if defined(_WIN32) 	
	}
#endif

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

#if defined(_WIN32)
			// Check if redirection is required
			if (theApp.m_remoteConfig.bRedirect) {
				// Check if provided card handle is remote card
				if (theApp.IsRemoteCard(hCard)) {
					// FORWARD TO REMOTE SOCKET 
					result = theApp.Remote_SCardTransmit(&(theApp.m_remoteConfig), theApp.GetReaderName(hCard), (SCARD_IO_REQUEST *) pioSendPci, (LPCBYTE)sendBuffer, cbSendLength, pioRecvPci, pbRecvBuffer + recvOffset, pcbRecvLength);
				}
			}
			else {
#endif

			result = (*Original_SCardTransmit)(hCard, pioSendPci, (LPCBYTE)sendBuffer, cbSendLength, pioRecvPci, pbRecvBuffer + recvOffset, pcbRecvLength);
#if defined(_WIN32) 	
			}
#endif
			recvOffset = *pcbRecvLength - 2;
		}
	}


	// SAVE TIME OF CARD RESPONSE
	//elapsedCard += clock();
	auto card_timestamp2 = Clock::now();
	auto elapsedCard = std::chrono::duration_cast<std::chrono::milliseconds>(card_timestamp2 - card_timestamp1).count();
	if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) {
		//sprintf(buffer, "responseTime:%d#\n", elapsedCard);
		buffer = string_format(_CONV("responseTime:%d#\n"), elapsedCard);
		CCommonFnc::File_AppendString(WINSCARD_LOG, buffer);

		//sprintf(buffer, "SCardTransmit result:0x%x#\n", result);
		buffer = string_format(_CONV("SCardTransmit result:0x%x#\n"), result);
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
		LogWinscardRules(message);
		CCommonFnc::BYTE_ConvertFromArrayToHexString(pbRecvBuffer, *pcbRecvLength, &message);
		//message.Insert(0, "   "); message += "\n";
		message.insert(0, _CONV("   ")); message += _CONV("\n");
		if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) LogWinscardRules(message);
		// APPLY OUTGOING RULES
		if (theApp.m_winscardConfig.bMODIFY_APDU_BY_RULES) theApp.ApplyRules(pbRecvBuffer, pcbRecvLength, OUTPUT_APDU);
		CCommonFnc::BYTE_ConvertFromArrayToHexString(pbRecvBuffer, *pcbRecvLength, &message);
		//message.Insert(0, "   "); message += "\n";
		message.insert(0, _CONV("   ")); message += _CONV("\n");
		if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) LogWinscardRules(message);
	}


	// increase apdu counter	
	apduCounter++;

	//elapsedLibrary += clock();
	auto lib_timestamp2 = Clock::now();
	auto elapsedLibrary = std::chrono::duration_cast<std::chrono::milliseconds>(lib_timestamp2 - lib_timestamp1).count();
	if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) {
		message = string_format(_CONV("responseTimeLibrary:%d#\n"), elapsedLibrary);
		LogWinscardRules(message);
	}

	return result;
}

static SCard LONG(STDCALL *Original_SCardConnect)(
	IN		SCARDCONTEXT hContext,
	IN		LPCSTR szReader,
	IN		DWORD dwShareMode,
	IN		DWORD dwPreferredProtocols,
	OUT		LPSCARDHANDLE phCard,
	OUT		LPDWORD pdwActiveProtocol);

SCard LONG STDCALL SCardConnect(
	IN		SCARDCONTEXT hContext,
	IN		LPCSTR szReader,
	IN		DWORD dwShareMode,
	IN		DWORD dwPreferredProtocols,
	OUT		LPSCARDHANDLE phCard,
	OUT		LPDWORD pdwActiveProtocol)
{
	LONG status = SCARD_S_SUCCESS;
	if (theApp.m_winscardConfig.bFORCE_CONNECT_SHARED_MODE) {
		// we will always set mode to shared, if required
		dwShareMode = SCARD_SHARE_SHARED;
	}

	// Detect remote cards (now only via reader prefix) and assign virtual card handle
	string_type readerName = szReader;
	if (readerName.find(REMOTE_READER_PREFIX) != -1) {
		theApp.m_nextRemoteCardID++;
		*phCard = theApp.m_nextRemoteCardID;
		theApp.remoteReadersMap[*phCard] = szReader;
		status = theApp.Remote_SCardConnect(&(theApp.m_remoteConfig), szReader);
	}
	else {
		// Standard physical reader
		status = (*Original_SCardConnect)(hContext, szReader, dwShareMode, dwPreferredProtocols, phCard, pdwActiveProtocol);
		string_type message;
		message = string_format(_CONV("SCardConnect(hContext:0x%x,%s,hCard:0x%x) called\n"), hContext, szReader, *phCard);
		LogWinscardRules(message);
	}

	// Store mapping between card handle and reader (used in card remoting)
	theApp.cardReaderMap[*phCard] = szReader;

	return status;
}

static SCard LONG(STDCALL*Original_SCardStatus)(
	SCARDHANDLE hCard,
	LPSTR szReaderName,
	LPDWORD pcchReaderLen,
	LPDWORD pdwState,
	LPDWORD pdwProtocol,
	LPBYTE pbAtr,
	LPDWORD pcbAtrLen
	);

SCard LONG STDCALL SCardStatus(
	SCARDHANDLE hCard,
	LPSTR szReaderName,
	LPDWORD pcchReaderLen,
	LPDWORD pdwState,
	LPDWORD pdwProtocol,
	LPBYTE pbAtr,
	LPDWORD pcbAtrLen)
{
	LogWinscardRules(string_format(_CONV("SCardStatus(hCard:0x%x,szReaderName:%s,pcchReaderLen:%d,pcbAtrLen:%d) called\n"), hCard, szReaderName, *pcchReaderLen, *pcbAtrLen));

	if (theApp.IsRemoteCard(hCard)) {
		// According to https://docs.microsoft.com/en-us/windows/desktop/api/winscard/nf-winscard-scardstatusa
		*pdwState = SCARD_SPECIFIC;
		*pdwProtocol = SCARD_PROTOCOL_T1;

		return SCARD_S_SUCCESS;
	}
	else {
		return (*Original_SCardStatus)(hCard, szReaderName, pcchReaderLen, pdwState, pdwProtocol, pbAtr, pcbAtrLen);
	}
}

static SCard LONG(STDCALL *Original_SCardListReaders)(
	IN      SCARDCONTEXT hContext,
	IN      LPCSTR mszGroups,
	OUT     LPSTR mszReaders,
	IN OUT  LPDWORD pcchReaders
	);

SCard LONG STDCALL SCardListReaders(
	IN      SCARDCONTEXT hContext,
	IN      LPCSTR mszGroups,
	OUT     LPSTR mszReaders,
	IN OUT  LPDWORD pcchReaders)
{
	LogWinscardRules(_CONV("SCardListReaders called\n"));

	int  status = SCARD_S_SUCCESS;
	ls     readersList;

	if (*pcchReaders == SCARD_AUTOALLOCATE) {
		// NO BUFFER IS SUPPLIED

		// OBTAIN REQUIRED LENGTH FOR REAL READERS
		if ((status = (*Original_SCardListReaders)(hContext, mszGroups, NULL, pcchReaders)) == SCARD_S_SUCCESS) {
			// ALLOCATE OWN BUFFER FOR REAL AND VIRTUAL READERS
			DWORD     newLen = (DWORD)(*pcchReaders + VIRTUAL_READERS_LEN + 2);
			char*   readers = new char[newLen];
			memset(readers, 0, newLen);
			*pcchReaders = newLen;
			if ((status = (*Original_SCardListReaders)(hContext, mszGroups, readers, pcchReaders)) == SCARD_S_SUCCESS) {
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
		if ((status = (*Original_SCardListReaders)(hContext, mszGroups, NULL, &realLen)) == SCARD_S_SUCCESS) {
			if ((realLen + VIRTUAL_READERS_LEN > *pcchReaders) || (mszReaders == NULL)) {
				// SUPPLIED BUFFER IS NOT LARGE ENOUGHT
				*pcchReaders = (DWORD) (realLen + VIRTUAL_READERS_LEN);
				if (mszReaders != NULL) status = SCARD_E_INSUFFICIENT_BUFFER;
			}
			else {
				// SUPPLIED BUFFER IS OK, COPY REAL AND VIRTUAL READERS
				realLen = *pcchReaders;
				if ((status = (*Original_SCardListReaders)(hContext, mszGroups, mszReaders, &realLen)) == SCARD_S_SUCCESS) {
					*pcchReaders = realLen;

					// ADD VIRTUAL READER
					// COPY NAME OF VIRTUAL READERS TO END
					memcpy(mszReaders + realLen, VIRT_READER_NAME, strlen(VIRT_READER_NAME));
					*pcchReaders = (DWORD) (realLen + strlen(VIRT_READER_NAME) + 1);
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
					offset += (DWORD) (theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length() + 1);
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

	ls::iterator   iter;
	std::string    availableReaders = "-> Found readers: ";
	for (iter = readersList.begin(); iter != readersList.end(); iter++) {
		availableReaders += *iter;
		availableReaders += ", ";
	}
	availableReaders += "\n";
	LogWinscardRules(availableReaders);

	return status;
}

static SCard LONG(STDCALL *Original_SCardListReaderGroups)(
	IN      SCARDCONTEXT hContext,
	OUT     LPSTR mszGroups,
	IN OUT  LPDWORD pcchGroups
	);

SCard LONG STDCALL SCardListReaderGroups(
	IN      SCARDCONTEXT hContext,
	OUT     LPSTR mszGroups,
	IN OUT  LPDWORD pcchGroups
) {
	LogWinscardRules(_CONV("SCardListReaderGroups called\n"));
	return (*Original_SCardListReaderGroups)(hContext, mszGroups, pcchGroups);
}

// Here are declared only windows specific functions
#if defined (WIN32)

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
	LogWinscardRules(message);

	// RESET APDU IN BYTE COUNTER
	theApp.m_processedApduByteCounter = 0;

	// RESET CARD
	if (theApp.m_remoteConfig.bRedirect && (theApp.m_remoteConfig.pSocket != NULL)) {
		string_type message;
		theApp.m_remoteConfig.pSocket->SendLine(_CONV("get reset 1000"));
		string_type l = theApp.m_remoteConfig.pSocket->ReceiveResponse(REMOTE_SOCKET_ENDSEQ, REMOTE_SOCKET_TIMEOUT);
		message = string_format(_CONV("\n:: %s"), l.c_str());
		//message.Replace("\n", " ");
		replace(message.begin(), message.end(), '\n', ' ');
		LogWinscardRules(message);

		// PREPARE FOR MEASUREMENT
		message = string_format(_CONV("get params 1 %d %d"), theApp.m_remoteConfig.measureApduByteCounter, theApp.m_remoteConfig.measureApduByteDelay);
		theApp.m_remoteConfig.pSocket->SendLine(message);
		l = theApp.m_remoteConfig.pSocket->ReceiveResponse(REMOTE_SOCKET_ENDSEQ, REMOTE_SOCKET_TIMEOUT);
		message = string_format(_CONV(":: %s"), l.c_str());
		LogWinscardRules(message);

		message = string_format(_CONV("post sampling %d"), theApp.m_remoteConfig.numSamples);
		theApp.m_remoteConfig.pSocket->SendLine(message);
		l = theApp.m_remoteConfig.pSocket->ReceiveResponse(REMOTE_SOCKET_ENDSEQ, REMOTE_SOCKET_TIMEOUT);
		message = string_format(_CONV(":: %s"), l.c_str());
		LogWinscardRules(message);

		// PREPARE FOR MEASUREMENT READING IN FUTURE
		theApp.m_remoteConfig.sampleReaded = FALSE;

		// CREATE VIRTUAL CARD HANDLE
		*phCard = HANDLE_VIRTUAL_CARD;
		*pdwActiveProtocol = SCARD_PROTOCOL_T0;

		status = SCARD_S_SUCCESS;
	}
	else {
		status = (*Original_SCardConnectW)(hContext, szReader, dwShareMode, dwPreferredProtocols, phCard, pdwActiveProtocol);
	}

	message = string_format(_CONV("-> hCard:0x%x\n"), *phCard);
	LogWinscardRules(message);

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
	LogWinscardRules(message);

	LONG    status = SCARD_S_SUCCESS;
	lws     readersList;

	if (*pcchReaders == SCARD_AUTOALLOCATE) {
		// NO BUFFER IS SUPPLIED

		// OBTAIN REQUIRED LENGTH FOR REAL READERS
		if ((status = (*Original_SCardListReadersW)(hContext, mszGroups, NULL, pcchReaders)) == SCARD_S_SUCCESS) {
			// ALLOCATE OWN BUFFER FOR REAL AND VIRTUAL READERS
			DWORD     newLen = (DWORD) (*pcchReaders + VIRTUAL_READERS_LEN);
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
				*pcchReaders = (DWORD) (realLen + VIRTUAL_READERS_LEN);
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
						*pcchReaders = (DWORD)(realLen + strlen(VIRT_READER_NAME) + 1);
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
					offset += (DWORD)(theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length() + 1);
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

	lws::iterator   iter;
	std::wstring availableReaders = L"-> Found readers: ";
	for (iter = readersList.begin(); iter != readersList.end(); iter++) {
		availableReaders += *iter;
		availableReaders += L", ";
	}
	availableReaders += L"\n";
	//LogWinscardRules(availableReaders);

	return status;
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
	LogWinscardRules(_CONV("SCardListReaderGroupsW called\n"));
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
	LogWinscardRules(message);
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
	LogWinscardRules(message);
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
	LogWinscardRules(_CONV("SCardListInterfacesA called\n"));
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
	LogWinscardRules(_CONV("SCardListInterfacesW called\n"));
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
	LogWinscardRules(_CONV("SCardGetProviderIdA called\n"));
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
	LogWinscardRules(_CONV("SCardGetProviderIdW called\n"));
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
	LogWinscardRules(_CONV("SCardGetCardTypeProviderNameA called\n"));
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
	LogWinscardRules(_CONV("SCardGetCardTypeProviderNameW called\n"));
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
	LogWinscardRules(_CONV("SCardIntroduceReaderGroupA called\n"));
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
	LogWinscardRules(_CONV("SCardIntroduceReaderGroupW called\n"));
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
	LogWinscardRules(_CONV("SCardForgetReaderGroupA called\n"));
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
	LogWinscardRules(_CONV("SCardForgetReaderGroupW called\n"));
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
	LogWinscardRules(_CONV("SCardIntroduceReaderA called\n"));
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
	LogWinscardRules(_CONV("SCardIntroduceReaderW called\n"));
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
	LogWinscardRules(_CONV("SCardForgetReaderA called\n"));
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
	LogWinscardRules(_CONV("SCardForgetReaderW called\n"));
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
	LogWinscardRules(_CONV("SCardAddReaderToGroupA called\n"));
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
	LogWinscardRules(_CONV("SCardAddReaderToGroupW called\n"));
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
	LogWinscardRules(_CONV("SCardRemoveReaderFromGroupA called\n"));
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
	LogWinscardRules(_CONV("SCardRemoveReaderFromGroupW called\n"));
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
	LogWinscardRules(_CONV("SCardIntroduceCardTypeA called\n"));
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
	LogWinscardRules(_CONV("SCardIntroduceCardTypeW called\n"));
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
	LogWinscardRules(_CONV("SCardSetCardTypeProviderNameA called\n"));
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
	LogWinscardRules(_CONV("SCardSetCardTypeProviderNameW called\n"));
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
	LogWinscardRules(_CONV("SCardForgetCardTypeA called\n"));
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
	LogWinscardRules(_CONV("SCardForgetCardTypeW called\n"));
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
	LogWinscardRules(message);
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
	LogWinscardRules(message);
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
	LogWinscardRules(message);
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
	LogWinscardRules(message);
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
	LogWinscardRules(_CONV("SCardGetStatusChangeA called\n"));
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
	LogWinscardRules(_CONV("SCardGetStatusChangeW called\n"));
	return (*Original_SCardGetStatusChangeW)(hContext, dwTimeout, rgReaderStates, cReaders);
}

static SCard LONG(STDCALL *Original_SCardUIDlgSelectCardA)(
	LPOPENCARDNAMEA_EX
	);

SCard LONG STDCALL SCardUIDlgSelectCardA(
	LPOPENCARDNAMEA_EX a
) {
	LogWinscardRules(_CONV("SCardUIDlgSelectCardA called\n"));
	return (*Original_SCardUIDlgSelectCardA)(a);
}


static SCard LONG(STDCALL *Original_SCardUIDlgSelectCardW)(
	LPOPENCARDNAMEW_EX
	);

SCard LONG STDCALL SCardUIDlgSelectCardW(
	LPOPENCARDNAMEW_EX a
) {
	LogWinscardRules(_CONV("SCardUIDlgSelectCardW called\n"));
	return (*Original_SCardUIDlgSelectCardW)(a);
}

static SCard LONG(STDCALL *Original_GetOpenCardNameA)(
	LPOPENCARDNAMEA
	);

SCard LONG STDCALL GetOpenCardNameA(
	LPOPENCARDNAMEA a
) {
	LogWinscardRules(_CONV("GetOpenCardNameA called\n"));
	return (*Original_GetOpenCardNameA)(a);
}

static SCard LONG(STDCALL *Original_GetOpenCardNameW)(
	LPOPENCARDNAMEW
	);

SCard LONG STDCALL GetOpenCardNameW(
	LPOPENCARDNAMEW a
) {
	LogWinscardRules(_CONV("GetOpenCardNameW called\n"));
	return (*Original_GetOpenCardNameW)(a);
}


static SCard LONG(STDCALL *Original_SCardDlgExtendedError)(void);

SCard LONG STDCALL SCardDlgExtendedError(void) {
	LogWinscardRules(_CONV("SCardDlgExtendedError called\n"));
	return (*Original_SCardDlgExtendedError)();
}

static SCard LONG(STDCALL *Original_SCardCancelTransaction)(
	IN      SCARDHANDLE hCard
	);

SCard LONG STDCALL SCardCancelTransaction(
	IN      SCARDHANDLE hCard
) {
	LogWinscardRules(_CONV("SCardCancelTransaction called\n"));
	if (theApp.IsRemoteCard(hCard)) {
		return SCARD_S_SUCCESS;
	}
	else {
		return (*Original_SCardCancelTransaction)(hCard);
	}
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
	LogWinscardRules(message);
	if (theApp.IsRemoteCard(hCard)) {
		return SCARD_S_SUCCESS;
		*pdwState = SCARD_SPECIFIC;
		*pdwProtocol = SCARD_PROTOCOL_T1;
		// TODO: fill pbAtr
	}
	else {
		return (*Original_SCardState)(hCard, pdwState, pdwProtocol, pbAtr, pcbAtrLen);
	}
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
	LogWinscardRules(message);
	if (theApp.IsRemoteCard(hCard)) {
		return SCARD_S_SUCCESS;
		*pdwState = SCARD_SPECIFIC;
		*pdwProtocol = SCARD_PROTOCOL_T1;
		// TODO: fill pbAtr
	}
	else {
		return (*Original_SCardStatusW)(hCard, szReaderName, pcchReaderLen, pdwState, pdwProtocol, pbAtr, pcbAtrLen);
	}
}

static SCard HANDLE(STDCALL *Original_SCardAccessStartedEvent)(void);

SCard HANDLE STDCALL SCardAccessStartedEvent(void) {
	LogWinscardRules(_CONV("SCardAccessStartedEvent called\n"));
	return (*Original_SCardAccessStartedEvent)();
}


static SCard void(STDCALL *Original_SCardReleaseStartedEvent)(void);

SCard void STDCALL SCardReleaseStartedEvent(void) {
	LogWinscardRules(_CONV("SCardReleaseStartedEvent called\n"));
	return (*Original_SCardReleaseStartedEvent)();
}
#endif

#if __linux__
static void* (*load_func)(void*, const char*) = dlsym;
//Linux specific functions declaration

static LONG(*Original_SCardGetStatusChange)(
	IN      SCARDCONTEXT hContext,
	IN      DWORD dwTimeout,
	IN OUT  SCARD_READERSTATE *rgReaderStates,
	IN      DWORD cReaders
	);

LONG SCardGetStatusChange(
	IN      SCARDCONTEXT hContext,
	IN      DWORD dwTimeout,
	IN OUT  SCARD_READERSTATE *rgReaderStates,
	IN      DWORD cReaders
) {
	LogWinscardRules(_CONV("SCardGetStatusChange called\n"));
	return (*Original_SCardGetStatusChange)(hContext, dwTimeout, rgReaderStates, cReaders);
}


#else 
typedef FARPROC(STDCALL *q) (HMODULE, LPCSTR);
static q load_func = GetProcAddress;
#endif

int initialize()
{
	char *error = "";
#ifdef __linux__ 
	hOriginal = dlopen("/lib/x86_64-linux-gnu/original.so", RTLD_LAZY);
	char *delimeter = ": ";
#else 
#if defined (_WIN32) && !defined(_WIN64)
	hOriginal = LoadLibrary(_CONV("original32.dll"));
#endif
#ifdef _WIN64
	DWORD lastError = 0;
	hOriginal = LoadLibrary(_CONV("original64.dll"));
	lastError = GetLastError();
#endif
	char *delimeter = "";
#endif 

	if (!hOriginal) {
#if __linux__
		error = dlerror();
#endif
		fprintf(stderr, "Failed to load original library%s%s\n", delimeter, error);
		return FALSE;
	}

#if __linux__
	dlerror();    // Clear any existing errors
#endif

#if defined(_WIN32) && !defined(_WIN64)
	Original_SCardTransmit =
		(long(STDCALL *)(unsigned long, const struct _SCARD_IO_REQUEST *, const unsigned char *, unsigned long, struct _SCARD_IO_REQUEST *, unsigned char *, unsigned long *))
		load_func(hOriginal, "SCardTransmit");
#endif
#if _WIN64
	Original_SCardTransmit =
		(long(STDCALL *)(SCARDHANDLE, LPCSCARD_IO_REQUEST, LPCBYTE, DWORD, LPSCARD_IO_REQUEST, LPBYTE, LPDWORD))
		load_func(hOriginal, "SCardTransmit");
#endif
#if __linux__
	Original_SCardTransmit =
		(long(STDCALL *)(SCARDHANDLE, LPCSCARD_IO_REQUEST, const unsigned char *, unsigned long, LPSCARD_IO_REQUEST, unsigned char *, unsigned long *))
		load_func(hOriginal, "SCardTransmit");
#endif
	if ((!Original_SCardTransmit)) {
#if __linux__
		error = dlerror();
#endif
		fprintf(stderr, "Could not find SCardTransmit procedure address%s%s\n", delimeter, error);
		return FALSE;
	}

	Original_SCardDisconnect =
		(long(STDCALL *)(SCARDHANDLE hCard, DWORD dwDisposition))
		load_func(hOriginal, "SCardDisconnect");
	if ((!Original_SCardDisconnect)) {
#if __linux__
		error = dlerror();
#endif
		fprintf(stderr, "Could not find SCardDisconnect procedure address%s%s\n", delimeter, error);
		return FALSE;
	}

	Original_SCardFreeMemory =
		(long(STDCALL *)(SCARDCONTEXT hContext, LPCVOID pvMem))
		load_func(hOriginal, "SCardFreeMemory");
	if ((!Original_SCardFreeMemory)) {
#if __linux__
		error = dlerror();
#endif
		fprintf(stderr, "Could not find SCardFreeMemory procedure address%s%s\n", delimeter, error);
		return FALSE;
	}

	Original_SCardEstablishContext = (LONG(STDCALL *)(
		IN  DWORD dwScope,
		IN  LPCVOID pvReserved1,
		IN  LPCVOID pvReserved2,
		OUT LPSCARDCONTEXT phContext))
		load_func(hOriginal, "SCardEstablishContext");
	if (!Original_SCardEstablishContext) {
#if __linux__
		error = dlerror();
#endif
		fprintf(stderr, "Could not find SCardEstablishContext procedure address%s%s\n", delimeter, error);
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
		fprintf(stderr, "Could not find SCardReleaseContext procedure address%s%s\n", delimeter, error);
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
		fprintf(stderr, "Could not find SCardIsValidContext procedure address%s%s\n", delimeter, error);
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
		fprintf(stderr, "Could not find SCardFreeMemory procedure address%s%s\n", delimeter, error);
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
		fprintf(stderr, "Could not find SCardCancel procedure address%s%s\n", delimeter, error);
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
		fprintf(stderr, "Could not find SCardReconnect procedure address%s%s\n", delimeter, error);
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
		fprintf(stderr, "Could not find SCardDisconnect procedure address%s%s\n", delimeter, error);
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
		fprintf(stderr, "Could not find SCardBeginTransaction procedure address%s%s\n", delimeter, error);
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
		fprintf(stderr, "Could not find SCardEndTransaction procedure address%s%s\n", delimeter, error);
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
		fprintf(stderr, "Could not find SCardTransmit procedure address%s%s\n", delimeter, error);
		return FALSE;
	}

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
		error = dlerror();
#endif
		fprintf(stderr, "Could not find SCardControl procedure address%s%s\n", delimeter, error);
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
		fprintf(stderr, "Could not find SCardGetAttrib procedure address%s%s\n", delimeter, error);
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
		fprintf(stderr, "Could not find SCardSetAttrib procedure address%s%s\n", delimeter, error);
		return FALSE;
	}

	//Initialization of windows specific funcions
#if defined (WIN32)
	Original_SCardStatus =
		(long(STDCALL *)(SCARDHANDLE hCard, LPSTR szReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen))
		load_func(hOriginal, "SCardStatusA");
	if ((!Original_SCardStatus)) {
		fprintf(stderr, "Could not find SCardStatusA procedure address\n");
		return FALSE;
	}

	Original_SCardConnectW =
		(long(STDCALL *)(SCARDCONTEXT hContext, LPCWSTR szReader, DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol))
		load_func(hOriginal, "SCardConnectW");
	if ((!Original_SCardConnectW)) {
		fprintf(stderr, "Could not find SCardConnectW procedure address\n");
		return FALSE;
	}

	Original_SCardListReadersW =
		(long(STDCALL *)(SCARDCONTEXT hContext, LPCWSTR mszGroups, LPWSTR mszReaders, LPDWORD pcchReaders))
		load_func(hOriginal, "SCardListReadersW");
	if ((!Original_SCardListReadersW)) {
		fprintf(stderr, "Could not find SCardListReadersW procedure address\n");
		return FALSE;
	}

	Original_SCardListReaders =
		(long(STDCALL *)(SCARDCONTEXT hContext, LPCSTR mszGroups, LPSTR mszReaders, LPDWORD pcchReaders))
		load_func(hOriginal, "SCardListReadersA");
	if ((!Original_SCardListReaders)) {
		fprintf(stderr, "Could not find SCardListReadersA procedure address\n");
		return FALSE;
	}

	Original_SCardListReaderGroups =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			OUT     LPSTR mszGroups,
			IN OUT  LPDWORD pcchGroups))
		load_func(hOriginal, "SCardListReaderGroupsA");
	if (!Original_SCardListReaderGroups) {
		fprintf(stderr, "Could not find SCardListReaderGroupsA procedure address\n");
		return FALSE;
	}

	Original_SCardListReaderGroupsW =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			OUT     LPWSTR mszGroups,
			IN OUT  LPDWORD pcchGroups))
		load_func(hOriginal, "SCardListReaderGroupsW");
	if (!Original_SCardListReaderGroupsW) {
		fprintf(stderr, "Could not find SCardListReaderGroupsW procedure address\n");
		return FALSE;
	}

	Original_SCardListReaders =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCSTR mszGroups,
			OUT     LPSTR mszReaders,
			IN OUT  LPDWORD pcchReaders))
		load_func(hOriginal, "SCardListReadersA");
	if (!Original_SCardListReaders) {
		fprintf(stderr, "Could not find SCardListReadersA procedure address\n");
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
		fprintf(stderr, "Could not find SCardListReadersW procedure address\n");
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
		fprintf(stderr, "Could not find SCardListCardsA procedure address\n");
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
		fprintf(stderr, "Could not find SCardListCardsW procedure address\n");
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
		fprintf(stderr, "Could not find SCardListInterfacesA procedure address\n");
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
		fprintf(stderr, "Could not find SCardListInterfacesW procedure address\n");
		return FALSE;
	}

	Original_SCardGetProviderIdA =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCSTR szCard,
			OUT     LPGUID pguidProviderId))
		load_func(hOriginal, "SCardGetProviderIdA");
	if (!Original_SCardGetProviderIdA) {
		fprintf(stderr, "Could not find SCardGetProviderIdA procedure address\n");
		return FALSE;
	}

	Original_SCardGetProviderIdW =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCWSTR szCard,
			OUT     LPGUID pguidProviderId))
		load_func(hOriginal, "SCardGetProviderIdW");
	if (!Original_SCardGetProviderIdW) {
		fprintf(stderr, "Could not find SCardGetProviderIdW procedure address\n");
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
		fprintf(stderr, "Could not find SCardGetCardTypeProviderNameA procedure address\n");
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
		fprintf(stderr, "Could not find SCardGetCardTypeProviderNameW procedure address\n");
		return FALSE;
	}

	Original_SCardIntroduceReaderGroupA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szGroupName))
		load_func(hOriginal, "SCardIntroduceReaderGroupA");
	if (!Original_SCardIntroduceReaderGroupA) {
		fprintf(stderr, "Could not find SCardIntroduceReaderGroupA procedure address\n");
		return FALSE;
	}

	Original_SCardIntroduceReaderGroupW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szGroupName))
		load_func(hOriginal, "SCardIntroduceReaderGroupW");
	if (!Original_SCardIntroduceReaderGroupW) {
		fprintf(stderr, "Could not find SCardIntroduceReaderGroupW procedure address\n");
		return FALSE;
	}

	Original_SCardForgetReaderGroupA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szGroupName))
		load_func(hOriginal, "SCardForgetReaderGroupA");
	if (!Original_SCardForgetReaderGroupA) {
		fprintf(stderr, "Could not find SCardForgetReaderGroupA procedure address\n");
		return FALSE;
	}

	Original_SCardForgetReaderGroupW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szGroupName))
		load_func(hOriginal, "SCardForgetReaderGroupW");
	if (!Original_SCardForgetReaderGroupW) {
		fprintf(stderr, "Could not find SCardForgetReaderGroupW procedure address\n");
		return FALSE;
	}

	Original_SCardIntroduceReaderA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szReaderName,
			IN LPCSTR szDeviceName))
		load_func(hOriginal, "SCardIntroduceReaderA");
	if (!Original_SCardIntroduceReaderA) {
		fprintf(stderr, "Could not find SCardIntroduceReaderA procedure address\n");
		return FALSE;
	}

	Original_SCardIntroduceReaderW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szReaderName,
			IN LPCWSTR szDeviceName))
		load_func(hOriginal, "SCardIntroduceReaderW");
	if (!Original_SCardIntroduceReaderW) {
		fprintf(stderr, "Could not find SCardIntroduceReaderW procedure address\n");
		return FALSE;
	}

	Original_SCardForgetReaderA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szReaderName))
		load_func(hOriginal, "SCardForgetReaderA");
	if (!Original_SCardForgetReaderA) {
		fprintf(stderr, "Could not find SCardForgetReaderA procedure address\n");
		return FALSE;
	}

	Original_SCardForgetReaderW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szReaderName))
		load_func(hOriginal, "SCardForgetReaderW");
	if (!Original_SCardForgetReaderW) {
		fprintf(stderr, "Could not find SCardForgetReaderW procedure address\n");
		return FALSE;
	}

	Original_SCardAddReaderToGroupA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szReaderName,
			IN LPCSTR szGroupName))
		load_func(hOriginal, "SCardAddReaderToGroupA");
	if (!Original_SCardAddReaderToGroupA) {
		fprintf(stderr, "Could not find SCardAddReaderToGroupA procedure address\n");
		return FALSE;
	}

	Original_SCardAddReaderToGroupW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szReaderName,
			IN LPCWSTR szGroupName))
		load_func(hOriginal, "SCardAddReaderToGroupW");
	if (!Original_SCardAddReaderToGroupW) {
		fprintf(stderr, "Could not find SCardAddReaderToGroupW procedure address\n");
		return FALSE;
	}

	Original_SCardRemoveReaderFromGroupA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szReaderName,
			IN LPCSTR szGroupName))
		load_func(hOriginal, "SCardRemoveReaderFromGroupA");
	if (!Original_SCardRemoveReaderFromGroupA) {
		fprintf(stderr, "Could not find SCardRemoveReaderFromGroupA procedure address\n");
		return FALSE;
	}

	Original_SCardRemoveReaderFromGroupW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szReaderName,
			IN LPCWSTR szGroupName))
		load_func(hOriginal, "SCardRemoveReaderFromGroupW");
	if (!Original_SCardRemoveReaderFromGroupW) {
		fprintf(stderr, "Could not find SCardRemoveReaderFromGroupW procedure address\n");
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
		fprintf(stderr, "Could not find SCardIntroduceCardTypeA procedure address\n");
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
		fprintf(stderr, "Could not find SCardIntroduceCardTypeW procedure address\n");
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
		fprintf(stderr, "Could not find SCardSetCardTypeProviderNameA procedure address\n");
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
		fprintf(stderr, "Could not find SCardSetCardTypeProviderNameW procedure address\n");
		return FALSE;
	}

	Original_SCardForgetCardTypeA =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szCardName))
		load_func(hOriginal, "SCardForgetCardTypeA");
	if (!Original_SCardForgetCardTypeA) {
		fprintf(stderr, "Could not find SCardForgetCardTypeA procedure address\n");
		return FALSE;
	}

	Original_SCardForgetCardTypeW =
		(LONG(STDCALL *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szCardName))
		load_func(hOriginal, "SCardForgetCardTypeW");
	if (!Original_SCardForgetCardTypeW) {
		fprintf(stderr, "Could not find SCardForgetCardTypeW procedure address\n");
		return FALSE;
	}

	Original_SCardAccessStartedEvent =
		(HANDLE(STDCALL *)(void))
		load_func(hOriginal, "SCardAccessStartedEvent");
	if (!Original_SCardAccessStartedEvent) {
		fprintf(stderr, "Could not find SCardAccessStartedEvent procedure address\n");
		return FALSE;
	}

	Original_SCardReleaseStartedEvent =
		(void(STDCALL *)(void))
		load_func(hOriginal, "SCardReleaseStartedEvent");
	if (!Original_SCardReleaseStartedEvent) {
		fprintf(stderr, "Could not find SCardReleaseStartedEvent procedure address\n");
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
		fprintf(stderr, "Could not find SCardLocateCardsA procedure address\n");
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
		fprintf(stderr, "Could not find SCardLocateCardsW procedure address\n");
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
		fprintf(stderr, "Could not find SCardLocateCardsByATRA procedure address\n");
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
		fprintf(stderr, "Could not find SCardLocateCardsByATRW procedure address\n");
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
		fprintf(stderr, "Could not find SCardGetStatusChangeA procedure address\n");
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
		fprintf(stderr, "Could not find SCardGetStatusChangeW procedure address\n");
		return FALSE;
	}

	Original_SCardConnect =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCSTR szReader,
			IN      DWORD dwShareMode,
			IN      DWORD dwPreferredProtocols,
			OUT     LPSCARDHANDLE phCard,
			OUT     LPDWORD pdwActiveProtocol))
		load_func(hOriginal, "SCardConnectA");
	if (!Original_SCardConnect) {
		fprintf(stderr, "Could not find SCardConnectA procedure address\n");
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
		fprintf(stderr, "Could not find SCardConnectW procedure address\n");
		return FALSE;
	}

	Original_SCardCancelTransaction =
		(LONG(STDCALL *)(
			IN      SCARDHANDLE hCard))
		load_func(hOriginal, "SCardCancelTransaction");
	if (!Original_SCardCancelTransaction) {
		LogWinscardRules(_CONV("Could not find SCardCancelTransaction procedure address\n"));
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
		fprintf(stderr, "Could not find SCardState procedure address\n");
		return FALSE;
	}

	Original_SCardStatus =
		(LONG(STDCALL *)(
			IN SCARDHANDLE hCard,
			OUT LPSTR szReaderName,
			IN OUT LPDWORD pcchReaderLen,
			OUT LPDWORD pdwState,
			OUT LPDWORD pdwProtocol,
			OUT LPBYTE pbAtr,
			IN OUT LPDWORD pcbAtrLen))
		load_func(hOriginal, "SCardStatusA");
	if (!Original_SCardStatus) {
		fprintf(stderr, "Could not find SCardStatusA procedure address\n");
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
		fprintf(stderr, "Could not find SCardStatusW procedure address\n");
		return FALSE;
	}

	Original_SCardUIDlgSelectCardA =
		(LONG(STDCALL *)(
			LPOPENCARDNAMEA_EX))
		load_func(hOriginal, "SCardUIDlgSelectCardA");
	if (!Original_SCardUIDlgSelectCardA) {
		LogWinscardRules(_CONV("Could not find SCardUIDlgSelectCardA procedure address\n"));
	}

	Original_SCardUIDlgSelectCardW =
		(LONG(STDCALL *)(
			LPOPENCARDNAMEW_EX))
		load_func(hOriginal, "SCardUIDlgSelectCardW");
	if (!Original_SCardUIDlgSelectCardW) {
		LogWinscardRules(_CONV("Could not find SCardUIDlgSelectCardW procedure address\n"));
	}

	Original_GetOpenCardNameA =
		(LONG(STDCALL *)(LPOPENCARDNAMEA))
		load_func(hOriginal, "GetOpenCardNameA");
	if (!Original_GetOpenCardNameA) {
		LogWinscardRules(_CONV("Could not find GetOpenCardNameA procedure address\n"));
	}

	Original_GetOpenCardNameW =
		(LONG(STDCALL *)(LPOPENCARDNAMEW))
		load_func(hOriginal, "GetOpenCardNameW");
	if (!Original_GetOpenCardNameW) {
		LogWinscardRules(_CONV("Could not find GetOpenCardNameW procedure address\n"));
	}

	Original_SCardDlgExtendedError =
		(LONG(STDCALL *)(void))
		load_func(hOriginal, "SCardDlgExtendedError ");
	if (!Original_SCardDlgExtendedError) {
		LogWinscardRules(_CONV("Could not find SCardDlgExtendedError procedure address\n"));
	}

#endif

	//Linux specific funcion initialization
#if __linux__

	Original_SCardConnect =
		(LONG(STDCALL *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCSTR szReader,
			IN      DWORD dwShareMode,
			IN      DWORD dwPreferredProtocols,
			OUT     LPSCARDHANDLE phCard,
			OUT     LPDWORD pdwActiveProtocol))
		load_func(hOriginal, "SCardConnect");
	if (!Original_SCardConnect) {
		error = dlerror();
		fprintf(stderr, "Could not find SCardConnect procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardStatus =
		(long(*)(SCARDHANDLE hCard, LPSTR szReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen))
		load_func(hOriginal, "SCardStatus");
	if ((!Original_SCardStatus)) {
		error = dlerror();
		fprintf(stderr, "Could not find SCardStatus procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetStatusChange =
		(LONG(*)(
			IN      SCARDCONTEXT hContext,
			IN      DWORD dwTimeout,
			IN OUT  SCARD_READERSTATE *rgReaderStates,
			IN      DWORD cReaders))
		load_func(hOriginal, "SCardGetStatusChange");
	if (!Original_SCardGetStatusChange) {
		error = dlerror();
		fprintf(stderr, "Could not find SCardGetStatusChange procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListReaders =
		(long(*)(SCARDCONTEXT hContext, LPCSTR mszGroups, LPSTR mszReaders, LPDWORD pcchReaders))
		load_func(hOriginal, "SCardListReaders");
	if ((!Original_SCardListReaders)) {
		error = dlerror();
		fprintf(stderr, "Could not find SCardListReaders procedure address %s\n", error);
		return FALSE;
	}

	Original_SCardListReaderGroups =
		(LONG(*)(
			IN      SCARDCONTEXT hContext,
			OUT     LPSTR mszGroups,
			IN OUT  LPDWORD pcchGroups))
		load_func(hOriginal, "SCardListReaderGroups");
	if (!Original_SCardListReaderGroups) {
		error = dlerror();
		fprintf(stderr, "Could not find SCardListReaderGroups procedure address:  %s\n", error);
		return FALSE;
	}

#endif

	return TRUE;
}


#endif

// CWinscardApp initialization

// CWinscardApp

#if defined (_WIN32)
BEGIN_MESSAGE_MAP(CWinscardApp, CWinApp)
END_MESSAGE_MAP()
#endif

// CWinscardApp construction

CWinscardApp::CWinscardApp()
{
	m_bRulesActive = FALSE;

	#ifdef __linux__
		LoadRules();
		initialize();
	#endif

	m_processedApduByteCounter = 0;
}


void GetDesktopPath(char_type* path)
{
#ifdef __linux__
	char* login;
	struct passwd *pass;
	pass = getpwuid(getuid());
	login = pass->pw_name;

	string_type stringPath = "/home/";
    stringPath += login;
    stringPath += "/Desktop/APDUPlay/";
    type_copy(rulesFilePath, stringPath.c_str());
#else
	char_type appData[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, appData);
	type_copy(path, appData);
	type_cat(path, _CONV("/APDUPlay/"));
#endif
}



#if defined (_WIN32)
BOOL CWinscardApp::InitInstance()
{
	CWinApp::InitInstance();
	// Check for user-defined path for debug file
	char* debugPath = std::getenv(ENV_APDUPLAY_DEBUG_PATH.c_str());
	if (debugPath != NULL) {
		APDUPLAY_DEBUG_FILE = debugPath;
	}

    srand((int) time(NULL));
	LogDebugString(_CONV("#####################################################################################\n"), false);
	LogDebugString(_CONV("InitInstance entered\n"));

#if defined (_WIN32) && !defined(_WIN64)
	LogDebugString(string_format(_CONV("APDUPlay winscard.dll v%s (32-bit version)\n"), LIBRARY_VERSION.c_str()));
#endif
#ifdef _WIN64
	LogDebugString(string_format(_CONV("APDUPlay winscard.dll v%s, (64-bit version)\n"), LIBRARY_VERSION.c_str()));
#endif


    // LOAD MODIFICATION RULES
    LoadRules();
	LogDebugString(_CONV("After LoadRules\n"));

    // CONNECT TO REMOTE SOCKET IF REQUIRED
    if (m_remoteConfig.bRedirect) {
		LogDebugString(_CONV("[REMOTE] Redirect = 1 => going to connect to specified socket (make sure socket is opened. If application terminates, try to set Redirect = 0 to disable remote redirecting)\n"));
		Remote_Connect(&m_remoteConfig);
    }
	if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) CCommonFnc::File_AppendString(WINSCARD_LOG, _CONV("[begin]\n"));

	BOOL bStatus = initialize();
	LogDebugString(string_format(_CONV("Finalizing InitInstance with %s\n"), bStatus ? _CONV("true") : _CONV("false")));

	return bStatus;
}

int CWinscardApp::Remote_Connect(REMOTE_CONFIG* pRemoteConfig) {
    string_type     message;
    
    string_type sIP(pRemoteConfig->IP);
	try {
		message = string_format(_CONV("Connnecting to remote proxy with IP:port = %s:%s\n"), pRemoteConfig->IP.c_str(), pRemoteConfig->port.c_str());
		LogWinscardRules(message);
		pRemoteConfig->pSocket = new SocketClient(sIP, type_to_int(pRemoteConfig->port.c_str(), NULL, 10));
	}
	catch (std::string error) {
		message = string_format(_CONV("Failed to connect to %s:%s (error: %s)\n"), pRemoteConfig->IP.c_str(), pRemoteConfig->port.c_str(), error);
		LogWinscardRules(message);
	}
	catch (...) {
		message = string_format(_CONV("Failed to connect to %s:%s\n"), pRemoteConfig->IP.c_str(), pRemoteConfig->port.c_str());
		LogWinscardRules(message);
	}

    return STAT_OK;
}

#endif


int CWinscardApp::ApplyRules(BYTE* pbBuffer, DWORD* pcbLength, int direction) {
    int             status = STAT_OK;

    if (m_bRulesActive) {
        //lar::iterator   iter;
        //lasr::iterator  iter2;
        APDU_SINGLE_RULE    singleRule;    
        BYTE            tempBuffer[MAX_APDU_LENGTH];
        BYTE            newBuffer[MAX_APDU_LENGTH];
        BOOL            bRuleFound = FALSE;
        
        // MAKE TEMP COPY
        memcpy(newBuffer, pbBuffer, *pcbLength);
        
        // PROCESS ALL RULES, IF MATCH THEN MODIFY BUFFER
        for (auto iter = rulesList.begin(); iter != rulesList.end(); iter++) {
            if ((iter->direction == direction) && (iter->usage == 1)) {
                //TEST ALL MATCH RULES
                BOOL    bAllMatch = TRUE;
                for (auto iter2 = iter->matchRules.begin(); iter2 != iter->matchRules.end(); iter2++) {
                    singleRule = *iter2;
                    // OBTAIN REFFERED APDU FROM HISTORY
                    memset(tempBuffer, 0, MAX_APDU_LENGTH);
                    GetApduFromHistory(tempBuffer, singleRule.history, singleRule.apduDirection);
					
                    if (tempBuffer[singleRule.element] == singleRule.value) {
                        // RULE MATCH     
                    }
                    else {
                        bAllMatch = FALSE; 
                        break;
                    }
                }

                // IF ALL MATCH THEN APPLY CHANGE RULES
                if (bAllMatch) {
					for (auto iter8 = iter->matchRules.begin(); iter8 != iter->matchRules.end(); iter8++) {
					}
                
                    for (auto iter2 = iter->actionRules.begin(); iter2 != iter->actionRules.end(); iter2++) {
                        singleRule = *iter2;
                        if (singleRule.valid) {
                            if (singleRule.element == LE_ELEM) *pcbLength = singleRule.value;
                            else newBuffer[singleRule.element] = singleRule.value;
                        }
                    }
                    
                    // NONDETERMINISTIC SLEEP IF REQUIRED
                    if (iter->msDelay > 0) {
						std::this_thread::sleep_for(std::chrono::milliseconds(iter->msDelay));
                        //_sleep(iter->msDelay);
                        // SLEEP RANDOMLY UP TO 1/10 OF ORIGINAL TIME
						std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (iter->msDelay / 10)));
                        //_sleep(rand() % (iter->msDelay / 10));
                    }
                    
                    // RULE FOUND
                    bRuleFound = TRUE;
                }
            }    
        }
        
        // CORRECT LENGTH, IF AT LEAST ONE RULE WAS APPLIED
        // IF NO RULE APPLIED, THAN KEEP SAME - WILL PROCESS EVEN MALLFORMED APDU WITH LC > 0 && no input data
        if (direction == INPUT_APDU && bRuleFound) {
            *pcbLength = pbBuffer[4] + OFFSET_CDATA;   // LC length
        } 

        // COPY RESULTING BUFFER BACK
        memcpy(pbBuffer, newBuffer, *pcbLength);
    }
    else {
        // RULES ARE NOT ACTIVE
    }        
    
    return status;
}

int CWinscardApp::GetApduFromHistory(BYTE* buffer, int history, int apduDirection) {
    int             status = STAT_OK;
    lab::iterator   iter;
    APDU_BUFFER     apduBuff;
    // GET APDU FROM APPROPRIATE SLOT
    if (apduDirection == INPUT_APDU) {
        iter = apduInList.begin();
        while (history > 0) {
            iter++;
            history--;
        }
        if (iter != apduInList.end()) {
            apduBuff = *iter;
            memcpy(buffer, apduBuff.buffer, sizeof(APDU_BUFFER));
        }
        else status = STAT_NOT_ENOUGHT_DATA_TYPE;
    }
    else {
        iter = apduOutList.begin();
        while (history > 0) iter++;
        if (iter != apduInList.end()) memcpy(buffer, iter->buffer, sizeof(APDU_BUFFER));
        else status = STAT_NOT_ENOUGHT_DATA_TYPE;
    }
    
    return status;
}

/**
Format request for remote proxy

Remote card protocol
 > <card reader name>
 > <cmd ID>:<"APDU" / "RESET" / "ENUM">:<optional hexa string, e.g. "00A4040304">
 # some comments
 all other lines not starting with > or # are ignored
*/
string_type Remote_FormatRequest(string_type targetReader, DWORD uniqueCmdID, string_type command, string_type commandData, string_type notes, string_type lineSeparator) {
	return string_format(">%s%s>%d%s%s%s%s%s%s", targetReader.c_str(), lineSeparator.c_str(), uniqueCmdID, CMD_SEPARATOR, command.c_str(), CMD_SEPARATOR, commandData.c_str(), lineSeparator.c_str(), notes.c_str());
}

#if defined (_WIN32)
LONG CWinscardApp::Remote_SCardConnect(REMOTE_CONFIG* pRemoteConfig, string_type targetReader) {
	LONG        status = 0;
	string_type     message;

	if (pRemoteConfig->pSocket != NULL) {
		try {
			// unique command ID
			theApp.m_remoteConfig.nextCommandID++;
			string_type l = Remote_FormatRequest(targetReader, theApp.m_remoteConfig.nextCommandID, CMD_RESET, "", "", CMD_LINE_SEPARATOR);
			pRemoteConfig->pSocket->SendLine(l);
			//message.Insert(0, "\n::-> ");
			message.insert(0, _CONV("\n::-> "));
			LogWinscardRules(message);
			_sleep(500);

			// OBTAIN RESPONSE, PARSE BACK 
			// TODO: parse response, propagate it, chck unique command ID
			l = pRemoteConfig->pSocket->ReceiveResponse(REMOTE_SOCKET_ENDSEQ, REMOTE_SOCKET_TIMEOUT);

			// Parse response
			string_type response;
			status = Remote_ParseResponse(l, theApp.m_remoteConfig.nextCommandID, &response);

			message = string_format(_CONV("\n::<- %s\n"), response.c_str());
			replace(message.begin(), message.end(), '\n', ' ');
			LogWinscardRules(message);
		}
		catch (const char* s) {
			message = string_format(_CONV("Remote_SCardConnect(), SendLine(%s), fail with (%s)"), message.c_str(), s);
			LogWinscardRules(message);
			status = SCARD_F_UNKNOWN_ERROR;
		}
		catch (...) {
			message = string_format(_CONV("Remote_SCardConnect(), SendLine(%s), fail with (unhandled exception)"), message.c_str());
			LogWinscardRules(message);
			status = SCARD_F_UNKNOWN_ERROR;
		}
	}
	else {
		status = SCARD_F_COMM_ERROR;
	}

	return status;
}
#endif

#if defined (_WIN32)

LONG CWinscardApp::Remote_ParseResponse(string_type rawResponse, DWORD expectedCommandID, string_type* response) {
	LONG status = SCARD_S_SUCCESS;

	size_t pos = 0;
	if (rawResponse.at(pos) != '>') {
		LogWinscardRules("'>'was expected at begin");
		status = SCARD_F_COMM_ERROR;
	}
	pos++;

	if (status == SCARD_S_SUCCESS) {
		size_t pos2 = rawResponse.find(CMD_SEPARATOR);
		string_type uniqueCmdID = rawResponse.substr(pos, pos2 - 1);
		if (expectedCommandID != atoi(uniqueCmdID.c_str())) {
			LogWinscardRules("Unexpected commandID in response");
			status = SCARD_F_COMM_ERROR;
		}
		pos = pos2 + 1;
	}

	if (status == SCARD_S_SUCCESS) {
		size_t pos2 = rawResponse.find(REMOTE_SOCKET_ENDSEQ, pos);
		*response = rawResponse.substr(pos, pos2 - pos);
	}

	return status;
}



LONG CWinscardApp::Remote_SCardTransmit(REMOTE_CONFIG* pRemoteConfig, string_type targetReader, SCARD_IO_REQUEST* pioSendPci, LPCBYTE pbSendBuffer, DWORD cbSendLength, SCARD_IO_REQUEST* pioRecvPci, LPBYTE pbRecvBuffer, LPDWORD pcbRecvLength) {
    LONG			status = 0;
    string_type     message;
    string_type     value;    
    
    if (pRemoteConfig->pSocket != NULL) {
        try {
			// unique command ID
			theApp.m_remoteConfig.nextCommandID++;
			CCommonFnc::BYTE_ConvertFromArrayToHexString(pbSendBuffer, cbSendLength, &value);
			string_type l = Remote_FormatRequest(targetReader, theApp.m_remoteConfig.nextCommandID, CMD_APDU, value, "", CMD_LINE_SEPARATOR);
            pRemoteConfig->pSocket->SendLine(l);
            //message.Insert(0, "\n::-> ");
			message.insert(0, _CONV("\n::-> "));
            LogWinscardRules(message);
            
            // SLEEP LONGER, IF MORE DATA WILL BE RETURNED BY SYSTEM 00 0c 00 00 xx CALL            
            if (memcmp(pbSendBuffer, GET_APDU1, sizeof(GET_APDU1)) == 0 || memcmp(pbSendBuffer, GET_APDU2, sizeof(GET_APDU2)) == 0) {
                _sleep(pbSendBuffer[4] * 20);       // LC * 20ms
            }
            else _sleep(500);
            
            // OBTAIN RESPONSE, PARSE BACK 
            l = pRemoteConfig->pSocket->ReceiveResponse(REMOTE_SOCKET_ENDSEQ, REMOTE_SOCKET_TIMEOUT);

			string_type response;
			status = Remote_ParseResponse(l, theApp.m_remoteConfig.nextCommandID, &response);

            message = string_format(_CONV("\n::<- %s\n"), l.c_str());
            //message.Replace("\n", " ");
			replace(message.begin(), message.end(), '\n', ' ');
            LogWinscardRules(message);
            
            if (response.find(CMD_RESPONSE_FAIL) == string_type::npos) {
                // RESPONSE CORRECT
                // NOTE: pbRecvBuffer IS ASSUMED TO HAVE 260B
                *pcbRecvLength = 260;
                status = CCommonFnc::BYTE_ConvertFromHexStringToArray(response, pbRecvBuffer, pcbRecvLength);
                
                // CHECK FOR RETURN STATUS, AT LEAST 2 BYTES REQUIRED
                if (*pcbRecvLength < 2) status = STAT_DATA_INCORRECT_LENGTH;
            }
            else {
                // COMMAND FAIL 
                status = STAT_SCARD_ERROR;
            }
        }
        catch (const char* s) {
            message = string_format(_CONV("\nRemote_SCardTransmit(), SendLine(%s), fail with (%s)"), message.c_str(), s);
            LogWinscardRules(message);
            status = SCARD_F_UNKNOWN_ERROR;
        } 
        catch (...) {
            message = string_format(_CONV("\nRemote_SCardTransmit(), SendLine(%s), fail with (unhandled exception)"), message.c_str());
            LogWinscardRules(message);
            status = SCARD_F_UNKNOWN_ERROR;
        }
    }

    return status;
}
#endif

#if !defined(_WIN32) || !defined(UNICODE)
int CWinscardApp::LoadRule(const char_type* section_name, dictionary* dict/*string_type filePath*/) {
	int status = STAT_OK;
	string_type valueName;
	string_type rulePart;
	string_type ruleString;
	string_type elemName;
	string_type subValue;
	string_type help;
	APDU_RULE rule;
	APDU_SINGLE_RULE singleRule;
	char_type sec_and_key[256];
	const char_type* char_value;
	int value;
	string_type section_name_string = section_name;

	if (compareWithNoCase(section_name, _CONV("WINSCARD")) == 0)
	{
		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":AUTO_REQUEST_DATA")), 2)) != 2)
		{
			m_winscardConfig.bAUTO_REQUEST_DATA = value;
		}

		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":FORCE_CONNECT_SHARED_MODE")), 2)) != 2)
		{
			m_winscardConfig.bFORCE_CONNECT_SHARED_MODE = value;
		}

		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":FORCE_APDU_NONZERO_INPUT_DATA")), 2)) != 2)
		{
			m_winscardConfig.bFORCE_APDU_NONZERO_INPUT_DATA = value;
		}

		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":LOG_EXCHANGED_APDU")), 2)) != 2)
		{
			m_winscardConfig.bLOG_EXCHANGED_APDU = value;
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":LOG_BASE_PATH")), "");
		if(type_length(char_value) != 0)
		{
			m_winscardConfig.sLOG_BASE_PATH = char_value;
		}
		
		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":MODIFY_APDU_BY_RULES")), 2)) != 2)
		{
			m_winscardConfig.bMODIFY_APDU_BY_RULES = value;
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":READER_ORDERED_FIRST")), "");
		if (type_length(char_value) != 0)
		{
			m_winscardConfig.sREADER_ORDERED_FIRST = char_value;
		}
	}

#if defined (_WIN32)
	if (compareWithNoCase(section_name, _CONV("REMOTE")) == 0)
	{
		// REMOTE SOCKET CONFIGURATION RULE

		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":REDIRECT")), 2)) != 2)
		{
			m_remoteConfig.bRedirect = value;
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":IP")), "");
		if (type_length(char_value) != 0)
		{
			m_remoteConfig.IP = char_value;
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":PORT")), "");
		if (type_length(char_value) != 0)
		{
			m_remoteConfig.port = char_value;
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":MEASURE_APDU")), "");
		if (type_length(char_value) != 0)
		{
			m_remoteConfig.measureApduLen = sizeof(m_remoteConfig.measureApdu);
			CCommonFnc::BYTE_ConvertFromHexStringToArray(char_value, m_remoteConfig.measureApdu, &(m_remoteConfig.measureApduLen));
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":MEASURE_BYTE_COUNTER")), "");
		if (type_length(char_value) != 0)
		{
			m_remoteConfig.measureApduByteCounter = type_to_int(char_value, NULL, 10);
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":MEASURE_BYTE_DELAY")), "");
		if (type_length(char_value) != 0)
		{
			m_remoteConfig.measureApduByteDelay = type_to_int(char_value, NULL, 10);
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":READ_RATIO")), "");
		if (type_length(char_value) != 0)
		{
			m_remoteConfig.readRatio = type_to_int(char_value, NULL, 10);
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":NUM_SAMPLES")), "");
		if (type_length(char_value) != 0)
		{
			m_remoteConfig.numSamples = type_to_int(char_value, NULL, 10);
		}
	}
#endif
	
	if (compareWithNoCase(section_name_string.substr(0, (int) type_length(_CONV("RULE"))).c_str(), _CONV("RULE")) == 0)
	{
		// COMMON RULE
		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":USAGE")), "");
		if (type_length(char_value) != 0)
		{
			rule.usage = type_to_int(char_value, NULL, 10);
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":APDUIN")), "");
		if (type_length(char_value) != 0)
		{
			rule.direction = type_to_int(char_value, NULL, 10);
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":DELAY")), "");
		if (type_length(char_value) != 0)
		{
			rule.msDelay = type_to_int(char_value, NULL, 10);
		}

		// SET RULE NAME FOR FUTURE IDENTIFICATION
		rule.ruleName = section_name_string;

		// LOAD MATCH RULES
		int counter = 1;
		size_t pos = 0;
		size_t pos2 = 0;
		valueName = string_format(_CONV(":MATCH%d"), counter);
		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, valueName.c_str()), "");
		while (type_length(char_value) != 0)
		{
			ruleString = char_value;
			ruleString += _CONV(" ");

			// FIND HISTORY ELEMENT, WILL BE SAME FOR ALL OTHER ELEMENTAREY RULES
			if ((pos = ruleString.find(_CONV("t="))) != string_type::npos)
			{
				//singleRule.history = atoi(ruleString.substr(pos + (int) type_length(_CONV("t="))).c_str());
				singleRule.history = type_to_int(ruleString.substr(pos + (int)type_length(_CONV("t="))).c_str(), NULL, 10);
				ruleString.erase(pos, ruleString.find(_CONV(","), pos) - pos + 1); // remove from rule string

			}
			// FIND DIRECTION ELEMENT (IN/OUT), WILL BE SAME FOR ALL OTHER ELEMENTAREY RULES
			if ((pos = ruleString.find(_CONV("in="))) != string_type::npos)
			{
				singleRule.apduDirection = type_to_int(ruleString.substr(pos + (int) type_length(_CONV("in="))).c_str(), NULL, 10);
				ruleString.erase(pos, ruleString.find(_CONV(","), pos) - pos + 1); // remove from rule string
			}

			// PARSE RULE AND CREATE ELEMENTARY RULES FOREACH BYTE
			pos2 = 0;
			while ((pos = ruleString.find(_CONV(","), pos2)) != string_type::npos)
			{
				rulePart = ruleString.substr(pos2, pos - pos2 + 1);

				//elemName = rulePart.Left(rulePart.Find("="));
				elemName = rulePart.substr(0, rulePart.find(_CONV("=")));

				if (compareWithNoCase(elemName.c_str(), _CONV("CLA")) == 0)
				{
                    singleRule.element = CLA_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.matchRules.push_back(singleRule);

				}
				if (compareWithNoCase(elemName.c_str(), _CONV("INS")) == 0)
				{
					singleRule.element = INS_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.matchRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("P1")) == 0)
				{
					singleRule.element = P1_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.matchRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("P2")) == 0)
				{
					singleRule.element = P2_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.matchRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("LC")) == 0)
				{
					singleRule.element = LC_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.matchRules.push_back(singleRule);
				}

				if (compareWithNoCase(elemName.substr(0, (int)type_length(_CONV("DATA"))).c_str(), _CONV("DATA")) == 0)
				{
					// DATA CAN BE WRITTEN IN MORE VALUES AT ONCE, STARTING ON POSITION DATAx
					// CREATE SEPARATE ELEMENT FOR EACH
					int offset = type_to_int(elemName.substr(elemName.find_first_of(_CONV("0123456789")), 0).c_str(), NULL, 10);
					// GO OVER ALL MATCH DATA
					string_type data = rulePart.substr(rulePart.find(_CONV("=")) + 1);
					//data.Replace(";", "");
					data.erase(std::remove(data.begin(), data.end(), ','), data.end());
					BYTE dataBuffer[300];
					DWORD dataBufferLen = 300;
					CCommonFnc::BYTE_ConvertFromHexStringToArray(data, dataBuffer, &dataBufferLen);

					for (DWORD i = 0; i < dataBufferLen; i++)
					{
						if (singleRule.apduDirection == INPUT_APDU) singleRule.element = offset + OFFSET_CDATA;
						else singleRule.element = offset;
						singleRule.value = dataBuffer[i];
						singleRule.valid = TRUE;
						rule.matchRules.push_back(singleRule);
						// increase offset for next element
						offset++;
					}
				}

				pos2 = pos + 1;
			}

			counter++;
			valueName = string_format(_CONV("MATCH%d"), counter);
			type_copy(sec_and_key, section_name);
			char_value = iniparser_getstring(dict, type_cat(sec_and_key, valueName.c_str()), "");
		}

		// LOAD ACTION RULES
		counter = 1;
		pos = 0;
		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, ":ACTION"), "");
		if (type_length(char_value) != 0)
		{
            ruleString = char_value;
			ruleString += _CONV(" ");
			// PARSE RULE AND CREATE ELEMENTARY RULES FOREACH BYTE
			singleRule.clear();
			// FIND DIRECTION ELEMENT (IN/OUT), WILL BE SAME FOR ALL OTHER ELEMENTARY RULES
			if ((pos = ruleString.find(_CONV("in="))) != string_type::npos)
			{
				singleRule.apduDirection = type_to_int(ruleString.substr(pos + (int) type_length(_CONV("in="))).c_str(), NULL, 10);
				//ruleString.Delete(pos, ruleString.find(";", pos) - pos + 1); // remove from rule string
				ruleString.erase(pos, ruleString.find(_CONV(","), pos) - pos + 1);
			}
			pos2 = 0;
			while ((pos = ruleString.find(_CONV(","), pos2)) != string_type::npos)
			{
				rulePart = ruleString.substr(pos2, pos - pos2 + 1);

				elemName = rulePart.substr(0, rulePart.find(_CONV("=")));

				if (compareWithNoCase(elemName.c_str(), _CONV("CLA")) == 0)
				{
					singleRule.element = CLA_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.actionRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("INS")) == 0)
				{
					singleRule.element = INS_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.actionRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("P1")) == 0)
				{
					singleRule.element = P1_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.actionRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("P2")) == 0)
				{
					singleRule.element = P2_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.actionRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("LC")) == 0)
				{
					singleRule.element = LC_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.actionRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("LE")) == 0)
				{
					singleRule.element = LE_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.actionRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.substr(0, (int) type_length(_CONV("DATA"))).c_str(), _CONV("DATA")) == 0)
				{
					// DATA CAN BE WRITTEN IN MORE VALUES AT ONCE, STARTING ON POSITION DATAx
					// CREATE SEPARATE ELEMENT FOR EACH
					int offset = type_to_int(elemName.substr(section_name_string.find_first_of(_CONV("0123456789"))).c_str(), NULL, 10);
					// GO OVER ALL MATCH DATA
					string_type data = rulePart.substr(rulePart.find(_CONV("=")) + 1);
					//data.Replace(";", "");
					data.erase(std::remove(data.begin(), data.end(), ','), data.end());
					BYTE dataBuffer[300];
					DWORD dataBufferLen = 300;
					CCommonFnc::BYTE_ConvertFromHexStringToArray(data, dataBuffer, &dataBufferLen);
					for (DWORD i = 0; i < dataBufferLen; i++)
					{
						if (singleRule.apduDirection == INPUT_APDU) singleRule.element = offset + OFFSET_CDATA;
						else singleRule.element = offset;
						singleRule.value = dataBuffer[i];
						singleRule.valid = TRUE;
						rule.actionRules.push_back(singleRule);
						// increase offset for next element
						offset++;
					}
				}

				pos2 = pos + 1;
			}
		}
		rulesList.push_back(rule);
	}

	return status;
}

int CWinscardApp::LoadRules() {
	int status = STAT_OK;
	FILE *file = NULL;
	char_type rulesFilePath[256];
	char_type baseDir[256];	// base rulesFilePath for input/output files
	type_copy(rulesFilePath, RULE_FILE.c_str());

	string_type date_and_time = getCurrentTimeString();

	WINSCARD_LOG += _CONV("_") + date_and_time + _CONV(".txt");
	WINSCARD_RULES_LOG += _CONV("_") + date_and_time + _CONV(".txt");
	
	//
	// Searching for 'winscard_rules.txt' (priority) 
	// 1. Lookup in local directory
	// 2. Lookup for APDUPLAY environmental variable
	// (UNUSED) 3. Lookup on user Desktop 

	if (!file) {  // 1. Lookup in local directory
		LogDebugString(string_format(_CONV("Going to open %s ... "), rulesFilePath));
		file = fopen(rulesFilePath, "r");
		if (file) {
			LogDebugString(_CONV("success\n"), false);
			type_copy(baseDir, "");
		}
		else {
			LogDebugString(_CONV("fail\n"), false);
		}
	}

	if (!file) { // 2. Lookup for APDUPLAY environmental variable
		char* configPath = std::getenv(ENV_APDUPLAY_WINSCARD_RULES_PATH.c_str());
		LogDebugString(string_format(_CONV("Going to query %s env variable ... "), ENV_APDUPLAY_WINSCARD_RULES_PATH.c_str()));
		if (configPath != NULL) {
			LogDebugString(string_format(_CONV(" defined (%s)\n"), configPath), false);
			// variable detected, try to open 
			string newRuleFile = string_format(_CONV("%s\\%s"), configPath, RULE_FILE.c_str());
			LogDebugString(string_format(_CONV("Going to open %s ... "), newRuleFile.c_str()));
			file = fopen(newRuleFile.c_str(), "r");
			if (file) {
				LogDebugString(_CONV("success\n"), false);
				type_copy(rulesFilePath, newRuleFile.c_str());
				type_copy(baseDir, configPath);
			}
			else {
				LogDebugString(_CONV("fail\n"), false);
			}
		}
		else {
			LogDebugString(_CONV("not found\n"), false);
		}
	}

/*
	if (!file) { // 3. Lookup on user Desktop
		GetDesktopPath(rulesFilePath);
		type_cat(rulesFilePath, RULE_FILE.c_str());
		file = fopen(rulesFilePath, "r"); // try to open file on desktop
	}
*/
    if (file) {
		fclose(file);
		dictionary* dict = iniparser_load(rulesFilePath);

		int number_of_sections = iniparser_getnsec(dict);

		for (int i = 0; i < number_of_sections; ++i)
		{
			const char* section_name = iniparser_getsecname(dict, i);
			LoadRule(section_name, dict);
		}

		m_bRulesActive = TRUE;

		iniparser_freedict(dict);
	}
	else {
		LogDebugString(_CONV("Rules file NOT found\n"));
		LogWinscardRules(_CONV("Rules file NOT found\n"));
	}

	if (!m_winscardConfig.sLOG_BASE_PATH.empty())
	{
		LogDebugString(string_format(_CONV("Logging base path is forced as '%s' (LOG_BASE_PATH)\n"), m_winscardConfig.sLOG_BASE_PATH.c_str()));

		// Use provided directory to store output files
		WINSCARD_RULES_LOG = string_format(_CONV("%s\\%s"), m_winscardConfig.sLOG_BASE_PATH.c_str(), WINSCARD_RULES_LOG.c_str());
		WINSCARD_LOG = string_format(_CONV("%s\\%s"), m_winscardConfig.sLOG_BASE_PATH.c_str(), WINSCARD_LOG.c_str());
		INSTRUCTION_FILE = string_format(_CONV("%s\\%s"), m_winscardConfig.sLOG_BASE_PATH.c_str(), INSTRUCTION_FILE.c_str());
	}
	else
	{
		LogDebugString(string_format(_CONV("Logging base path not forced, using '%s'\n"), baseDir));

		// Use base directory to store output files
		WINSCARD_RULES_LOG = string_format(_CONV("%s%s"), baseDir, WINSCARD_RULES_LOG.c_str());
		WINSCARD_LOG = string_format(_CONV("%s%s"), baseDir, WINSCARD_LOG.c_str());
		INSTRUCTION_FILE = string_format(_CONV("%s%s"), baseDir, INSTRUCTION_FILE.c_str());
	}

	std::fstream instruction_file;
	instruction_file.open(INSTRUCTION_FILE, std::ios::in);

	if (instruction_file.is_open())
	{
		theApp.m_winscardConfig.bLOG_WRITE_DESCRIPTION = TRUE;
		instruction_file.close();
		LogWinscardRules(_CONV("Instruction file found"));
		instructionDict = iniparser_load((const char*) INSTRUCTION_FILE.c_str());
	}

	return status;
}
#endif

/**
Returns reader name corresponding to provided card handle
*/
string_type CWinscardApp::GetReaderName(IN SCARDHANDLE hCard) {
	return theApp.cardReaderMap[hCard];
}

/**
Returns reader name corresponding to provided card handle
*/
boolean CWinscardApp::IsRemoteCard(IN SCARDHANDLE hCard) {
	auto it = theApp.remoteReadersMap.find(hCard);
	return it != theApp.remoteReadersMap.end();
}


#if defined(_WIN32) && defined(_UNICODE)
int CWinscardApp::LoadRule(string_type ruleName, string_type filePath) {
	int     status = STAT_OK;
	char_type    buffer[10000];
	DWORD   cBuffer = 10000;
	string_type valueName;
	string_type rulePart;
	string_type ruleString;
	string_type elemName;
	string_type subValue;
	string_type help;
	APDU_RULE   rule;
	APDU_SINGLE_RULE    singleRule;

	if (compareWithNoCase(ruleName.c_str(), _CONV("WINSCARD")) == 0) {
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("AUTO_REQUEST_DATA"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_winscardConfig.bAUTO_REQUEST_DATA = (type_to_int(buffer, NULL, 10) == 0) ? FALSE : TRUE;
		}
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("FORCE_CONNECT_SHARED_MODE"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_winscardConfig.bFORCE_CONNECT_SHARED_MODE = (type_to_int(buffer, NULL, 10) == 0) ? FALSE : TRUE;
		}
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("FORCE_APDU_NONZERO_INPUT_DATA"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_winscardConfig.bFORCE_APDU_NONZERO_INPUT_DATA = (type_to_int(buffer, NULL, 10) == 0) ? FALSE : TRUE;
		}
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("LOG_EXCHANGED_APDU"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_winscardConfig.bLOG_EXCHANGED_APDU = (type_to_int(buffer, NULL, 10) == 0) ? FALSE : TRUE;
		}
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("LOG_BASE_PATH"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_winscardConfig.sLOG_BASE_PATH = buffer;
		}
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("MODIFY_APDU_BY_RULES"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_winscardConfig.bMODIFY_APDU_BY_RULES = (type_to_int(buffer, NULL, 10) == 0) ? FALSE : TRUE;
		}
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("READER_ORDERED_FIRST"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_winscardConfig.sREADER_ORDERED_FIRST = buffer;
		}

	}

	if (compareWithNoCase(ruleName.c_str(), _CONV("REMOTE")) == 0) {
		// REMOTE CONFIGURATION RULE
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("REDIRECT"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_remoteConfig.bRedirect = (type_to_int(buffer, NULL, 10) == 0) ? FALSE : TRUE;
		}
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("IP"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_remoteConfig.IP = buffer;
		}
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("PORT"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_remoteConfig.port = buffer;
		}
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("MEASURE_APDU"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_remoteConfig.measureApduLen = sizeof(m_remoteConfig.measureApdu);
			CCommonFnc::BYTE_ConvertFromHexStringToArray(buffer, m_remoteConfig.measureApdu, &(m_remoteConfig.measureApduLen));
		}
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("MEASURE_BYTE_COUNTER"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_remoteConfig.measureApduByteCounter = type_to_int(buffer, NULL, 10);
		}
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("MEASURE_BYTE_DELAY"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_remoteConfig.measureApduByteDelay = type_to_int(buffer, NULL, 10);
		}
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("READ_RATIO"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_remoteConfig.readRatio = type_to_int(buffer, NULL, 10);
		}
		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("NUM_SAMPLES"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			m_remoteConfig.numSamples = type_to_int(buffer, NULL, 10);
		}



	}
	if (compareWithNoCase(ruleName.substr(0, (int)type_length(_CONV("RULE"))).c_str(), _CONV("RULE")) == 0) {
		// COMMON RULE

		if ((GetPrivateProfileString(ruleName.c_str(), _CONV("USAGE"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
			rule.usage = type_to_int(buffer, NULL, 10);

			if ((GetPrivateProfileString(ruleName.c_str(), _CONV("APDUIN"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
				rule.direction = type_to_int(buffer, NULL, 10);
			}
			if ((GetPrivateProfileString(ruleName.c_str(), _CONV("DELAY"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
				rule.msDelay = type_to_int(buffer, NULL, 10);
			}


			// SET RULE NAME FOR FUTURE IDENTIFICATION
			rule.ruleName = ruleName;

			// LOAD MATCH RULES
			int counter = 1;
			int pos = 0;
			int pos2 = 0;
			valueName = string_format(_CONV("MATCH%d"), counter);
			while ((GetPrivateProfileString(ruleName.c_str(), valueName.c_str(), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
				ruleString = buffer; ruleString += _CONV(" ");

				// FIND HISTORY ELEMENT, WILL BE SAME FOR ALL OTHER ELEMENTAREY RULES
				if ((pos = ruleString.find(_CONV("t="))) != string_type::npos) {
					//singleRule.history = atoi(ruleString.substr(pos + (int) type_length(_CONV("t="))).c_str());
					singleRule.history = type_to_int(ruleString.substr(pos + (int)type_length(_CONV("t="))).c_str(), NULL, 10);
					ruleString.erase(pos, ruleString.find(_CONV(";"), pos) - pos + 1); // remove from rule string
				}
				// FIND DIRECTION ELEMENT (IN/OUT), WILL BE SAME FOR ALL OTHER ELEMENTAREY RULES
				if ((pos = ruleString.find(_CONV("in="))) != string_type::npos) {
					singleRule.apduDirection = type_to_int(ruleString.substr(pos + (int)type_length(_CONV("in="))).c_str(), NULL, 10);
					ruleString.erase(pos, ruleString.find(_CONV(";"), pos) - pos + 1); // remove from rule string
				}

				// PARSE RULE AND CREATE ELEMENTARY RULES FOREACH BYTE
				pos2 = 0;
				while ((pos = ruleString.find(_CONV(";"), pos2)) != -1) {
					rulePart = ruleString.substr(pos2, pos - pos2 + 1);

					//elemName = rulePart.Left(rulePart.Find("="));
					elemName = rulePart.substr(0, rulePart.find(_CONV("=")));

					if (compareWithNoCase(elemName.c_str(), _CONV("CLA")) == 0) {
						singleRule.element = CLA_ELEM;
						CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
						singleRule.valid = TRUE; rule.matchRules.push_back(singleRule);
					}
					if (compareWithNoCase(elemName.c_str(), _CONV("INS")) == 0) {
						singleRule.element = INS_ELEM;
						CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
						singleRule.valid = TRUE; rule.matchRules.push_back(singleRule);
					}
					if (compareWithNoCase(elemName.c_str(), _CONV("P1")) == 0) {
						singleRule.element = P1_ELEM;
						CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
						singleRule.valid = TRUE; rule.matchRules.push_back(singleRule);
					}
					if (compareWithNoCase(elemName.c_str(), _CONV("P2")) == 0) {
						singleRule.element = P2_ELEM;
						CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
						singleRule.valid = TRUE; rule.matchRules.push_back(singleRule);
					}
					if (compareWithNoCase(elemName.c_str(), _CONV("LC")) == 0) {
						singleRule.element = LC_ELEM;
						CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
						singleRule.valid = TRUE; rule.matchRules.push_back(singleRule);
					}
					if (compareWithNoCase(elemName.substr(0, (int)type_length(_CONV("DATA"))).c_str(), _CONV("DATA")) == 0) {
						// DATA CAN BE WRITTEN IN MORE VALUES AT ONCE, STARTING ON POSITION DATAx
						// CREATE SEPARATE ELEMENT FOR EACH
						int offset = type_to_int(elemName.substr(ruleName.find_first_of(_CONV("0123456789")), 0).c_str(), NULL, 10);
						// GO OVER ALL MATCH DATA
						string_type data = rulePart.substr(rulePart.find(_CONV("=")) + 1);
						//data.Replace(";", "");
						data.erase(remove(data.begin(), data.end(), ';'), data.end());
						BYTE    dataBuffer[300];
						DWORD   dataBufferLen = 300;
						CCommonFnc::BYTE_ConvertFromHexStringToArray(data, dataBuffer, &dataBufferLen);
						for (DWORD i = 0; i < dataBufferLen; i++) {
							if (singleRule.apduDirection == INPUT_APDU) singleRule.element = offset + OFFSET_CDATA;
							else singleRule.element = offset;
							singleRule.value = dataBuffer[i];
							singleRule.valid = TRUE; rule.matchRules.push_back(singleRule);
							// increase offset for next element
							offset++;
						}
					}

					pos2 = pos + 1;
				}

				counter++;
				valueName = string_format(_CONV("MATCH%d"), counter);
			}

			// LOAD ACTION RULES
			counter = 1;
			pos = 0;
			if ((GetPrivateProfileString(ruleName.c_str(), _CONV("ACTION"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
				ruleString = buffer; ruleString += _CONV(" ");
				// PARSE RULE AND CREATE ELEMENTARY RULES FOREACH BYTE
				singleRule.clear();
				// FIND DIRECTION ELEMENT (IN/OUT), WILL BE SAME FOR ALL OTHER ELEMENTARY RULES
				if ((pos = ruleString.find(_CONV("in="))) != string_type::npos) {
					singleRule.apduDirection = type_to_int(ruleString.substr(pos + (int)type_length(_CONV("in="))).c_str(), NULL, 10);
					//ruleString.Delete(pos, ruleString.find(";", pos) - pos + 1); // remove from rule string
					ruleString.erase(pos, ruleString.find(_CONV(";"), pos) - pos + 1);
				}
				pos2 = 0;
				while ((pos = ruleString.find(_CONV(";"), pos2)) != string_type::npos) {
					rulePart = ruleString.substr(pos2, pos - pos2 + 1);

					elemName = rulePart.substr(0, rulePart.find(_CONV("=")));

					if (compareWithNoCase(elemName.c_str(), _CONV("CLA")) == 0) {
						singleRule.element = CLA_ELEM;
						CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
						singleRule.valid = TRUE; rule.actionRules.push_back(singleRule);
					}
					if (compareWithNoCase(elemName.c_str(), _CONV("INS")) == 0) {
						singleRule.element = INS_ELEM;
						CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
						singleRule.valid = TRUE; rule.actionRules.push_back(singleRule);
					}
					if (compareWithNoCase(elemName.c_str(), _CONV("P1")) == 0) {
						singleRule.element = P1_ELEM;
						CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
						singleRule.valid = TRUE; rule.actionRules.push_back(singleRule);
					}
					if (compareWithNoCase(elemName.c_str(), _CONV("P2")) == 0) {
						singleRule.element = P2_ELEM;
						CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
						singleRule.valid = TRUE; rule.actionRules.push_back(singleRule);
					}
					if (compareWithNoCase(elemName.c_str(), _CONV("LC")) == 0) {
						singleRule.element = LC_ELEM;
						CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
						singleRule.valid = TRUE; rule.actionRules.push_back(singleRule);
					}
					if (compareWithNoCase(elemName.c_str(), _CONV("LE")) == 0) {
						singleRule.element = LE_ELEM;
						CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
						singleRule.valid = TRUE; rule.actionRules.push_back(singleRule);
					}
					if (compareWithNoCase(elemName.substr(0, (int)type_length(_CONV("DATA"))).c_str(), _CONV("DATA")) == 0) {
						// DATA CAN BE WRITTEN IN MORE VALUES AT ONCE, STARTING ON POSITION DATAx
						// CREATE SEPARATE ELEMENT FOR EACH
						int offset = type_to_int(elemName.substr(ruleName.find_first_of(_CONV("0123456789"))).c_str(), NULL, 10);
						// GO OVER ALL MATCH DATA
						string_type data = rulePart.substr(rulePart.find(_CONV("=")) + 1);
						//data.Replace(";", "");
						data.erase(remove(data.begin(), data.end(), ';'), data.end());
						BYTE    dataBuffer[300];
						DWORD   dataBufferLen = 300;
						CCommonFnc::BYTE_ConvertFromHexStringToArray(data, dataBuffer, &dataBufferLen);
						for (DWORD i = 0; i < dataBufferLen; i++) {
							if (singleRule.apduDirection == INPUT_APDU) singleRule.element = offset + OFFSET_CDATA;
							else singleRule.element = offset;
							singleRule.value = dataBuffer[i];
							singleRule.valid = TRUE; rule.actionRules.push_back(singleRule);
							// increase offset for next element
							offset++;
						}
					}

					pos2 = pos + 1;
				}
			}

			rulesList.push_back(rule);
		}
	}

	return status;
}

int CWinscardApp::LoadRules() {

	int status = STAT_OK;
	char_type    buffer[10000];
	DWORD   cBuffer = 10000;
	DWORD   cReaded = 0;
	lws     valuesList;
	string_type filePath;
	FILE* file;
	
	const int buffsize = 4096;
	TCHAR buf[buffsize] = _CONV("");
	
	memset(buffer, 0, cBuffer);

	char_type rulesFilePath[256];
	type_copy(rulesFilePath, RULE_FILE.c_str());

	string_type date_and_time = getCurrentTimeString();

	WINSCARD_LOG += _CONV("_") + date_and_time + _CONV(".txt");
	WINSCARD_RULES_LOG += _CONV("_") + date_and_time + _CONV(".txt");

	if (!(file = type_fopen(rulesFilePath, _CONV("r")))) // try to open file in current directory
	{
		type_copy(rulesFilePath, GetDesktopPath());
		type_cat(rulesFilePath, RULE_FILE.c_str());
		file = type_fopen(rulesFilePath, _CONV("r")); // try to open file on desktop
	}

	if (file) {

		// OBTAIN FULL FILE PATH FOR RULES FILE
		GetFullPathName(RULE_FILE.c_str(), buffsize, buf, NULL);
		fclose(file);

		filePath = string_type(buf);

		string_type message;
		message = string_format(_CONV("Rules file found: %s\n"), filePath.c_str());
		LogWinscardRules(message);

		// OBTAIN SECTION NAMES
		if ((cReaded = GetPrivateProfileString(NULL, NULL, _CONV(""), buffer, cBuffer, rulesFilePath)) != 0) {
			// PARSE SECTION NAMES, TRY TO LOAD EACH RULE

			CCommonFnc::String_ParseNullSeparatedArray(buffer, cBuffer, &valuesList);

			for (auto iter = valuesList.begin(); iter != valuesList.end(); iter++) {
				LoadRule(*iter, rulesFilePath);
			}
		}
		m_bRulesActive = TRUE;
	}
	else {
		LogWinscardRules(_CONV("Rules file NOT found\n"));
	}

	if (!m_winscardConfig.sLOG_BASE_PATH.empty())
	{
		WINSCARD_RULES_LOG = string_format(_CONV("%s%s"), m_winscardConfig.sLOG_BASE_PATH.c_str(), WINSCARD_RULES_LOG.c_str());
		WINSCARD_LOG = string_format(_CONV("%s%s"), m_winscardConfig.sLOG_BASE_PATH.c_str(), WINSCARD_LOG.c_str());
	}
	else
	{
		WINSCARD_RULES_LOG = string_format(_CONV("%s%s"), GetDesktopPath(), WINSCARD_RULES_LOG.c_str());
		WINSCARD_LOG = string_format(_CONV("%s%s"), GetDesktopPath(), WINSCARD_LOG.c_str());
	}

	std::fstream instruction_file;
	instruction_file.open(INSTRUCTION_FILE, std::ios::in);

	if (instruction_file.is_open())
	{
		theApp.m_winscardConfig.bLOG_WRITE_DESCRIPTION = TRUE;
		instruction_file.close();
		LogWinscardRules(_CONV("Instruction file found"));
		instructionDict = iniparser_load((const char*)INSTRUCTION_FILE.c_str());
	}

	return status;
}
#endif

