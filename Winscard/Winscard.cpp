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
#include "winscard.h"
#include "CommonFnc.h"
#include <time.h>
#ifdef _DEBUG
#define new DEBUG_NEW
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
extern WINSCARDAPI (.*?) WINAPI.*?(SCard.*?)\((.*?)\);
static WINSCARDAPI \1 (WINAPI *Original_\2)
\3
);
/**/
#pragma warning(disable:4996)   

#define RULE_FILE           "winscard_rules.txt"
string WINSCARD_RULES_LOG = "winscard_rules_log.txt";
string WINSCARD_LOG =       "winscard_log.txt";


#define SCSAT_SOCKET_TIMEOUT            5
#define SCSAT_SOCKET_LONG_TIMEOUT       20
#define SCSAT_SOCKET_ENDSEQ             "@@"

//#define HANDLE_VIRTUAL_CARD             0xABADBABE
#define HANDLE_VIRTUAL_CARD             0x1

//BYTE    START_APDU[] = {0xB0,  0x05, 0x01, 0x00, 0x01};
BYTE    START_APDU[] = {0xB0, 0x01, 0x01, 0x00, 0x0C};

BYTE    PIN_COUNTER_APDU[] = {0xB0,  0x05, 0x01, 0x00, 0x01};
BYTE    GET_APDU1[] = {0x00, 0xC0, 0x00, 0x00};
BYTE    GET_APDU2[] = {0xC0, 0xC0, 0x00, 0x00};

//#define VIRT_READER_NAME        "VirtOpenPGP"
#define VIRT_READER_NAME        ""
#define VIRTUAL_READERS_LEN     strlen(VIRT_READER_NAME)

/* ******************************************************************************* */

static HANDLE  hOut=0;
static HMODULE hOriginal=0;

int apduCounter = 0;

/* ******************************************************************************* */

/* The following values variables MUST be defined here, but MUST NOT be referenced
   in this or any other program module. The DEF file is set to forward their linkage
   to the "original.dll". If we need the data that these variables should be pointing
   to, we must GetProcAddress on "original.dll" and use the data there.
*/
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

    
/* ******************************************************************************* */

int compareWithNoCase(const char* str1, const char* str2) {
	char *str1_2 = new char[strlen(str1)];
	char *str2_2 = new char[strlen(str2)];
	strcpy(str1_2, str1);
	strcpy(str2_2, str2);
	toupper(*str1_2);
	toupper(*str2_2);
	int result = strcmp(str1_2, str2_2);
	delete[] str1_2;
	delete[] str2_2;
	return result;
}

void DumpMemory( LPCBYTE location, DWORD length ) {
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
	string message;
	CCommonFnc::BYTE_ConvertFromArrayToHexString((BYTE*) location, length, &message);
	CCommonFnc::File_AppendString(WINSCARD_LOG, message);
	CCommonFnc::File_AppendString(WINSCARD_LOG, "\r\n");

}

static WINSCARDAPI LONG (WINAPI *Original_SCardEstablishContext)(
    IN  DWORD dwScope,
    IN  LPCVOID pvReserved1,
    IN  LPCVOID pvReserved2,
    OUT LPSCARDCONTEXT phContext
);

WINSCARDAPI LONG WINAPI SCardEstablishContext(
    IN  DWORD dwScope,
    IN  LPCVOID pvReserved1,
    IN  LPCVOID pvReserved2,
    OUT LPSCARDCONTEXT phContext
){
    string message;
	message = string_format("SCardEstablishContext() called\n");
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    LONG status = (*Original_SCardEstablishContext)(dwScope,pvReserved1,pvReserved2,phContext);
    //message.Format("-> hContext:0x%x\n", *phContext);
	message = string_format("-> hContext:0x%x\n", *phContext);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    return status;
}


static WINSCARDAPI LONG (WINAPI *Original_SCardReleaseContext)(
    IN      SCARDCONTEXT hContext
);

WINSCARDAPI LONG WINAPI SCardReleaseContext(
    IN      SCARDCONTEXT hContext
){
    string message;
	message = string_format("SCardReleaseContext(hContext:0x%x) called\n", hContext);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    return (*Original_SCardReleaseContext)(hContext);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardIsValidContext)(
    IN      SCARDCONTEXT hContext
);

WINSCARDAPI LONG WINAPI SCardIsValidContext(
    IN      SCARDCONTEXT hContext
){
    string message;
	message = string_format("SCardIsValidContext(hContext:0x%x) called\n", hContext);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    return (*Original_SCardIsValidContext)(hContext);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardListReaderGroupsA)(
    IN      SCARDCONTEXT hContext,
    OUT     LPSTR mszGroups,
    IN OUT  LPDWORD pcchGroups
);

WINSCARDAPI LONG WINAPI SCardListReaderGroupsA(
    IN      SCARDCONTEXT hContext,
    OUT     LPSTR mszGroups,
    IN OUT  LPDWORD pcchGroups
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardListReaderGroupsA called\n");
    return (*Original_SCardListReaderGroupsA)(hContext,mszGroups,pcchGroups);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardListReaderGroupsW)(
    IN      SCARDCONTEXT hContext,
    OUT     LPWSTR mszGroups,
    IN OUT  LPDWORD pcchGroups
);

WINSCARDAPI LONG WINAPI SCardListReaderGroupsW(
    IN      SCARDCONTEXT hContext,
    OUT     LPWSTR mszGroups,
    IN OUT  LPDWORD pcchGroups
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardListReaderGroupsW called\n");
    return (*Original_SCardListReaderGroupsW)(hContext,mszGroups,pcchGroups);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardListCardsA)(
    IN      SCARDCONTEXT hContext,
    IN      LPCBYTE pbAtr,
    IN      LPCGUID rgquidInterfaces,
    IN      DWORD cguidInterfaceCount,
    OUT     LPSTR mszCards,
    IN OUT  LPDWORD pcchCards
);

WINSCARDAPI LONG WINAPI SCardListCardsA(
    IN      SCARDCONTEXT hContext,
    IN      LPCBYTE pbAtr,
    IN      LPCGUID rgquidInterfaces,
    IN      DWORD cguidInterfaceCount,
    OUT     LPSTR mszCards,
    IN OUT  LPDWORD pcchCards
){
    string message;
	message = string_format("SCardListCardsA(hContext:0x%x) called\n", hContext);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    return (*Original_SCardListCardsA)(hContext,pbAtr,rgquidInterfaces,cguidInterfaceCount,mszCards,pcchCards);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardListCardsW)(
    IN      SCARDCONTEXT hContext,
    IN      LPCBYTE pbAtr,
    IN      LPCGUID rgquidInterfaces,
    IN      DWORD cguidInterfaceCount,
    OUT     LPWSTR mszCards,
    IN OUT  LPDWORD pcchCards
);

WINSCARDAPI LONG WINAPI SCardListCardsW(
    IN      SCARDCONTEXT hContext,
    IN      LPCBYTE pbAtr,
    IN      LPCGUID rgquidInterfaces,
    IN      DWORD cguidInterfaceCount,
    OUT     LPWSTR mszCards,
    IN OUT  LPDWORD pcchCards
){
    string message;
	message = string_format("SCardListCardsW(hContext:0x%x) called\n", hContext);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    return (*Original_SCardListCardsW)(hContext,pbAtr,rgquidInterfaces,cguidInterfaceCount,mszCards,pcchCards);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardListInterfacesA)(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR szCard,
    OUT     LPGUID pguidInterfaces,
    IN OUT  LPDWORD pcguidInterfaces
);

WINSCARDAPI LONG WINAPI SCardListInterfacesA(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR szCard,
    OUT     LPGUID pguidInterfaces,
    IN OUT  LPDWORD pcguidInterfaces
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardListInterfacesA called\n");
    return (*Original_SCardListInterfacesA)(hContext,szCard,pguidInterfaces,pcguidInterfaces);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardListInterfacesW)(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR szCard,
    OUT     LPGUID pguidInterfaces,
    IN OUT  LPDWORD pcguidInterfaces
);

WINSCARDAPI LONG WINAPI SCardListInterfacesW(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR szCard,
    OUT     LPGUID pguidInterfaces,
    IN OUT  LPDWORD pcguidInterfaces
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardListInterfacesW called\n");
    return (*Original_SCardListInterfacesW)(hContext,szCard,pguidInterfaces,pcguidInterfaces);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardGetProviderIdA)(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR szCard,
    OUT     LPGUID pguidProviderId
);

WINSCARDAPI LONG WINAPI SCardGetProviderIdA(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR szCard,
    OUT     LPGUID pguidProviderId
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardGetProviderIdA called\n");
    return (*Original_SCardGetProviderIdA)(hContext,szCard,pguidProviderId);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardGetProviderIdW)(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR szCard,
    OUT     LPGUID pguidProviderId
);

WINSCARDAPI LONG WINAPI SCardGetProviderIdW(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR szCard,
    OUT     LPGUID pguidProviderId
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardGetProviderIdW called\n");
    return (*Original_SCardGetProviderIdW)(hContext,szCard,pguidProviderId);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardGetCardTypeProviderNameA)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName,
    IN DWORD dwProviderId,
    OUT LPSTR szProvider,
    IN OUT LPDWORD pcchProvider
);

WINSCARDAPI LONG WINAPI SCardGetCardTypeProviderNameA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName,
    IN DWORD dwProviderId,
    OUT LPSTR szProvider,
    IN OUT LPDWORD pcchProvider
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardGetCardTypeProviderNameA called\n");
    return (*Original_SCardGetCardTypeProviderNameA)(hContext,szCardName,dwProviderId,szProvider,pcchProvider);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardGetCardTypeProviderNameW)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN DWORD dwProviderId,
    OUT LPWSTR szProvider,
    IN OUT LPDWORD pcchProvider
);

WINSCARDAPI LONG WINAPI SCardGetCardTypeProviderNameW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN DWORD dwProviderId,
    OUT LPWSTR szProvider,
    IN OUT LPDWORD pcchProvider
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardGetCardTypeProviderNameW called\n");
    return (*Original_SCardGetCardTypeProviderNameW)(hContext,szCardName,dwProviderId,szProvider,pcchProvider);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardIntroduceReaderGroupA)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szGroupName
);

WINSCARDAPI LONG WINAPI SCardIntroduceReaderGroupA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szGroupName
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardIntroduceReaderGroupA called\n");
    return (*Original_SCardIntroduceReaderGroupA)(hContext,szGroupName);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardIntroduceReaderGroupW)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szGroupName
);

WINSCARDAPI LONG WINAPI SCardIntroduceReaderGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szGroupName
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardIntroduceReaderGroupW called\n");
    return (*Original_SCardIntroduceReaderGroupW)(hContext,szGroupName);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardForgetReaderGroupA)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szGroupName
);

WINSCARDAPI LONG WINAPI SCardForgetReaderGroupA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szGroupName
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardForgetReaderGroupA called\n");
    return (*Original_SCardForgetReaderGroupA)(hContext,szGroupName);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardForgetReaderGroupW)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szGroupName
);

WINSCARDAPI LONG WINAPI SCardForgetReaderGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szGroupName
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardForgetReaderGroupW called\n");
    return (*Original_SCardForgetReaderGroupW)(hContext,szGroupName);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardIntroduceReaderA)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szDeviceName
);

WINSCARDAPI LONG WINAPI SCardIntroduceReaderA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szDeviceName
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardIntroduceReaderA called\n");
    return (*Original_SCardIntroduceReaderA)(hContext,szReaderName,szDeviceName);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardIntroduceReaderW)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szDeviceName
);

WINSCARDAPI LONG WINAPI SCardIntroduceReaderW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szDeviceName
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardIntroduceReaderW called\n");
    return (*Original_SCardIntroduceReaderW)(hContext,szReaderName,szDeviceName);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardForgetReaderA)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName
);

WINSCARDAPI LONG WINAPI SCardForgetReaderA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardForgetReaderA called\n");
    return (*Original_SCardForgetReaderA)(hContext,szReaderName);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardForgetReaderW)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName
);

WINSCARDAPI LONG WINAPI SCardForgetReaderW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardForgetReaderW called\n");
    return (*Original_SCardForgetReaderW)(hContext,szReaderName);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardAddReaderToGroupA)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szGroupName
);

