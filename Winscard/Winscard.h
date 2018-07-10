// Winscard.h : main header file for the Winscard DLL
//
#if defined(_WIN32)
#pragma once
//#include <afxdtctl.h>
#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#endif
#include "../Iniparser/iniparser_all.h"
#include <unordered_map>

using namespace std;


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
	dictionary* instructionDict = nullptr;
	unordered_map<SCARDHANDLE, string_type> cardReaderMap;
	unordered_map<SCARDHANDLE, string_type>	remoteReadersMap;
	DWORD   m_nextRemoteCardID = 1;
	


#if defined (_WIN32)
    REMOTE_CONFIG  m_remoteConfig;
#endif
    WINSCARD_CONFIG m_winscardConfig;
    int             m_processedApduByteCounter;

    lptr    m_charAllocatedMemoryList;
    lptr    m_wcharAllocatedMemoryList;

#if defined (_WIN32)
    int Remote_Connect(REMOTE_CONFIG* pRemoteConfig);
#endif

    int LoadRule(const char_type* section_name, dictionary* dict);
	int LoadRule(string_type ruleName, string_type filePath);
	int LoadRules();

	void WriteDescription(BYTE insByte);

    int ApplyRules(BYTE* pbSendBuffer, DWORD* pcbSendLength, int direction);
    int GetApduFromHistory(BYTE* buffer, int history, int apduDirection);

#if defined (_WIN32)
    LONG Remote_SCardTransmit(REMOTE_CONFIG* pRemoteConfig, string_type targetReader, SCARD_IO_REQUEST* pioSendPci, LPCBYTE pbSendBuffer, DWORD cbSendLength, SCARD_IO_REQUEST* pioRecvPci, LPBYTE pbRecvBuffer, LPDWORD pcbRecvLength);
	LONG Remote_SCardConnect(REMOTE_CONFIG* pRemoteConfig, string_type targetReader);
	LONG Remote_ParseResponse(string_type rawResponse, DWORD expectedUniqueID, string_type* respCommand);
#endif
	int LoadRule(string_type ruleName, dictionary* dict);

	string_type GetReaderName(IN SCARDHANDLE hCard);
	boolean		IsRemoteCard(IN SCARDHANDLE hCard);

#if defined (_WIN32)
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
#endif
};
