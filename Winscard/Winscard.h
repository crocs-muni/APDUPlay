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
	unordered_map<string_type, string_type> remoteCardsATRMap;
	DWORD   m_nextRemoteCardID = 1;
	
	string_type loadingBinaryName;
	


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
	LONG Remote_SendRequest(REMOTE_CONFIG* pRemoteConfig, string_type targetReader, string_type command, string_type commandData, string_type* pResponse);
	LONG Remote_SCardConnect(REMOTE_CONFIG* pRemoteConfig, string_type targetReader, string_type* pATR);
	LONG Remote_ParseResponse(string_type rawResponse, DWORD expectedUniqueID, string_type* respCommand);
	LONG Remote_ListReaders(REMOTE_CONFIG* pRemoteConfig, list<string_type>* pResponse);
	LONG Remote_SCardReleaseContext();
#endif
	int LoadRule(string_type ruleName, dictionary* dict);

	string_type GetReaderName(IN SCARDHANDLE hCard);
	boolean		IsRemoteCard(IN SCARDHANDLE hCard);
	boolean		IsRemoteReader(IN string_type readerName);


#if defined (_WIN32)
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
#endif
};