WINSCARDAPI LONG WINAPI SCardAddReaderToGroupA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szGroupName
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardAddReaderToGroupA called\n");
    return (*Original_SCardAddReaderToGroupA)(hContext,szReaderName,szGroupName);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardAddReaderToGroupW)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szGroupName
);

WINSCARDAPI LONG WINAPI SCardAddReaderToGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szGroupName
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardAddReaderToGroupW called\n");
    return (*Original_SCardAddReaderToGroupW)(hContext,szReaderName,szGroupName);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardRemoveReaderFromGroupA)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szGroupName
);

WINSCARDAPI LONG WINAPI SCardRemoveReaderFromGroupA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szGroupName
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardRemoveReaderFromGroupA called\n");
    return (*Original_SCardRemoveReaderFromGroupA)(hContext,szReaderName,szGroupName);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardRemoveReaderFromGroupW)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szGroupName
);

WINSCARDAPI LONG WINAPI SCardRemoveReaderFromGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szGroupName
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardRemoveReaderFromGroupW called\n");
    return (*Original_SCardRemoveReaderFromGroupW)(hContext,szReaderName,szGroupName);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardIntroduceCardTypeA)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName,
    IN LPCGUID pguidPrimaryProvider,
    IN LPCGUID rgguidInterfaces,
    IN DWORD dwInterfaceCount,
    IN LPCBYTE pbAtr,
    IN LPCBYTE pbAtrMask,
    IN DWORD cbAtrLen
);

WINSCARDAPI LONG WINAPI SCardIntroduceCardTypeA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName,
    IN LPCGUID pguidPrimaryProvider,
    IN LPCGUID rgguidInterfaces,
    IN DWORD dwInterfaceCount,
    IN LPCBYTE pbAtr,
    IN LPCBYTE pbAtrMask,
    IN DWORD cbAtrLen
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardIntroduceCardTypeA called\n");
    return (*Original_SCardIntroduceCardTypeA)(hContext,szCardName,pguidPrimaryProvider,rgguidInterfaces,dwInterfaceCount,pbAtr,pbAtrMask,cbAtrLen);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardIntroduceCardTypeW)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN LPCGUID pguidPrimaryProvider,
    IN LPCGUID rgguidInterfaces,
    IN DWORD dwInterfaceCount,
    IN LPCBYTE pbAtr,
    IN LPCBYTE pbAtrMask,
    IN DWORD cbAtrLen
);

WINSCARDAPI LONG WINAPI SCardIntroduceCardTypeW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN LPCGUID pguidPrimaryProvider,
    IN LPCGUID rgguidInterfaces,
    IN DWORD dwInterfaceCount,
    IN LPCBYTE pbAtr,
    IN LPCBYTE pbAtrMask,
    IN DWORD cbAtrLen
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardIntroduceCardTypeW called\n");
    return (*Original_SCardIntroduceCardTypeW)(hContext,szCardName,pguidPrimaryProvider,rgguidInterfaces,dwInterfaceCount,pbAtr,pbAtrMask,cbAtrLen);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardSetCardTypeProviderNameA)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName,
    IN DWORD dwProviderId,
    IN LPCSTR szProvider
);

WINSCARDAPI LONG WINAPI SCardSetCardTypeProviderNameA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName,
    IN DWORD dwProviderId,
    IN LPCSTR szProvider
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardSetCardTypeProviderNameA called\n");
    return (*Original_SCardSetCardTypeProviderNameA)(hContext,szCardName,dwProviderId,szProvider);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardSetCardTypeProviderNameW)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN DWORD dwProviderId,
    IN LPCWSTR szProvider
);

WINSCARDAPI LONG WINAPI SCardSetCardTypeProviderNameW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN DWORD dwProviderId,
    IN LPCWSTR szProvider
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardSetCardTypeProviderNameW called\n");
    return (*Original_SCardSetCardTypeProviderNameW)(hContext,szCardName,dwProviderId,szProvider);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardForgetCardTypeA)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName
);

WINSCARDAPI LONG WINAPI SCardForgetCardTypeA(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardForgetCardTypeA called\n");
    return (*Original_SCardForgetCardTypeA)(hContext,szCardName);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardForgetCardTypeW)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName
);

WINSCARDAPI LONG WINAPI SCardForgetCardTypeW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardForgetCardTypeW called\n");
    return (*Original_SCardForgetCardTypeW)(hContext,szCardName);
}

static WINSCARDAPI HANDLE (WINAPI *Original_SCardAccessStartedEvent)(void);

WINSCARDAPI HANDLE WINAPI SCardAccessStartedEvent(void){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardAccessStartedEvent called\n");
    return (*Original_SCardAccessStartedEvent)();
}


static WINSCARDAPI void (WINAPI *Original_SCardReleaseStartedEvent)(void);

WINSCARDAPI void WINAPI SCardReleaseStartedEvent(void){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardReleaseStartedEvent called\n");
    return (*Original_SCardReleaseStartedEvent)();
}


static WINSCARDAPI LONG (WINAPI *Original_SCardLocateCardsA)(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR mszCards,
    IN OUT  LPSCARD_READERSTATEA rgReaderStates,
    IN      DWORD cReaders
);

WINSCARDAPI LONG WINAPI SCardLocateCardsA(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR mszCards,
    IN OUT  LPSCARD_READERSTATEA rgReaderStates,
    IN      DWORD cReaders
){
    string message;
    message = string_format("SCardLocateCardsA(%s,0x%x) called\n", mszCards, hContext);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    return (*Original_SCardLocateCardsA)(hContext,mszCards,rgReaderStates,cReaders);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardLocateCardsW)(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR mszCards,
    IN OUT  LPSCARD_READERSTATEW rgReaderStates,
    IN      DWORD cReaders
);

WINSCARDAPI LONG WINAPI SCardLocateCardsW(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR mszCards,
    IN OUT  LPSCARD_READERSTATEW rgReaderStates,
    IN      DWORD cReaders
){
    string message;
    message = string_format("SCardLocateCardsW(%S,0x%x) called\n", mszCards, hContext);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    return (*Original_SCardLocateCardsW)(hContext,mszCards,rgReaderStates,cReaders);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardLocateCardsByATRA)(
    IN      SCARDCONTEXT hContext,
    IN      LPSCARD_ATRMASK rgAtrMasks,
    IN      DWORD cAtrs,
    IN OUT  LPSCARD_READERSTATEA rgReaderStates,
    IN      DWORD cReaders
);

WINSCARDAPI LONG WINAPI SCardLocateCardsByATRA(
    IN      SCARDCONTEXT hContext,
    IN      LPSCARD_ATRMASK rgAtrMasks,
    IN      DWORD cAtrs,
    IN OUT  LPSCARD_READERSTATEA rgReaderStates,
    IN      DWORD cReaders
){
    string message;
    message = string_format("SCardLocateCardsByATRA(hContext:0x%x) called\n", hContext);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    return (*Original_SCardLocateCardsByATRA)(hContext,rgAtrMasks,cAtrs,rgReaderStates,cReaders);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardLocateCardsByATRW)(
    IN      SCARDCONTEXT hContext,
    IN      LPSCARD_ATRMASK rgAtrMasks,
    IN      DWORD cAtrs,
    IN OUT  LPSCARD_READERSTATEW rgReaderStates,
    IN      DWORD cReaders
);

WINSCARDAPI LONG WINAPI SCardLocateCardsByATRW(
    IN      SCARDCONTEXT hContext,
    IN      LPSCARD_ATRMASK rgAtrMasks,
    IN      DWORD cAtrs,
    IN OUT  LPSCARD_READERSTATEW rgReaderStates,
    IN      DWORD cReaders
){
    string message;
    message = string_format("SCardLocateCardsByATRW(hContext:0x%x) called\n", hContext);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    return (*Original_SCardLocateCardsByATRW)(hContext,rgAtrMasks,cAtrs,rgReaderStates,cReaders);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardGetStatusChangeA)(
    IN      SCARDCONTEXT hContext,
    IN      DWORD dwTimeout,
    IN OUT  LPSCARD_READERSTATEA rgReaderStates,
    IN      DWORD cReaders
);

WINSCARDAPI LONG WINAPI SCardGetStatusChangeA(
    IN      SCARDCONTEXT hContext,
    IN      DWORD dwTimeout,
    IN OUT  LPSCARD_READERSTATEA rgReaderStates,
    IN      DWORD cReaders
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardGetStatusChangeA called\n");
    return (*Original_SCardGetStatusChangeA)(hContext,dwTimeout,rgReaderStates,cReaders);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardGetStatusChangeW)(
    IN      SCARDCONTEXT hContext,
    IN      DWORD dwTimeout,
    IN OUT  LPSCARD_READERSTATEW rgReaderStates,
    IN      DWORD cReaders
);

WINSCARDAPI LONG WINAPI SCardGetStatusChangeW(
    IN      SCARDCONTEXT hContext,
    IN      DWORD dwTimeout,
    IN OUT  LPSCARD_READERSTATEW rgReaderStates,
    IN      DWORD cReaders
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardGetStatusChangeW called\n");
    return (*Original_SCardGetStatusChangeW)(hContext,dwTimeout,rgReaderStates,cReaders);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardCancel)(
    IN      SCARDCONTEXT hContext
);

WINSCARDAPI LONG WINAPI SCardCancel(
    IN      SCARDCONTEXT hContext
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardCancel called\n");
    return (*Original_SCardCancel)(hContext);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardConnectA)(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR szReader,
    IN      DWORD dwShareMode,
    IN      DWORD dwPreferredProtocols,
    OUT     LPSCARDHANDLE phCard,
    OUT     LPDWORD pdwActiveProtocol
);

WINSCARDAPI LONG WINAPI SCardConnectA(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR szReader,
    IN      DWORD dwShareMode,
    IN      DWORD dwPreferredProtocols,
    OUT     LPSCARDHANDLE phCard,
    OUT     LPDWORD pdwActiveProtocol
){

    if (theApp.m_winscardConfig.bFORCE_CONNECT_SHARED_MODE) { 
        // we will always set mode to shared, if required
        dwShareMode = SCARD_SHARE_SHARED;
    }


    LONG status = (*Original_SCardConnectA)(hContext,szReader,dwShareMode,dwPreferredProtocols,phCard,pdwActiveProtocol);
    string message;
	message = string_format("SCardConnectA(hContext:0x%x,%s,hCard:0x%x) called\n", hContext,szReader,*phCard);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    return status;
}

static WINSCARDAPI LONG (WINAPI *Original_SCardReconnect)(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwShareMode,
    IN      DWORD dwPreferredProtocols,
    IN      DWORD dwInitialization,
    OUT     LPDWORD pdwActiveProtocol
);

WINSCARDAPI LONG WINAPI SCardReconnect(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwShareMode,
    IN      DWORD dwPreferredProtocols,
    IN      DWORD dwInitialization,
    OUT     LPDWORD pdwActiveProtocol
){
    string message;
    message = string_format("SCardReconnect(hCard:0x%x) called\n", hCard);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    return (*Original_SCardReconnect)(hCard,dwShareMode,dwPreferredProtocols,dwInitialization,pdwActiveProtocol);
}

static WINSCARDAPI LONG (WINAPI *Original_SCardBeginTransaction)(
    IN      SCARDHANDLE hCard
);

WINSCARDAPI LONG WINAPI SCardBeginTransaction(
    IN      SCARDHANDLE hCard
){
    CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardBeginTransaction called\n");
    return (*Original_SCardBeginTransaction)(hCard);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardEndTransaction)(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwDisposition
);

WINSCARDAPI LONG WINAPI SCardEndTransaction(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwDisposition
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardEndTransaction called\n");
    return (*Original_SCardEndTransaction)(hCard,dwDisposition);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardCancelTransaction)(
    IN      SCARDHANDLE hCard
);

WINSCARDAPI LONG WINAPI SCardCancelTransaction(
    IN      SCARDHANDLE hCard
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardCancelTransaction called\n");
    return (*Original_SCardCancelTransaction)(hCard);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardState)(
    IN SCARDHANDLE hCard,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen
);

