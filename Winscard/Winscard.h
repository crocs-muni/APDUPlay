// Winscard.h : main header file for the Winscard DLL
//

#pragma once
//#include <afxdtctl.h>
#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "../Iniparser/iniparser_all.h"

#define     SCSAT_MAX_SAMPLING_FREQUENCY        100
#define		SCSAT_MAX_NUMBER_OF_SAMPLES			48000000 




// CWinscardApp
// See Winscard.cpp for the implementation of this class
//

#ifdef __linux__
	class CWinApp {};
#endif

class CWinscardApp : public CWinApp
{
public:
	CWinscardApp();
    ~CWinscardApp();
// Overrides

    lar     rulesList;
    lab     apduInList;    
    lab     apduOutList;    
    BOOL    m_bRulesActive;

#if defined (_WIN32)
    SCSAT04_CONFIG  m_scsat04Config;
#endif
    WINSCARD_CONFIG m_winscardConfig;
    int             m_processedApduByteCounter;

    lptr    m_charAllocatedMemoryList;
    lptr    m_wcharAllocatedMemoryList;

#if defined (_WIN32)
    int ConnectSCSAT04(SCSAT04_CONFIG* pSCSATConfig);
    int SCSAT_CreateAndReceiveSamples(SCSAT04_CONFIG* pSCSATConfig, string_type* pNewFilePath);
#endif

    int LoadRule(const char_type* section_name, dictionary* dict);
    int LoadRules();

    int ApplyRules(BYTE* pbSendBuffer, DWORD* pcbSendLength, int direction);
    int GetApduFromHistory(BYTE* buffer, int history, int apduDirection);

#if defined (_WIN32)
//	int SCSAT_EnsureFileHeader(CString filePath, SCSAT_MEASURE_INFO* pInfo);
    LONG SCSAT_SCardTransmit(SCSAT04_CONFIG* pSCSATConfig, SCARD_IO_REQUEST* pioSendPci, LPCBYTE pbSendBuffer, DWORD cbSendLength, SCARD_IO_REQUEST* pioRecvPci, LPBYTE pbRecvBuffer, LPDWORD pcbRecvLength);
#endif
	int LoadRule(string_type ruleName, dictionary* dict);

#if defined (_WIN32)
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
#endif
};