WINSCARDAPI LONG WINAPI SCardState(
    IN SCARDHANDLE hCard,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen
){
    string message;
    message = string_format("SCardState(hCard:0x%x) called\n", hCard);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    return (*Original_SCardState)(hCard,pdwState,pdwProtocol,pbAtr,pcbAtrLen);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardStatusW)(
    IN SCARDHANDLE hCard,
    OUT LPWSTR szReaderName,
    IN OUT LPDWORD pcchReaderLen,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen
);

WINSCARDAPI LONG WINAPI SCardStatusW(
    IN SCARDHANDLE hCard,
    OUT LPWSTR szReaderName,
    IN OUT LPDWORD pcchReaderLen,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen
){
    string message;
    message = string_format("SCardStatusW(hCard:0x%x) called\n", hCard);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    return (*Original_SCardStatusW)(hCard,szReaderName,pcchReaderLen,pdwState,pdwProtocol,pbAtr,pcbAtrLen);
}

static WINSCARDAPI LONG (WINAPI *Original_SCardControl)(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwControlCode,
    IN      LPCVOID lpInBuffer,
    IN      DWORD nInBufferSize,
    OUT     LPVOID lpOutBuffer,
    IN      DWORD nOutBufferSize,
    OUT     LPDWORD lpBytesReturned
);

WINSCARDAPI LONG WINAPI SCardControl(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwControlCode,
    IN      LPCVOID lpInBuffer,
    IN      DWORD nInBufferSize,
    OUT     LPVOID lpOutBuffer,
    IN      DWORD nOutBufferSize,
    OUT     LPDWORD lpBytesReturned
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardControl called\n");
    return (*Original_SCardControl)(hCard,dwControlCode,lpInBuffer,nInBufferSize,lpOutBuffer,nOutBufferSize,lpBytesReturned);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardGetAttrib)(
    IN SCARDHANDLE hCard,
    IN DWORD dwAttrId,
    OUT LPBYTE pbAttr,
    IN OUT LPDWORD pcbAttrLen
);

WINSCARDAPI LONG WINAPI SCardGetAttrib(
    IN SCARDHANDLE hCard,
    IN DWORD dwAttrId,
    OUT LPBYTE pbAttr,
    IN OUT LPDWORD pcbAttrLen
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardGetAttrib called\n");
    return (*Original_SCardGetAttrib)(hCard,dwAttrId,pbAttr,pcbAttrLen);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardSetAttrib)(
    IN SCARDHANDLE hCard,
    IN DWORD dwAttrId,
    IN LPCBYTE pbAttr,
    IN DWORD cbAttrLen
);

WINSCARDAPI LONG WINAPI SCardSetAttrib(
    IN SCARDHANDLE hCard,
    IN DWORD dwAttrId,
    IN LPCBYTE pbAttr,
    IN DWORD cbAttrLen
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardSetAttrib called\n");
    return (*Original_SCardSetAttrib)(hCard,dwAttrId,pbAttr,cbAttrLen);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardUIDlgSelectCardA)(
    LPOPENCARDNAMEA_EX
);

WINSCARDAPI LONG WINAPI SCardUIDlgSelectCardA(
    LPOPENCARDNAMEA_EX a
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardUIDlgSelectCardA called\n");
    return (*Original_SCardUIDlgSelectCardA)(a);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardUIDlgSelectCardW)(
    LPOPENCARDNAMEW_EX
);

WINSCARDAPI LONG WINAPI SCardUIDlgSelectCardW(
    LPOPENCARDNAMEW_EX a
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardUIDlgSelectCardW called\n");
    return (*Original_SCardUIDlgSelectCardW)(a);
}

static WINSCARDAPI LONG (WINAPI *Original_GetOpenCardNameA)(
    LPOPENCARDNAMEA
);

WINSCARDAPI LONG WINAPI GetOpenCardNameA(
    LPOPENCARDNAMEA a
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "GetOpenCardNameA called\n");
    return (*Original_GetOpenCardNameA)(a);
}
    
static WINSCARDAPI LONG (WINAPI *Original_GetOpenCardNameW)(
    LPOPENCARDNAMEW
);

WINSCARDAPI LONG WINAPI GetOpenCardNameW(
    LPOPENCARDNAMEW a
){
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "GetOpenCardNameW called\n");
    return (*Original_GetOpenCardNameW)(a);
}


static WINSCARDAPI LONG (WINAPI *Original_SCardDlgExtendedError )(void);

WINSCARDAPI LONG WINAPI SCardDlgExtendedError (void){
    CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardDlgExtendedError called\n");
    return (*Original_SCardDlgExtendedError )();
}



/* ******************************************************************************* */


// CWinscardApp

BEGIN_MESSAGE_MAP(CWinscardApp, CWinApp)
END_MESSAGE_MAP()

// CWinscardApp construction

CWinscardApp::CWinscardApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_bRulesActive = FALSE;
    m_processedApduByteCounter = 0;
}

CWinscardApp::~CWinscardApp()
{
    FreeLibrary(hOriginal);

	// Reference to WINSCARD_LOG will fail with access to 0xfeefee (global CString WINSCARD_LOG does not exists at the time of dll release (strange))
//	if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) CCommonFnc::File_AppendString(WINSCARD_LOG, "[end]\r\n");
    
    if (m_scsat04Config.pSocket != NULL) delete m_scsat04Config.pSocket;

    lptr::iterator  iter;
    for (iter = m_charAllocatedMemoryList.begin(); iter != m_charAllocatedMemoryList.end(); iter++) {
        char* ptr = (char*) *iter;
        if (ptr != NULL) delete[] ptr;
    }
    m_charAllocatedMemoryList.clear();
    for (iter = m_wcharAllocatedMemoryList.begin(); iter != m_wcharAllocatedMemoryList.end(); iter++) {
        WCHAR* ptr = (WCHAR*) *iter;
        if (ptr != NULL) delete[] ptr;
    }
    m_wcharAllocatedMemoryList.clear();
}



static WINSCARDAPI LONG (WINAPI *Original_SCardConnectW)(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR szReader,
    IN      DWORD dwShareMode,
    IN      DWORD dwPreferredProtocols,
    OUT     LPSCARDHANDLE phCard,
    OUT     LPDWORD pdwActiveProtocol
);

WINSCARDAPI LONG WINAPI SCardConnectW(
    SCARDCONTEXT hContext,
    LPCWSTR szReader,
    DWORD dwShareMode,
    DWORD dwPreferredProtocols,
    LPSCARDHANDLE phCard,
    LPDWORD pdwActiveProtocol)
{
    LONG    status = SCARD_S_SUCCESS;
    string message;
    message = string_format("SCardConnectW(hContext:0x%x, %S) called\n", hContext, szReader);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);

    // RESET APDU IN BYTE COUNTER
    theApp.m_processedApduByteCounter = 0;

    // RESET CARD
    if (theApp.m_scsat04Config.bRedirect && (theApp.m_scsat04Config.pSocket != NULL)) {
        string message;
        theApp.m_scsat04Config.pSocket->SendLine("get reset 1000");
        std::string l = theApp.m_scsat04Config.pSocket->ReceiveResponse(SCSAT_SOCKET_ENDSEQ, SCSAT_SOCKET_TIMEOUT);
        message = string_format("\n:: %s", l.c_str());
        //message.Replace("\n", " ");
		message.erase(remove(message.begin(), message.end(), '\r'), message.end());
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
        
        // PREPARE FOR MEASUREMENT
        message = string_format("get params 1 %d %d", theApp.m_scsat04Config.measureApduByteCounter, theApp.m_scsat04Config.measureApduByteDelay);
        theApp.m_scsat04Config.pSocket->SendLine((LPCTSTR) message.c_str());
        l = theApp.m_scsat04Config.pSocket->ReceiveResponse(SCSAT_SOCKET_ENDSEQ, SCSAT_SOCKET_TIMEOUT);
        message = string_format(":: %s", l.c_str());
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
        
        message = string_format("post sampling %d", theApp.m_scsat04Config.numSamples);
        theApp.m_scsat04Config.pSocket->SendLine((LPCTSTR) message.c_str());
        l = theApp.m_scsat04Config.pSocket->ReceiveResponse(SCSAT_SOCKET_ENDSEQ, SCSAT_SOCKET_TIMEOUT);
        message = string_format(":: %s", l.c_str());
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
        
        // PREPARE FOR MEASUREMENT READING IN FUTURE
        theApp.m_scsat04Config.sampleReaded = FALSE;
        
        // CREATE VIRTUAL CARD HANDLE
        *phCard = HANDLE_VIRTUAL_CARD;
        *pdwActiveProtocol = SCARD_PROTOCOL_T0;
        
        status = SCARD_S_SUCCESS;
    }
    else {
        status = (*Original_SCardConnectW)(hContext,szReader,dwShareMode,dwPreferredProtocols,phCard,pdwActiveProtocol);
    }
    
    message = string_format("-> hCard:0x%x\n", *phCard);
    CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    
    return status;
}    

static WINSCARDAPI LONG (WINAPI *Original_SCardFreeMemory)(
    IN SCARDCONTEXT hContext,
    IN LPCVOID pvMem
);

WINSCARDAPI LONG WINAPI SCardFreeMemory(
    IN SCARDCONTEXT hContext,
    IN LPCVOID pvMem)
{
    string message;
    message = string_format("SCardFreeMemory(hContext:0x%x) called\n", hContext);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);

    LONG    status = SCARD_S_SUCCESS;
    
    // TRY TO FIND GIVEN MEMORY REFFERENCE IN LOCAL ALLOCATIONS
    BOOL            bFound = FALSE;
    lptr::iterator  iter;
    for (iter = theApp.m_charAllocatedMemoryList.begin(); iter != theApp.m_charAllocatedMemoryList.end(); iter++) {
        char* ptr = (char*) *iter;
        if (ptr != NULL && (ptr == pvMem)) {
            delete[] ptr;
            bFound = TRUE;
            
            theApp.m_charAllocatedMemoryList.erase(iter);
            break;
        }
    }
    for (iter = theApp.m_wcharAllocatedMemoryList.begin(); iter != theApp.m_wcharAllocatedMemoryList.end(); iter++) {
        WCHAR* ptr = (WCHAR*) *iter;
        if (ptr != NULL && (ptr == pvMem)) {
            delete[] ptr;
            bFound = TRUE;
            
            theApp.m_wcharAllocatedMemoryList.erase(iter);
            break;
        }
    }
    // IF NOT FOUND, PASS TO ORIGINAL LIBRARY
    if (!bFound) status = (*Original_SCardFreeMemory)(hContext,pvMem);
           
    return status;
}

static WINSCARDAPI LONG (WINAPI *Original_SCardListReadersW)( 
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR mszGroups,
    OUT     LPWSTR mszReaders,
    IN OUT  LPDWORD pcchReaders
);
    
WINSCARDAPI LONG WINAPI SCardListReadersW( 
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR mszGroups,
    OUT     LPWSTR mszReaders,
    IN OUT  LPDWORD pcchReaders) 
{
    string message;
    message = string_format("SCardListReadersW(hContext:0x%x) called\n", hContext);
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
   
    LONG    status = SCARD_S_SUCCESS;
    lcs     readersList;
    
    if (*pcchReaders == SCARD_AUTOALLOCATE) {
        // NO BUFFER IS SUPPLIED
        
        // OBTAIN REQUIRED LENGTH FOR REAL READERS
        if ((status = (*Original_SCardListReadersW)(hContext,mszGroups,NULL,pcchReaders)) == SCARD_S_SUCCESS) {
            // ALLOCATE OWN BUFFER FOR REAL AND VIRTUAL READERS
            int     newLen = *pcchReaders + VIRTUAL_READERS_LEN;
            WCHAR*   readers = new WCHAR[newLen];
            memset(readers, 0, newLen * sizeof(WCHAR));
            *pcchReaders = newLen;
            if ((status = (*Original_SCardListReadersW)(hContext,mszGroups,readers,pcchReaders)) == SCARD_S_SUCCESS) {
                // COPY NAME OF VIRTUAL READERS TO END
                for (DWORD i = 0; i < strlen(VIRT_READER_NAME) + 1; i++) {
                    readers[i + *pcchReaders] = VIRT_READER_NAME[i];
                }
                // ADD TRAILING ZERO
                *pcchReaders += (DWORD) strlen(VIRT_READER_NAME) + 1;
                readers[*pcchReaders - 1] = 0;
                // CAST mszReaders TO char** IS NECESSARY TO CORRECTLY PROPAGATE ALLOCATED BUFFER              
                WCHAR**  temp = (WCHAR**) mszReaders;
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
        if ((status = (*Original_SCardListReadersW)(hContext,mszGroups,NULL,&realLen)) == SCARD_S_SUCCESS) {
            if ((realLen + VIRTUAL_READERS_LEN > *pcchReaders) || (mszReaders == NULL)) {
                // SUPPLIED BUFFER IS NOT LARGE ENOUGHT
                *pcchReaders = realLen + VIRTUAL_READERS_LEN;  
                if (mszReaders != NULL) status = SCARD_E_INSUFFICIENT_BUFFER;
            } 
            else {
                // SUPPLIED BUFFER IS OK, COPY REAL AND VIRTUAL READERS
                realLen = *pcchReaders;
				memset(mszReaders, 0, *pcchReaders * sizeof(WCHAR));
                if ((status = (*Original_SCardListReadersW)(hContext,mszGroups,mszReaders,&realLen)) == SCARD_S_SUCCESS) {
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
        if (theApp.m_winscardConfig.sREADER_ORDERED_FIRST != _T("")) {
            // REODERING OF READERS WILL BE PERFORMED
            
            // TRY TO FIND POSITION OF PREFFERED READER IN BUFFER
            for (DWORD i = 0; i < *pcchReaders - theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length(); i++) {
                if (memcmp((LPCTSTR) theApp.m_winscardConfig.sREADER_ORDERED_FIRST.c_str(), mszReaders + i, theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length() * sizeof(WCHAR)) == 0) {
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
    string         availableReaders = _T("-> Found readers: ");
    for (iter = readersList.begin(); iter != readersList.end(); iter++) {
        availableReaders += *iter;
        availableReaders += _T(", ");
    }
    availableReaders += _T("\n");
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, availableReaders);
    
    return status;
}

static WINSCARDAPI LONG (WINAPI *Original_SCardListReadersA)( 
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR mszGroups,
    OUT     LPSTR mszReaders,
    IN OUT  LPDWORD pcchReaders
);
    
WINSCARDAPI LONG WINAPI SCardListReadersA( 
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR mszGroups,
    OUT     LPSTR mszReaders,
    IN OUT  LPDWORD pcchReaders)
{
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardListReadersA called\n");
   
    int  status = SCARD_S_SUCCESS;
    lcs     readersList;
    
    if (*pcchReaders == SCARD_AUTOALLOCATE) {
        // NO BUFFER IS SUPPLIED
        
        // OBTAIN REQUIRED LENGTH FOR REAL READERS
        if ((status = (*Original_SCardListReadersA)(hContext,mszGroups,NULL,pcchReaders)) == SCARD_S_SUCCESS) {
            // ALLOCATE OWN BUFFER FOR REAL AND VIRTUAL READERS
            int     newLen = *pcchReaders + VIRTUAL_READERS_LEN + 2;
            char*   readers = new char[newLen];
            memset(readers, 0, newLen);
            *pcchReaders = newLen;
            if ((status = (*Original_SCardListReadersA)(hContext,mszGroups,readers,pcchReaders)) == SCARD_S_SUCCESS) {
                // COPY NAME OF VIRTUAL READERS TO END
                memcpy(readers + *pcchReaders, VIRT_READER_NAME, strlen(VIRT_READER_NAME));
                // ADD TRAILING ZERO
                *pcchReaders += (DWORD) strlen(VIRT_READER_NAME) + 1;
                readers[*pcchReaders - 1] = 0;
                // CAST mszReaders TO char** IS NECESSARY TO CORRECTLY PROPAGATE ALLOCATED BUFFER              
                char**  temp = (char**) mszReaders;
                *temp = readers;
                CCommonFnc::String_ParseNullSeparatedArray((BYTE*) readers, *pcchReaders - 1, &readersList);
                // ADD ALLOCATED MEMORY TO LIST FOR FUTURE DEALLOCATION
                theApp.m_charAllocatedMemoryList.push_back(readers);
            }
        }
    }
    else {
        // BUFFER SUPPLIED
        // OBTAIN REQUIRED LENGTH FOR REAL READERS
        DWORD     realLen = *pcchReaders;
        if ((status = (*Original_SCardListReadersA)(hContext,mszGroups,NULL,&realLen)) == SCARD_S_SUCCESS) {
            if ((realLen + VIRTUAL_READERS_LEN > *pcchReaders) || (mszReaders == NULL)) {
                // SUPPLIED BUFFER IS NOT LARGE ENOUGHT
                *pcchReaders = realLen + VIRTUAL_READERS_LEN;  
                if (mszReaders != NULL) status = SCARD_E_INSUFFICIENT_BUFFER;
            } 
            else {
                // SUPPLIED BUFFER IS OK, COPY REAL AND VIRTUAL READERS
                realLen = *pcchReaders;
                if ((status = (*Original_SCardListReadersA)(hContext,mszGroups,mszReaders,&realLen)) == SCARD_S_SUCCESS) {
                    *pcchReaders = realLen;
                    
// ADD VIRTUAL READER
                    // COPY NAME OF VIRTUAL READERS TO END
                    memcpy(mszReaders + realLen, VIRT_READER_NAME, strlen(VIRT_READER_NAME));
                    *pcchReaders = realLen + strlen(VIRT_READER_NAME) + 1;
                    // ADD TRAILING ZERO
                    mszReaders[*pcchReaders - 1] = 0;
/**/                    
                    CCommonFnc::String_ParseNullSeparatedArray((BYTE*) mszReaders, *pcchReaders - 1, &readersList);
                }                
            }   
        }
    }

    if (status == STAT_OK && mszReaders != NULL) {
        if (theApp.m_winscardConfig.sREADER_ORDERED_FIRST != _T("")) {
            // REODERING OF READERS WILL BE PERFORMED
            
            // TRY TO FIND POSITION OF PREFFERED READER IN BUFFER
            for (DWORD i = 0; i < *pcchReaders - theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length(); i++) {
                if (memcmp((LPCTSTR) theApp.m_winscardConfig.sREADER_ORDERED_FIRST.c_str(), mszReaders + i, theApp.m_winscardConfig.sREADER_ORDERED_FIRST.length()) == 0) {
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
    string         availableReaders = _T("-> Found readers: ");
    for (iter = readersList.begin(); iter != readersList.end(); iter++) {
        availableReaders += *iter;
        availableReaders += _T(", ");
    }
    availableReaders += _T("\n");
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, availableReaders);

    return status;
}
    
static WINSCARDAPI LONG (WINAPI *Original_SCardDisconnect)(
    SCARDHANDLE hCard,
    DWORD dwDisposition
);  

WINSCARDAPI LONG WINAPI SCardDisconnect(
    SCARDHANDLE hCard,
    DWORD dwDisposition)
{
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardDisconnect called\n");

    // DISCONNECT FROM CARD
    if (hCard == HANDLE_VIRTUAL_CARD) { 
        // DO NOTHING
        return SCARD_S_SUCCESS;
    } else {
        return (*Original_SCardDisconnect)(hCard,dwDisposition);
    }
}   
static WINSCARDAPI LONG (WINAPI *Original_SCardStatusA)(
    SCARDHANDLE hCard, 
    LPTSTR szReaderName, 
    LPDWORD pcchReaderLen, 
    LPDWORD pdwState, 
    LPDWORD pdwProtocol, 
    LPBYTE pbAtr, 
    LPDWORD pcbAtrLen
);

WINSCARDAPI LONG WINAPI SCardStatusA(
    SCARDHANDLE hCard, 
    LPTSTR szReaderName, 
    LPDWORD pcchReaderLen, 
    LPDWORD pdwState, 
    LPDWORD pdwProtocol, 
    LPBYTE pbAtr, 
    LPDWORD pcbAtrLen)
{
    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardStatusA called\n");
    return (*Original_SCardStatusA)(hCard,szReaderName,pcchReaderLen,pdwState,pdwProtocol,pbAtr,pcbAtrLen);
}    

static WINSCARDAPI LONG (WINAPI *Original_SCardTransmit)(
  IN SCARDHANDLE hCard,  
  IN LPCSCARD_IO_REQUEST pioSendPci,
  IN LPCBYTE pbSendBuffer,
  IN DWORD cbSendLength,
  IN OUT LPSCARD_IO_REQUEST pioRecvPci,
  OUT LPBYTE pbRecvBuffer,
  IN OUT LPDWORD pcbRecvLength
);

WINSCARDAPI LONG WINAPI SCardTransmit(
  IN SCARDHANDLE hCard,  
  IN LPCSCARD_IO_REQUEST pioSendPci,
  IN LPCBYTE pbSendBuffer,
  IN DWORD cbSendLength,
  IN OUT LPSCARD_IO_REQUEST pioRecvPci,
  OUT LPBYTE pbRecvBuffer,
  IN OUT LPDWORD pcbRecvLength
) {

    if (theApp.m_winscardConfig.bLOG_FUNCTIONS_CALLS) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "SCardTransmit called\n");

    LONG result = SCARD_S_SUCCESS; 
//    DWORD written;
    char *txMsg   = "transmitted:";
    char *rxMsg   = "received:";
    char *crlf    = "\r\n";
    const int bufferLength = 1024;
    char buffer[bufferLength];
    char  sendBuffer[300];  
    clock_t elapsedCard;
    clock_t elapsedLibrary;
    string     message;

	elapsedLibrary = -clock();
    if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) {
        sprintf(buffer,"SCardTransmit (handle 0x%0.8X)#\r\n",hCard);
	    CCommonFnc::File_AppendString(WINSCARD_LOG, buffer);

        sprintf(buffer,"apduCounter:%d#\r\n",apduCounter);
	    CCommonFnc::File_AppendString(WINSCARD_LOG, buffer);

        sprintf(buffer,"totalBytesINCounter:%d#\r\n",theApp.m_processedApduByteCounter + 1);
	    CCommonFnc::File_AppendString(WINSCARD_LOG, buffer);

	    CCommonFnc::File_AppendString(WINSCARD_LOG, txMsg);

        DumpMemory( pbSendBuffer, cbSendLength );    
    }
    

    // SAVE INCOMING APDU
    APDU_BUFFER     apduBuff;
    memset(&apduBuff, 0, sizeof(APDU_BUFFER));
    memcpy(&apduBuff, pbSendBuffer, cbSendLength);
    theApp.apduInList.push_front(apduBuff);

    if (theApp.m_winscardConfig.bMODIFY_APDU_BY_RULES) {
        message = string_format("\nIncoming rules applied for apduCounter %d: \n", apduCounter);
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
        CCommonFnc::BYTE_ConvertFromArrayToHexString((BYTE*) pbSendBuffer, cbSendLength, &message);
        //message.Insert(0, "   "); message += "\n";
		message.insert(0, "   "); message += "\n";
        if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    }

    // COPY INPUT DATA
    memcpy(sendBuffer, pbSendBuffer, cbSendLength);

    // APPLY INCOMING RULES
    if (theApp.m_winscardConfig.bMODIFY_APDU_BY_RULES) {
        if (theApp.m_winscardConfig.bMODIFY_APDU_BY_RULES) theApp.ApplyRules((BYTE*) sendBuffer, &cbSendLength, INPUT_APDU);
        CCommonFnc::BYTE_ConvertFromArrayToHexString((BYTE*) sendBuffer, cbSendLength, &message);
        //message.Insert(0, "   "); message += "\n";
		message.insert(0, "   "); message += "\n";
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
            result = theApp.SCSAT_SCardTransmit(&(theApp.m_scsat04Config), (SCARD_IO_REQUEST *) pioSendPci,(LPCBYTE)sendBuffer,cbSendLength,pioRecvPci,pbRecvBuffer,pcbRecvLength);
            
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
        result = (*Original_SCardTransmit)(hCard,pioSendPci,(LPCBYTE)sendBuffer,cbSendLength,pioRecvPci,pbRecvBuffer,pcbRecvLength);
    }
    
    
    // HACK - if required, then perform transparently data readout on behalf of reader
    // RECEIVE RESPONSE DATA, IF ANY 
    if ((*pcbRecvLength == 2)&& theApp.m_winscardConfig.bAUTO_REQUEST_DATA) {
        // READOUT ALL DATA
        DWORD   recvOffset = 0;
        while (((pbRecvBuffer[recvOffset]) == 0x61 ) || ((pbRecvBuffer[recvOffset]) == 0x6C)) { // 0x61 ... SW_BYTES_REMAINING_00, 0x6C ... SW_CORRECT_LENGTH_00
            // GET DATA APDU
            sendBuffer[0] = (BYTE) 0x00;
            //sendBuffer[0] = (BYTE) 0xC0;
            //sendBuffer[0] = (BYTE) 0xA0;
            
            sendBuffer[1] = (BYTE) 0xC0;
            sendBuffer[2] = (BYTE) 0x00;
            sendBuffer[3] = (BYTE) 0x00;
            
            // HACK TO DEAL WITH CARDS THAT CANNOT HANDLE 254B AND MORE APDUS - if 0x61 0x00 (SW_BYTES_REMAINING_00 with zero remaining bytes is detected, then ask for 254 bytes instead
            if ((pbRecvBuffer[*pcbRecvLength - 1] & 0xff) == 0)  sendBuffer[4] = (BYTE) 254;
            else sendBuffer[4] = (BYTE) pbRecvBuffer[*pcbRecvLength - 1];

            cbSendLength = 5;
            
            int tmp = sendBuffer[4] & 0xff; tmp += 2; *pcbRecvLength = tmp;
            //*pcbRecvLength = sendBuffer[4] & 0xff + 2;
            result = (*Original_SCardTransmit)(hCard,pioSendPci,(LPCBYTE)sendBuffer,cbSendLength,pioRecvPci,pbRecvBuffer + recvOffset,pcbRecvLength);
            recvOffset = *pcbRecvLength - 2;
        }
    }
    
        
    // SAVE TIME OF CARD RESPONSE
    elapsedCard += clock();
    if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) {    
        sprintf(buffer,"responseTime:%d#\r\n",elapsedCard);
	    CCommonFnc::File_AppendString(WINSCARD_LOG, buffer);

        sprintf(buffer,"SCardTransmit result:0x%x#\r\n",result);
	    CCommonFnc::File_AppendString(WINSCARD_LOG, buffer);
    }
    
    if (result != SCARD_S_SUCCESS) {
        // CHANGE LENGTH OF RESPONSE TO PREVENT PARSING OF INCORRECT DATA
        *pcbRecvLength = 0;
    }

    if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) {
        CCommonFnc::File_AppendString(WINSCARD_LOG, rxMsg);
        DumpMemory( pbRecvBuffer, *pcbRecvLength );
        CCommonFnc::File_AppendString(WINSCARD_LOG, crlf);
    }

    // SAVE OUTGOING APDU
    memset(&apduBuff, 0, sizeof(APDU_BUFFER));
    memcpy(&apduBuff, pbRecvBuffer, *pcbRecvLength);
    theApp.apduOutList.push_front(apduBuff);

    if (theApp.m_winscardConfig.bMODIFY_APDU_BY_RULES) {
        message = string_format("\nOutgoing rules applied for apduCounter %d: \n", apduCounter);
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
        CCommonFnc::BYTE_ConvertFromArrayToHexString(pbRecvBuffer, *pcbRecvLength, &message);
        //message.Insert(0, "   "); message += "\n";
		message.insert(0, "   "); message += "\n";
        if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
        // APPLY OUTGOING RULES
        if (theApp.m_winscardConfig.bMODIFY_APDU_BY_RULES) theApp.ApplyRules(pbRecvBuffer, pcbRecvLength, OUTPUT_APDU);
        CCommonFnc::BYTE_ConvertFromArrayToHexString(pbRecvBuffer, *pcbRecvLength, &message);
        //message.Insert(0, "   "); message += "\n";
		message.insert(0, "   "); message += "\n";
        if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    }      
    
    
    
    // SCSAT04 - READ MEASUREMENT SAMPLE FROM BOARD IF NOT READED YET AND TRIGGER APPDU WAS REACHED (NUMBER OF BYTES IN)
    if (theApp.m_scsat04Config.bRedirect && !(theApp.m_scsat04Config.sampleReaded) && (theApp.m_processedApduByteCounter >= theApp.m_scsat04Config.measureApduByteCounter)) {
        // DOWNLOAD DATA FROM MEASUREMENT (IF ANY) 
        if (theApp.m_scsat04Config.pSocket != NULL) {
            string message;
            message = string_format("get powertrace 0 %d", theApp.m_scsat04Config.readRatio);
            theApp.m_scsat04Config.pSocket->SendLine((LPCTSTR) message.c_str());
            theApp.m_scsat04Config.baseReadOffset = 0;

            string sampleFilePath;
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
        message = string_format("responseTimeLibrary:%d#\r\n",elapsedLibrary);
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "...............................................\r\n");
    }

    return result;

}


// CWinscardApp initialization

BOOL CWinscardApp::InitInstance()
{
	CWinApp::InitInstance();

    srand((int) time(NULL));
    
    // LOAD MODIFICATION RULES
    LoadRules();

    // CONNECT TO SCSAT04 IF REQUIRED
    if (m_scsat04Config.bRedirect) {
        ConnectSCSAT04(&m_scsat04Config);
    }
/*
	DWORD written;
    hOut = CreateFile("winscard.txt",GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if ( ! hOut ) {
        MessageBox(NULL,"could not create output file","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }
/**/
    //WriteFile( hOut, "[begin]\r\n", (DWORD) strlen("[begin]\r\n"), &written, NULL );
	if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) CCommonFnc::File_AppendString(WINSCARD_LOG, "[begin]\r\n");

    hOriginal = LoadLibrary("original.dll");
    if ( ! hOriginal ) {
        MessageBox(NULL,"could not load original library","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }
    
    Original_SCardTransmit = 
        (long (__stdcall *)(unsigned long,const struct _SCARD_IO_REQUEST *,const unsigned char *,unsigned long,struct _SCARD_IO_REQUEST *,unsigned char *,unsigned long *))
            GetProcAddress(hOriginal,"SCardTransmit");
    if ( (!Original_SCardTransmit) ) {
        MessageBox(NULL,"could not find SCardTransmit procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardStatusA = 
        (long (__stdcall *)(SCARDHANDLE hCard, LPTSTR szReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen))
            GetProcAddress(hOriginal,"SCardStatusA");
    if ( (!Original_SCardStatusA) ) {
        MessageBox(NULL,"could not find SCardStatusA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }
    
    Original_SCardConnectW = 
        (long (__stdcall *)(SCARDCONTEXT hContext,LPCWSTR szReader,DWORD dwShareMode,DWORD dwPreferredProtocols,LPSCARDHANDLE phCard,LPDWORD pdwActiveProtocol))
            GetProcAddress(hOriginal,"SCardConnectW");
    if ( (!Original_SCardConnectW) ) {
        MessageBox(NULL,"could not find SCardConnectW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }
    
    Original_SCardDisconnect = 
        (long (__stdcall *)(SCARDHANDLE hCard, DWORD dwDisposition))
            GetProcAddress(hOriginal,"SCardDisconnect");
    if ( (!Original_SCardDisconnect) ) {
        MessageBox(NULL,"could not find SCardDisconnect procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardFreeMemory = 
        (long (__stdcall *)(SCARDCONTEXT hContext, LPCVOID pvMem))
            GetProcAddress(hOriginal,"SCardFreeMemory");
    if ( (!Original_SCardFreeMemory) ) {
        MessageBox(NULL,"could not find SCardFreeMemory procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }
    
    Original_SCardListReadersW = 
        (long (__stdcall *)(SCARDCONTEXT hContext, LPCWSTR mszGroups, LPWSTR mszReaders, LPDWORD pcchReaders))
            GetProcAddress(hOriginal,"SCardListReadersW");
    if ( (!Original_SCardListReadersW) ) {
        MessageBox(NULL,"could not find SCardListReadersW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }
    
    Original_SCardListReadersA = 
        (long (__stdcall *)(SCARDCONTEXT hContext, LPCSTR mszGroups, LPSTR mszReaders, LPDWORD pcchReaders))
            GetProcAddress(hOriginal,"SCardListReadersA");
    if ( (!Original_SCardListReadersA) ) {
        MessageBox(NULL,"could not find SCardListReadersA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }


    Original_SCardEstablishContext = 
        (LONG (__stdcall *)(
    IN  DWORD dwScope,
    IN  LPCVOID pvReserved1,
    IN  LPCVOID pvReserved2,
    OUT LPSCARDCONTEXT phContext))
            GetProcAddress(hOriginal,"SCardEstablishContext");
    if ( (!Original_SCardEstablishContext) ) {
        MessageBox(NULL,"Could not find SCardEstablishContext procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardReleaseContext = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext))
            GetProcAddress(hOriginal,"SCardReleaseContext");
    if ( (!Original_SCardReleaseContext) ) {
        MessageBox(NULL,"Could not find SCardReleaseContext procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardIsValidContext = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext))
            GetProcAddress(hOriginal,"SCardIsValidContext");
    if ( (!Original_SCardIsValidContext) ) {
        MessageBox(NULL,"Could not find SCardIsValidContext procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardListReaderGroupsA = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    OUT     LPSTR mszGroups,
    IN OUT  LPDWORD pcchGroups))
            GetProcAddress(hOriginal,"SCardListReaderGroupsA");
    if ( (!Original_SCardListReaderGroupsA) ) {
        MessageBox(NULL,"Could not find SCardListReaderGroupsA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardListReaderGroupsW = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    OUT     LPWSTR mszGroups,
    IN OUT  LPDWORD pcchGroups))
            GetProcAddress(hOriginal,"SCardListReaderGroupsW");
    if ( (!Original_SCardListReaderGroupsW) ) {
        MessageBox(NULL,"Could not find SCardListReaderGroupsW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardListReadersA = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR mszGroups,
    OUT     LPSTR mszReaders,
    IN OUT  LPDWORD pcchReaders))
            GetProcAddress(hOriginal,"SCardListReadersA");
    if ( (!Original_SCardListReadersA) ) {
        MessageBox(NULL,"Could not find SCardListReadersA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardListReadersW = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR mszGroups,
    OUT     LPWSTR mszReaders,
    IN OUT  LPDWORD pcchReaders))
            GetProcAddress(hOriginal,"SCardListReadersW");
    if ( (!Original_SCardListReadersW) ) {
        MessageBox(NULL,"Could not find SCardListReadersW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardListCardsA = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPCBYTE pbAtr,
    IN      LPCGUID rgquidInterfaces,
    IN      DWORD cguidInterfaceCount,
    OUT     LPSTR mszCards,
    IN OUT  LPDWORD pcchCards))
            GetProcAddress(hOriginal,"SCardListCardsA");
    if ( (!Original_SCardListCardsA) ) {
        MessageBox(NULL,"Could not find SCardListCardsA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardListCardsW = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPCBYTE pbAtr,
    IN      LPCGUID rgquidInterfaces,
    IN      DWORD cguidInterfaceCount,
    OUT     LPWSTR mszCards,
    IN OUT  LPDWORD pcchCards))
            GetProcAddress(hOriginal,"SCardListCardsW");
    if ( (!Original_SCardListCardsW) ) {
        MessageBox(NULL,"Could not find SCardListCardsW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardListInterfacesA = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR szCard,
    OUT     LPGUID pguidInterfaces,
    IN OUT  LPDWORD pcguidInterfaces))
            GetProcAddress(hOriginal,"SCardListInterfacesA");
    if ( (!Original_SCardListInterfacesA) ) {
        MessageBox(NULL,"Could not find SCardListInterfacesA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardListInterfacesW = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR szCard,
    OUT     LPGUID pguidInterfaces,
    IN OUT  LPDWORD pcguidInterfaces))
            GetProcAddress(hOriginal,"SCardListInterfacesW");
    if ( (!Original_SCardListInterfacesW) ) {
        MessageBox(NULL,"Could not find SCardListInterfacesW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardGetProviderIdA = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR szCard,
    OUT     LPGUID pguidProviderId))
            GetProcAddress(hOriginal,"SCardGetProviderIdA");
    if ( (!Original_SCardGetProviderIdA) ) {
        MessageBox(NULL,"Could not find SCardGetProviderIdA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardGetProviderIdW = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR szCard,
    OUT     LPGUID pguidProviderId))
            GetProcAddress(hOriginal,"SCardGetProviderIdW");
    if ( (!Original_SCardGetProviderIdW) ) {
        MessageBox(NULL,"Could not find SCardGetProviderIdW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardGetCardTypeProviderNameA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName,
    IN DWORD dwProviderId,
    OUT LPSTR szProvider,
    IN OUT LPDWORD pcchProvider))
            GetProcAddress(hOriginal,"SCardGetCardTypeProviderNameA");
    if ( (!Original_SCardGetCardTypeProviderNameA) ) {
        MessageBox(NULL,"Could not find SCardGetCardTypeProviderNameA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardGetCardTypeProviderNameW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN DWORD dwProviderId,
    OUT LPWSTR szProvider,
    IN OUT LPDWORD pcchProvider))
            GetProcAddress(hOriginal,"SCardGetCardTypeProviderNameW");
    if ( (!Original_SCardGetCardTypeProviderNameW) ) {
        MessageBox(NULL,"Could not find SCardGetCardTypeProviderNameW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardIntroduceReaderGroupA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szGroupName))
            GetProcAddress(hOriginal,"SCardIntroduceReaderGroupA");
    if ( (!Original_SCardIntroduceReaderGroupA) ) {
        MessageBox(NULL,"Could not find SCardIntroduceReaderGroupA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardIntroduceReaderGroupW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szGroupName))
            GetProcAddress(hOriginal,"SCardIntroduceReaderGroupW");
    if ( (!Original_SCardIntroduceReaderGroupW) ) {
        MessageBox(NULL,"Could not find SCardIntroduceReaderGroupW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardForgetReaderGroupA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szGroupName))
            GetProcAddress(hOriginal,"SCardForgetReaderGroupA");
    if ( (!Original_SCardForgetReaderGroupA) ) {
        MessageBox(NULL,"Could not find SCardForgetReaderGroupA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardForgetReaderGroupW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szGroupName))
            GetProcAddress(hOriginal,"SCardForgetReaderGroupW");
    if ( (!Original_SCardForgetReaderGroupW) ) {
        MessageBox(NULL,"Could not find SCardForgetReaderGroupW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardIntroduceReaderA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szDeviceName))
            GetProcAddress(hOriginal,"SCardIntroduceReaderA");
    if ( (!Original_SCardIntroduceReaderA) ) {
        MessageBox(NULL,"Could not find SCardIntroduceReaderA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardIntroduceReaderW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szDeviceName))
            GetProcAddress(hOriginal,"SCardIntroduceReaderW");
    if ( (!Original_SCardIntroduceReaderW) ) {
        MessageBox(NULL,"Could not find SCardIntroduceReaderW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardForgetReaderA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName))
            GetProcAddress(hOriginal,"SCardForgetReaderA");
    if ( (!Original_SCardForgetReaderA) ) {
        MessageBox(NULL,"Could not find SCardForgetReaderA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardForgetReaderW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName))
            GetProcAddress(hOriginal,"SCardForgetReaderW");
    if ( (!Original_SCardForgetReaderW) ) {
        MessageBox(NULL,"Could not find SCardForgetReaderW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardAddReaderToGroupA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szGroupName))
            GetProcAddress(hOriginal,"SCardAddReaderToGroupA");
    if ( (!Original_SCardAddReaderToGroupA) ) {
        MessageBox(NULL,"Could not find SCardAddReaderToGroupA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardAddReaderToGroupW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szGroupName))
            GetProcAddress(hOriginal,"SCardAddReaderToGroupW");
    if ( (!Original_SCardAddReaderToGroupW) ) {
        MessageBox(NULL,"Could not find SCardAddReaderToGroupW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardRemoveReaderFromGroupA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szGroupName))
            GetProcAddress(hOriginal,"SCardRemoveReaderFromGroupA");
    if ( (!Original_SCardRemoveReaderFromGroupA) ) {
        MessageBox(NULL,"Could not find SCardRemoveReaderFromGroupA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardRemoveReaderFromGroupW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szGroupName))
            GetProcAddress(hOriginal,"SCardRemoveReaderFromGroupW");
    if ( (!Original_SCardRemoveReaderFromGroupW) ) {
        MessageBox(NULL,"Could not find SCardRemoveReaderFromGroupW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardIntroduceCardTypeA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName,
    IN LPCGUID pguidPrimaryProvider,
    IN LPCGUID rgguidInterfaces,
    IN DWORD dwInterfaceCount,
    IN LPCBYTE pbAtr,
    IN LPCBYTE pbAtrMask,
    IN DWORD cbAtrLen))
            GetProcAddress(hOriginal,"SCardIntroduceCardTypeA");
    if ( (!Original_SCardIntroduceCardTypeA) ) {
        MessageBox(NULL,"Could not find SCardIntroduceCardTypeA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardIntroduceCardTypeW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN LPCGUID pguidPrimaryProvider,
    IN LPCGUID rgguidInterfaces,
    IN DWORD dwInterfaceCount,
    IN LPCBYTE pbAtr,
    IN LPCBYTE pbAtrMask,
    IN DWORD cbAtrLen))
            GetProcAddress(hOriginal,"SCardIntroduceCardTypeW");
    if ( (!Original_SCardIntroduceCardTypeW) ) {
        MessageBox(NULL,"Could not find SCardIntroduceCardTypeW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardSetCardTypeProviderNameA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName,
    IN DWORD dwProviderId,
    IN LPCSTR szProvider))
            GetProcAddress(hOriginal,"SCardSetCardTypeProviderNameA");
    if ( (!Original_SCardSetCardTypeProviderNameA) ) {
        MessageBox(NULL,"Could not find SCardSetCardTypeProviderNameA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardSetCardTypeProviderNameW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN DWORD dwProviderId,
    IN LPCWSTR szProvider))
            GetProcAddress(hOriginal,"SCardSetCardTypeProviderNameW");
    if ( (!Original_SCardSetCardTypeProviderNameW) ) {
        MessageBox(NULL,"Could not find SCardSetCardTypeProviderNameW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardForgetCardTypeA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName))
            GetProcAddress(hOriginal,"SCardForgetCardTypeA");
    if ( (!Original_SCardForgetCardTypeA) ) {
        MessageBox(NULL,"Could not find SCardForgetCardTypeA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardForgetCardTypeW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName))
            GetProcAddress(hOriginal,"SCardForgetCardTypeW");
    if ( (!Original_SCardForgetCardTypeW) ) {
        MessageBox(NULL,"Could not find SCardForgetCardTypeW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardFreeMemory = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCVOID pvMem))
            GetProcAddress(hOriginal,"SCardFreeMemory");
    if ( (!Original_SCardFreeMemory) ) {
        MessageBox(NULL,"Could not find SCardFreeMemory procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardAccessStartedEvent = 
        (HANDLE (__stdcall *)(void))
            GetProcAddress(hOriginal,"SCardAccessStartedEvent");
    if ( (!Original_SCardAccessStartedEvent) ) {
        MessageBox(NULL,"Could not find SCardAccessStartedEvent procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardReleaseStartedEvent = 
        (void (__stdcall *)(void))
            GetProcAddress(hOriginal,"SCardReleaseStartedEvent");
    if ( (!Original_SCardReleaseStartedEvent) ) {
        MessageBox(NULL,"Could not find SCardReleaseStartedEvent procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardLocateCardsA = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR mszCards,
    IN OUT  LPSCARD_READERSTATEA rgReaderStates,
    IN      DWORD cReaders))
            GetProcAddress(hOriginal,"SCardLocateCardsA");
    if ( (!Original_SCardLocateCardsA) ) {
        MessageBox(NULL,"Could not find SCardLocateCardsA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardLocateCardsW = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR mszCards,
    IN OUT  LPSCARD_READERSTATEW rgReaderStates,
    IN      DWORD cReaders))
            GetProcAddress(hOriginal,"SCardLocateCardsW");
    if ( (!Original_SCardLocateCardsW) ) {
        MessageBox(NULL,"Could not find SCardLocateCardsW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardLocateCardsByATRA = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPSCARD_ATRMASK rgAtrMasks,
    IN      DWORD cAtrs,
    IN OUT  LPSCARD_READERSTATEA rgReaderStates,
    IN      DWORD cReaders))
            GetProcAddress(hOriginal,"SCardLocateCardsByATRA");
    if ( (!Original_SCardLocateCardsByATRA) ) {
        MessageBox(NULL,"Could not find SCardLocateCardsByATRA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardLocateCardsByATRW = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPSCARD_ATRMASK rgAtrMasks,
    IN      DWORD cAtrs,
    IN OUT  LPSCARD_READERSTATEW rgReaderStates,
    IN      DWORD cReaders))
            GetProcAddress(hOriginal,"SCardLocateCardsByATRW");
    if ( (!Original_SCardLocateCardsByATRW) ) {
        MessageBox(NULL,"Could not find SCardLocateCardsByATRW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardGetStatusChangeA = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      DWORD dwTimeout,
    IN OUT  LPSCARD_READERSTATEA rgReaderStates,
    IN      DWORD cReaders))
            GetProcAddress(hOriginal,"SCardGetStatusChangeA");
    if ( (!Original_SCardGetStatusChangeA) ) {
        MessageBox(NULL,"Could not find SCardGetStatusChangeA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardGetStatusChangeW = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      DWORD dwTimeout,
    IN OUT  LPSCARD_READERSTATEW rgReaderStates,
    IN      DWORD cReaders))
            GetProcAddress(hOriginal,"SCardGetStatusChangeW");
    if ( (!Original_SCardGetStatusChangeW) ) {
        MessageBox(NULL,"Could not find SCardGetStatusChangeW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardCancel = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext))
            GetProcAddress(hOriginal,"SCardCancel");
    if ( (!Original_SCardCancel) ) {
        MessageBox(NULL,"Could not find SCardCancel procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardConnectA = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR szReader,
    IN      DWORD dwShareMode,
    IN      DWORD dwPreferredProtocols,
    OUT     LPSCARDHANDLE phCard,
    OUT     LPDWORD pdwActiveProtocol))
            GetProcAddress(hOriginal,"SCardConnectA");
    if ( (!Original_SCardConnectA) ) {
        MessageBox(NULL,"Could not find SCardConnectA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardConnectW = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR szReader,
    IN      DWORD dwShareMode,
    IN      DWORD dwPreferredProtocols,
    OUT     LPSCARDHANDLE phCard,
    OUT     LPDWORD pdwActiveProtocol))
            GetProcAddress(hOriginal,"SCardConnectW");
    if ( (!Original_SCardConnectW) ) {
        MessageBox(NULL,"Could not find SCardConnectW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardReconnect = 
        (LONG (__stdcall *)(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwShareMode,
    IN      DWORD dwPreferredProtocols,
    IN      DWORD dwInitialization,
    OUT     LPDWORD pdwActiveProtocol))
            GetProcAddress(hOriginal,"SCardReconnect");
    if ( (!Original_SCardReconnect) ) {
        MessageBox(NULL,"Could not find SCardReconnect procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardDisconnect = 
        (LONG (__stdcall *)(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwDisposition))
            GetProcAddress(hOriginal,"SCardDisconnect");
    if ( (!Original_SCardDisconnect) ) {
        MessageBox(NULL,"Could not find SCardDisconnect procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardBeginTransaction = 
        (LONG (__stdcall *)(
    IN      SCARDHANDLE hCard))
            GetProcAddress(hOriginal,"SCardBeginTransaction");
    if ( (!Original_SCardBeginTransaction) ) {
        MessageBox(NULL,"Could not find SCardBeginTransaction procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardEndTransaction = 
        (LONG (__stdcall *)(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwDisposition))
            GetProcAddress(hOriginal,"SCardEndTransaction");
    if ( (!Original_SCardEndTransaction) ) {
        MessageBox(NULL,"Could not find SCardEndTransaction procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardCancelTransaction = 
        (LONG (__stdcall *)(
    IN      SCARDHANDLE hCard))
            GetProcAddress(hOriginal,"SCardCancelTransaction");
    if ( (!Original_SCardCancelTransaction) ) {
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "Could not find SCardCancelTransaction procedure address\n");
    }

    Original_SCardState = 
        (LONG (__stdcall *)(
    IN SCARDHANDLE hCard,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen))
            GetProcAddress(hOriginal,"SCardState");
    if ( (!Original_SCardState) ) {
        MessageBox(NULL,"Could not find SCardState procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardStatusA = 
        (LONG (__stdcall *)(
    IN SCARDHANDLE hCard,
    OUT LPSTR szReaderName,
    IN OUT LPDWORD pcchReaderLen,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen))
            GetProcAddress(hOriginal,"SCardStatusA");
    if ( (!Original_SCardStatusA) ) {
        MessageBox(NULL,"Could not find SCardStatusA procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardStatusW = 
        (LONG (__stdcall *)(
    IN SCARDHANDLE hCard,
    OUT LPWSTR szReaderName,
    IN OUT LPDWORD pcchReaderLen,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen))
            GetProcAddress(hOriginal,"SCardStatusW");
    if ( (!Original_SCardStatusW) ) {
        MessageBox(NULL,"Could not find SCardStatusW procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardTransmit = 
        (LONG (__stdcall *)(
    IN SCARDHANDLE hCard,
    IN LPCSCARD_IO_REQUEST pioSendPci,
    IN LPCBYTE pbSendBuffer,
    IN DWORD cbSendLength,
    IN OUT LPSCARD_IO_REQUEST pioRecvPci,
    OUT LPBYTE pbRecvBuffer,
    IN OUT LPDWORD pcbRecvLength))
            GetProcAddress(hOriginal,"SCardTransmit");
    if ( (!Original_SCardTransmit) ) {
        MessageBox(NULL,"Could not find SCardTransmit procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardControl = 
        (LONG (__stdcall *)(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwControlCode,
    IN      LPCVOID lpInBuffer,
    IN      DWORD nInBufferSize,
    OUT     LPVOID lpOutBuffer,
    IN      DWORD nOutBufferSize,
    OUT     LPDWORD lpBytesReturned))
            GetProcAddress(hOriginal,"SCardControl");
    if ( (!Original_SCardControl) ) {
        MessageBox(NULL,"Could not find SCardControl procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardGetAttrib = 
        (LONG (__stdcall *)(
    IN SCARDHANDLE hCard,
    IN DWORD dwAttrId,
    OUT LPBYTE pbAttr,
    IN OUT LPDWORD pcbAttrLen))
            GetProcAddress(hOriginal,"SCardGetAttrib");
    if ( (!Original_SCardGetAttrib) ) {
        MessageBox(NULL,"Could not find SCardGetAttrib procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardSetAttrib = 
        (LONG (__stdcall *)(
    IN SCARDHANDLE hCard,
    IN DWORD dwAttrId,
    IN LPCBYTE pbAttr,
    IN DWORD cbAttrLen))
            GetProcAddress(hOriginal,"SCardSetAttrib");
    if ( (!Original_SCardSetAttrib) ) {
        MessageBox(NULL,"Could not find SCardSetAttrib procedure address","error",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardUIDlgSelectCardA = 
        (LONG (__stdcall *)(
    LPOPENCARDNAMEA_EX))
            GetProcAddress(hOriginal,"SCardUIDlgSelectCardA");
    if ( (!Original_SCardUIDlgSelectCardA) ) {
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "Could not find SCardUIDlgSelectCardA procedure address\n");
    }

    Original_SCardUIDlgSelectCardW = 
        (LONG (__stdcall *)(
    LPOPENCARDNAMEW_EX))
            GetProcAddress(hOriginal,"SCardUIDlgSelectCardW");
    if ( (!Original_SCardUIDlgSelectCardW) ) {
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "Could not find SCardUIDlgSelectCardW procedure address\n");
    }

    Original_GetOpenCardNameA = 
        (LONG (__stdcall *)(LPOPENCARDNAMEA))
            GetProcAddress(hOriginal,"GetOpenCardNameA");
    if ( (!Original_GetOpenCardNameA) ) {
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "Could not find GetOpenCardNameA procedure address\n");
    }

    Original_GetOpenCardNameW = 
        (LONG (__stdcall *)(LPOPENCARDNAMEW))
            GetProcAddress(hOriginal,"GetOpenCardNameW");
    if ( (!Original_GetOpenCardNameW) ) {
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "Could not find GetOpenCardNameW procedure address\n");
    }

    Original_SCardDlgExtendedError  = 
        (LONG (__stdcall *)(void))
            GetProcAddress(hOriginal,"SCardDlgExtendedError ");
    if ( (!Original_SCardDlgExtendedError ) ) {
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "Could not find SCardDlgExtendedError procedure address\n");
    }




	return TRUE;
}

int CWinscardApp::ConnectSCSAT04(SCSAT04_CONFIG* pSCSATConfig) {
    string     message;
    
    std::string sIP(pSCSATConfig->IP);
    pSCSATConfig->pSocket = new SocketClient(sIP, atoi(pSCSATConfig->port.c_str()));
    string l = pSCSATConfig->pSocket->ReceiveLine(SCSAT_SOCKET_TIMEOUT);
    message = string_format("\n> SCSAT connect ... %s", l.c_str());
    CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);

    // INIT SCSAT CONNECTION
    pSCSATConfig->pSocket->SendLine("get init 1000");
    l = pSCSATConfig->pSocket->ReceiveResponse(SCSAT_SOCKET_ENDSEQ, SCSAT_SOCKET_TIMEOUT);
    message = string_format("\n:: %s", l.c_str());
    CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    
    return STAT_OK;
}

int CWinscardApp::SCSAT_CreateAndReceiveSamples(SCSAT04_CONFIG* pSCSATConfig, string* pNewFilePath) {
    int             status = STAT_OK;
    string         message;
    string         sampleFilePath;
    SCSAT_MEASURE_INFO  measureInfo;

    
    // store info about measure
    measureInfo.baseOffset = m_scsat04Config.baseReadOffset;
    measureInfo.frequency = (m_scsat04Config.readRatio == 0) ? SCSAT_MAX_SAMPLING_FREQUENCY : (SCSAT_MAX_SAMPLING_FREQUENCY / m_scsat04Config.readRatio);
    
    sampleFilePath = string_format("dataout.datx");
    CCommonFnc::File_GetAvailableFileName(sampleFilePath, &sampleFilePath);
    
    // WRITE MEASUREMENT
    int numSamples = 0; 
    BYTE bytesPerSample = 2;
	SAMPLE_PLOT* pReceivedSample = new SAMPLE_PLOT;
	pReceivedSample->dataFilePath = sampleFilePath;
    pReceivedSample->dataBlob.dwActLen = pReceivedSample->dataBlob.dwMaxLen = SCSAT_MAX_NUMBER_OF_SAMPLES;
    pReceivedSample->dataBlob.pData = new int[pReceivedSample->dataBlob.dwMaxLen];
    pReceivedSample->measureInfo.copy(&measureInfo);
    
    pSCSATConfig->pSocket->ReceiveLineToMemory(&(pReceivedSample->dataBlob), SCSAT_SOCKET_SHORT_TIMEOUT, bytesPerSample);

    measureInfo.numSamples = pReceivedSample->dataBlob.dwActLen;

    // store number of written samples
    string tmp;
    tmp = string_format("%d", measureInfo.numSamples);
    CCommonFnc::SCSAT_EnsureFileHeader(sampleFilePath, &measureInfo);
    WritePrivateProfileString(SCSAT_MEASURE_SECTION, SCSAT_MEASURE_NUMSAMPLES, tmp.c_str(), sampleFilePath.c_str());
    
    // WRITE MEASUREMENT INFO INTO FILE 
	CCommonFnc::SCSAT_SaveSamples(sampleFilePath, pReceivedSample);
	CCommonFnc::Sample_Free(pReceivedSample);    
	delete pReceivedSample;
    
   
    *pNewFilePath = sampleFilePath;
                                
    return status;
}



int CWinscardApp::ApplyRules(BYTE* pbBuffer, DWORD* pcbLength, int direction) {
    int             status = STAT_OK;
    
    if (m_bRulesActive) {
        lar::iterator   iter;
        lasr::iterator  iter2;
        APDU_SINGLE_RULE    singleRule;    
        BYTE            tempBuffer[MAX_APDU_LENGTH];
        BYTE            newBuffer[MAX_APDU_LENGTH];
        BOOL            bRuleFound = FALSE;
        
        // MAKE TEMP COPY
        memcpy(newBuffer, pbBuffer, *pcbLength);
        
        // PROCESS ALL RULES, IF MATCH THEN MODIFY BUFFER
        for (iter = rulesList.begin(); iter != rulesList.end(); iter++) {
            if ((iter->direction == direction) && (iter->usage == 1)) {
                //TEST ALL MATCH RULES
                BOOL    bAllMatch = TRUE;
                for (iter2 = iter->matchRules.begin(); iter2 != iter->matchRules.end(); iter2++) {
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
                    // LOG ACTON
                    string message; message = string_format("   rule applied: %s\n", iter->ruleName);
                    CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
                
                    for (iter2 = iter->actionRules.begin(); iter2 != iter->actionRules.end(); iter2++) {
                        singleRule = *iter2;
                        if (singleRule.valid) {
                            if (singleRule.element == LE_ELEM) *pcbLength = singleRule.value;
                            else newBuffer[singleRule.element] = singleRule.value;
                        }
                    }
                    
                    // NEDETERMINISTIC SLEEP IF REQUIRED
                    if (iter->msDelay > 0) {
                        _sleep(iter->msDelay); 
                        // SLEEP RANDOMLY UP TO 1/10 OF ORIGINAL TIME
                        _sleep(rand() % (iter->msDelay / 10));
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

LONG CWinscardApp::SCSAT_SCardTransmit(SCSAT04_CONFIG* pSCSATConfig, SCARD_IO_REQUEST* pioSendPci, LPCBYTE pbSendBuffer, DWORD cbSendLength, SCARD_IO_REQUEST* pioRecvPci, LPBYTE pbRecvBuffer, LPDWORD pcbRecvLength) {
    LONG        status = 0;
    string     message;
    string     value;    
    
    if (pSCSATConfig->pSocket != NULL) {
        try {
            // FORMAT APDU STRING
            CCommonFnc::BYTE_ConvertFromArrayToHexString((BYTE*) pbSendBuffer, cbSendLength, &value);
            message = string_format("%s %s", SCSAT_GET_APDU, value);
            string l((LPCSTR) message.c_str());
            pSCSATConfig->pSocket->SendLine(l);
            //message.Insert(0, "\n::-> ");
			message.insert(0, "\n::-> ");
            CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
            
            // SLEEP LONGER, IF MORE DATA WILL BE RETURNED BY SYSTEM 00 0c 00 00 xx CALL            
            if (memcmp(pbSendBuffer, GET_APDU1, sizeof(GET_APDU1)) == 0 || memcmp(pbSendBuffer, GET_APDU2, sizeof(GET_APDU2)) == 0) {
                _sleep(pbSendBuffer[4] * 20);       // LC * 20ms
            }
            else _sleep(500);
             
            
            // OBTAIN RESPONSE, PARSE BACK 
            l = pSCSATConfig->pSocket->ReceiveResponse(SCSAT_SOCKET_ENDSEQ, SCSAT_SOCKET_TIMEOUT);
            message = string_format("\n::<- %s", l.c_str());
            //message.Replace("\n", " ");
			replace(message.begin(), message.end(), '\n', ' ');
            CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
            
            // CHECK IF RESPONSE IS CORRECT
            string response = l.c_str();
            //response.MakeLower();
			char c; int i = 0;
			while (response[i])
			{
				c = response[i];
				putchar(tolower(c));
				i++;
			}
            if (response.find(SCSAT_GET_APDU_FAIL) == string::npos) {
                // RESPONSE CORRECT
				int position = response.find("\n");
	            if (position == string::npos) {
					position = -1;
	            }
		        value = response.substr(position + 1, string::npos);
				
				string tempVal = "";
				if (position - 1 > 0) {
					//string tempVal = value.Left(value.Find("\n")-1);
					tempVal = value.substr(0, position - 1); // <--------------- OTAZKA
				}   
                value = tempVal;
                // NOTE: pbRecvBuffer IS ASSUMED TO HAVE 260B
                *pcbRecvLength = 260;
                status = CCommonFnc::BYTE_ConvertFromHexStringToArray(value, pbRecvBuffer, pcbRecvLength);
                
                // CHECK FOR RETURN STATUS, AT LEAST 2 BYTES REQUIRED
                if (*pcbRecvLength < 2) status = STAT_DATA_INCORRECT_LENGTH;
            }
            else {
                // COMMAND FAIL 
                status = STAT_SCARD_ERROR;
            }
        }
        catch (const char* s) {
            message = string_format("\nSCSAT_SCardTransmit(), SendLine(%s), fail with (%s)", message, s);
            CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
            status = SCARD_F_UNKNOWN_ERROR;
        } 
        catch (...) {
            message = string_format("\nSCSAT_SCardTransmit(), SendLine(%s), fail with (unhandled exception)", message);
            CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
            status = SCARD_F_UNKNOWN_ERROR;
        }
    }

    return status;
}

int CWinscardApp::LoadRule(string ruleName, string filePath) {
    int     status = STAT_OK;
    char    buffer[10000];
    DWORD   cBuffer = 10000;
    string valueName;
    string rulePart;
    string ruleString;
    string elemName;
    string subValue;
    string help;
    APDU_RULE   rule;
    APDU_SINGLE_RULE    singleRule;
    
    if (compareWithNoCase(ruleName.c_str(), "WINSCARD") == 0) {
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "AUTO_REQUEST_DATA", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_winscardConfig.bAUTO_REQUEST_DATA = (atoi(buffer) == 0) ? FALSE : TRUE;
        }
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "FORCE_CONNECT_SHARED_MODE", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_winscardConfig.bFORCE_CONNECT_SHARED_MODE = (atoi(buffer) == 0) ? FALSE : TRUE;
        }
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "FORCE_APDU_NONZERO_INPUT_DATA", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_winscardConfig.bFORCE_APDU_NONZERO_INPUT_DATA = (atoi(buffer) == 0) ? FALSE : TRUE;
        }
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "LOG_EXCHANGED_APDU", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_winscardConfig.bLOG_EXCHANGED_APDU = (atoi(buffer) == 0) ? FALSE : TRUE;
        }
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "LOG_BASE_PATH", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_winscardConfig.sLOG_BASE_PATH = buffer;
        } 
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "MODIFY_APDU_BY_RULES", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_winscardConfig.bMODIFY_APDU_BY_RULES = (atoi(buffer) == 0) ? FALSE : TRUE;
        }
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "LOG_FUNCTIONS_CALLS", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_winscardConfig.bLOG_FUNCTIONS_CALLS = (atoi(buffer) == 0) ? FALSE : TRUE;
        }
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "READER_ORDERED_FIRST", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_winscardConfig.sREADER_ORDERED_FIRST = buffer;
        }
        
    }
        
    if (compareWithNoCase(ruleName.c_str(), "SCSAT04") == 0) {
        // SCSAT04 CONFIGURATION RULE
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "REDIRECT", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.bRedirect = (atoi(buffer) == 0) ? FALSE : TRUE;
        }
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "IP", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.IP = buffer;
        }
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "PORT", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.port = buffer;
        }
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "MEASURE_APDU", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.measureApduLen = sizeof(m_scsat04Config.measureApdu);
            CCommonFnc::BYTE_ConvertFromHexStringToArray(buffer, m_scsat04Config.measureApdu, &(m_scsat04Config.measureApduLen));
        }
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "MEASURE_BYTE_COUNTER", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.measureApduByteCounter = atoi(buffer);
        }
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "MEASURE_BYTE_DELAY", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.measureApduByteDelay = atoi(buffer);
        }
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "READ_RATIO", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.readRatio = atoi(buffer);
        }
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "NUM_SAMPLES", "", buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.numSamples = atoi(buffer);
        }
        
        
        
    }
    if (compareWithNoCase(ruleName.substr(0, (int)strlen("RULE")).c_str(), "RULE") == 0) {
        // COMMON RULE
    
        if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "USAGE", "", buffer, cBuffer, filePath.c_str())) > 0) {
            rule.usage = atoi(buffer);
            
            if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "APDUIN", "", buffer, cBuffer, filePath.c_str())) > 0) {
                rule.direction = atoi(buffer);
            }
            if ((GetPrivateProfileString((LPCTSTR) ruleName.c_str(), "DELAY", "", buffer, cBuffer, filePath.c_str())) > 0) {
                rule.msDelay = atoi(buffer);
            }
            
            
            // SET RULE NAME FOR FUTURE IDENTIFICATION
            rule.ruleName = ruleName;
            
            // LOAD MATCH RULES
            int counter = 1;
            int pos = 0;
            int pos2 = 0;
            valueName = string_format("MATCH%d", counter);
            while((GetPrivateProfileString(ruleName.c_str(), valueName.c_str(), "", buffer, cBuffer, filePath.c_str())) > 0) {
                ruleString = buffer; ruleString += " ";
                
                // FIND HISTORY ELEMENT, WILL BE SAME FOR ALL OTHER ELEMENTAREY RULES
                if ((pos = ruleString.find("t=")) != string::npos) {
                    singleRule.history = atoi(ruleString.substr(pos + (int) strlen("t=")).c_str());
                    ruleString.erase(pos, ruleString.find(";", pos) - pos + 1); // remove from rule string
                }                    
                // FIND DIRECTION ELEMENT (IN/OUT), WILL BE SAME FOR ALL OTHER ELEMENTAREY RULES
                if ((pos = ruleString.find("in=")) != string::npos) {
                    singleRule.apduDirection = atoi(ruleString.substr(pos + (int) strlen("in=")).c_str());
                    ruleString.erase(pos, ruleString.find(";", pos) - pos + 1); // remove from rule string
                }                    

                // PARSE RULE AND CREATE ELEMENTARY RULES FOREACH BYTE
                pos2 = 0;
                while ((pos = ruleString.find(";", pos2)) != -1) {
                    rulePart = ruleString.substr(pos2, pos - pos2 + 1);
                    
                    //elemName = rulePart.Left(rulePart.Find("="));
					elemName = rulePart.substr(0, rulePart.find("="));

                    if (compareWithNoCase(elemName.c_str(), "CLA") == 0) {
                        singleRule.element = CLA_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find("=")+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.matchRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), "INS") == 0) {
                        singleRule.element = INS_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find("=")+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.matchRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), "P1") == 0) {
                        singleRule.element = P1_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find("=")+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.matchRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), "P2") == 0) {
                        singleRule.element = P2_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find("=")+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.matchRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), "LC") == 0) {
                        singleRule.element = LC_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find("=")+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.matchRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.substr(0, (int) strlen("DATA")).c_str(), "DATA") == 0) {
                        // DATA CAN BE WRITTEN IN MORE VALUES AT ONCE, STARTING ON POSITION DATAx
                        // CREATE SEPARATE ELEMENT FOR EACH
                        int offset = atoi(elemName.substr(ruleName.find_first_of("0123456789"), 0).c_str());
                        // GO OVER ALL MATCH DATA
                        string data = rulePart.substr(rulePart.find("=") + 1);
                        //data.Replace(";", "");
						data.erase(remove(data.begin(), data.end(), ';'), data.end());
                        BYTE    dataBuffer[300];
                        DWORD   dataBufferLen = 300;
                        CCommonFnc::BYTE_ConvertFromHexStringToArray(data, dataBuffer, &dataBufferLen);
                        for (DWORD i = 0; i < dataBufferLen; i++) {
                            if (singleRule.apduDirection == INPUT_APDU) singleRule.element = offset + OFFSET_CDATA;    
                            else singleRule.element = offset;
                            singleRule.value = dataBuffer[i];
                            singleRule.valid = TRUE;rule.matchRules.push_back(singleRule);
                            // increase offset for next element
                            offset++; 
                        }
                    } 
                
                    pos2 = pos + 1; 
                }
                            
                counter++;
                valueName = string_format("MATCH%d", counter);
            }

            // LOAD ACTION RULES
            counter = 1;
            pos = 0;
            if ((GetPrivateProfileString(ruleName.c_str(), "ACTION", "", buffer, cBuffer, filePath.c_str())) > 0) {
                ruleString = buffer; ruleString += " ";
                // PARSE RULE AND CREATE ELEMENTARY RULES FOREACH BYTE
                singleRule.clear();
                // FIND DIRECTION ELEMENT (IN/OUT), WILL BE SAME FOR ALL OTHER ELEMENTARY RULES
                if ((pos = ruleString.find("in=")) != string::npos) {
                    singleRule.apduDirection = atoi(ruleString.substr(pos + (int) strlen("in=")).c_str());
                    //ruleString.Delete(pos, ruleString.find(";", pos) - pos + 1); // remove from rule string
					ruleString.erase(pos, ruleString.find(";", pos) - pos + 1);
                }                    
                pos2 = 0;
                while ((pos = ruleString.find(";", pos2)) != string::npos) {
                    rulePart = ruleString.substr(pos2, pos - pos2 + 1);
                    
                    elemName = rulePart.substr(0, rulePart.find("="));
                    
                    if (compareWithNoCase(elemName.c_str(), "CLA") == 0) {
                        singleRule.element = CLA_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find("=")+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.actionRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), "INS") == 0) {
                        singleRule.element = INS_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find("=")+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.actionRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), "P1") == 0) {
                        singleRule.element = P1_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find("=")+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.actionRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), "P2") == 0) {
                        singleRule.element = P2_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find("=")+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.actionRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), "LC") == 0) {
                        singleRule.element = LC_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find("=")+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.actionRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), "LE") == 0) {
                        singleRule.element = LE_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find("=")+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.actionRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.substr(0, (int) strlen("DATA")).c_str(), "DATA") == 0) {
                        // DATA CAN BE WRITTEN IN MORE VALUES AT ONCE, STARTING ON POSITION DATAx
                        // CREATE SEPARATE ELEMENT FOR EACH
                        int offset = atoi(elemName.substr(ruleName.find_first_of("0123456789")).c_str());
                        // GO OVER ALL MATCH DATA
                        string data = rulePart.substr(rulePart.find("=") + 1);
                        //data.Replace(";", "");
						data.erase(remove(data.begin(), data.end(), ';'), data.end());
                        BYTE    dataBuffer[300];
                        DWORD   dataBufferLen = 300;
                        CCommonFnc::BYTE_ConvertFromHexStringToArray(data, dataBuffer, &dataBufferLen);
                        for (DWORD i = 0; i < dataBufferLen; i++) {
                            if (singleRule.apduDirection == INPUT_APDU) singleRule.element = offset + OFFSET_CDATA;    
                            else singleRule.element = offset;
                            singleRule.value = dataBuffer[i];
                            singleRule.valid = TRUE;rule.actionRules.push_back(singleRule);
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
    char    buffer[10000];
    DWORD   cBuffer = 10000;
    DWORD   cReaded = 0;
    lcs     valuesList;
    lcs::iterator   iter;
    string filePath;
    
    memset(buffer, 0, cBuffer);
  
    CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "#########################################\n");
    
    // OBTAIN FULL FILE PATH FOR RULES FILE
    CFile   file;
    if (file.Open(RULE_FILE, CFile::modeRead)) {
        filePath = file.GetFilePath();
        file.Close();
        
        string message; 
    	message = string_format("Rules file found: %s\n", filePath);
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);

        // OBTAIN SECTION NAMES
        if ((cReaded = GetPrivateProfileString(NULL, NULL, "", buffer, cBuffer, filePath.c_str())) != 0) {
            // PARSE SECTION NAMES, TRY TO LOAD EACH RULE
            CCommonFnc::String_ParseNullSeparatedArray((BYTE*) buffer, cBuffer, &valuesList);
            
            for (iter = valuesList.begin(); iter != valuesList.end(); iter++) {
                LoadRule(*iter, filePath);    
            }
        }
        
        m_bRulesActive = TRUE;
    }
    else {
        // NO RULES DETECTED
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "Rules file NOT found\n");
    }

	WINSCARD_RULES_LOG = string_format("%swinscard_rules_log.txt", m_winscardConfig.sLOG_BASE_PATH);
	WINSCARD_LOG = string_format("%swinscard_log.txt", m_winscardConfig.sLOG_BASE_PATH);
	

    return status;
}

