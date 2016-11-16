#include "stdafx.h"
#include "Interface.h"

// CWinscardApp initialization

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
	if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) CCommonFnc::File_AppendString(WINSCARD_LOG, _CONV("[begin]\r\n"));

    hOriginal = LoadLibrary(_CONV("original.dll"));
    if ( ! hOriginal ) {
        MessageBox(NULL, _CONV("could not load original library"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }
    
    Original_SCardTransmit = 
        (long (__stdcall *)(unsigned long,const struct _SCARD_IO_REQUEST *,const unsigned char *,unsigned long,struct _SCARD_IO_REQUEST *,unsigned char *,unsigned long *))
            GetProcAddress(hOriginal,"SCardTransmit");
    if ( (!Original_SCardTransmit) ) {
        MessageBox(NULL, _CONV("could not find SCardTransmit procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardStatusA = 
        (long (__stdcall *)(SCARDHANDLE hCard, LPSTR szReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen))
            GetProcAddress(hOriginal,"SCardStatusA");
    if ( (!Original_SCardStatusA) ) {
        MessageBox(NULL, _CONV("could not find SCardStatusA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }
    
    Original_SCardConnectW = 
        (long (__stdcall *)(SCARDCONTEXT hContext,LPCWSTR szReader,DWORD dwShareMode,DWORD dwPreferredProtocols,LPSCARDHANDLE phCard,LPDWORD pdwActiveProtocol))
            GetProcAddress(hOriginal,"SCardConnectW");
    if ( (!Original_SCardConnectW) ) {
        MessageBox(NULL, _CONV("could not find SCardConnectW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }
    
    Original_SCardDisconnect = 
        (long (__stdcall *)(SCARDHANDLE hCard, DWORD dwDisposition))
            GetProcAddress(hOriginal,"SCardDisconnect");
    if ( (!Original_SCardDisconnect) ) {
        MessageBox(NULL, _CONV("could not find SCardDisconnect procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardFreeMemory = 
        (long (__stdcall *)(SCARDCONTEXT hContext, LPCVOID pvMem))
            GetProcAddress(hOriginal,"SCardFreeMemory");
    if ( (!Original_SCardFreeMemory) ) {
        MessageBox(NULL, _CONV("could not find SCardFreeMemory procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }
    
    Original_SCardListReadersW = 
        (long (__stdcall *)(SCARDCONTEXT hContext, LPCWSTR mszGroups, LPWSTR mszReaders, LPDWORD pcchReaders))
            GetProcAddress(hOriginal,"SCardListReadersW");
    if ( (!Original_SCardListReadersW) ) {
        MessageBox(NULL, _CONV("could not find SCardListReadersW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }
    
    Original_SCardListReadersA = 
        (long (__stdcall *)(SCARDCONTEXT hContext, LPCSTR mszGroups, LPSTR mszReaders, LPDWORD pcchReaders))
            GetProcAddress(hOriginal,"SCardListReadersA");
    if ( (!Original_SCardListReadersA) ) {
        MessageBox(NULL, _CONV("could not find SCardListReadersA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardEstablishContext procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardReleaseContext = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext))
            GetProcAddress(hOriginal,"SCardReleaseContext");
    if ( (!Original_SCardReleaseContext) ) {
        MessageBox(NULL, _CONV("Could not find SCardReleaseContext procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardIsValidContext = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext))
            GetProcAddress(hOriginal,"SCardIsValidContext");
    if ( (!Original_SCardIsValidContext) ) {
        MessageBox(NULL, _CONV("Could not find SCardIsValidContext procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardListReaderGroupsA = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    OUT     LPSTR mszGroups,
    IN OUT  LPDWORD pcchGroups))
            GetProcAddress(hOriginal,"SCardListReaderGroupsA");
    if ( (!Original_SCardListReaderGroupsA) ) {
        MessageBox(NULL, _CONV("Could not find SCardListReaderGroupsA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardListReaderGroupsW = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    OUT     LPWSTR mszGroups,
    IN OUT  LPDWORD pcchGroups))
            GetProcAddress(hOriginal,"SCardListReaderGroupsW");
    if ( (!Original_SCardListReaderGroupsW) ) {
        MessageBox(NULL, _CONV("Could not find SCardListReaderGroupsW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardListReadersA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardListReadersW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardListCardsA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardListCardsW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardListInterfacesA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardListInterfacesW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardGetProviderIdA = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPCSTR szCard,
    OUT     LPGUID pguidProviderId))
            GetProcAddress(hOriginal,"SCardGetProviderIdA");
    if ( (!Original_SCardGetProviderIdA) ) {
        MessageBox(NULL, _CONV("Could not find SCardGetProviderIdA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardGetProviderIdW = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext,
    IN      LPCWSTR szCard,
    OUT     LPGUID pguidProviderId))
            GetProcAddress(hOriginal,"SCardGetProviderIdW");
    if ( (!Original_SCardGetProviderIdW) ) {
        MessageBox(NULL, _CONV("Could not find SCardGetProviderIdW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardGetCardTypeProviderNameA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardGetCardTypeProviderNameW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardIntroduceReaderGroupA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szGroupName))
            GetProcAddress(hOriginal,"SCardIntroduceReaderGroupA");
    if ( (!Original_SCardIntroduceReaderGroupA) ) {
        MessageBox(NULL, _CONV("Could not find SCardIntroduceReaderGroupA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardIntroduceReaderGroupW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szGroupName))
            GetProcAddress(hOriginal,"SCardIntroduceReaderGroupW");
    if ( (!Original_SCardIntroduceReaderGroupW) ) {
        MessageBox(NULL, _CONV("Could not find SCardIntroduceReaderGroupW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardForgetReaderGroupA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szGroupName))
            GetProcAddress(hOriginal,"SCardForgetReaderGroupA");
    if ( (!Original_SCardForgetReaderGroupA) ) {
        MessageBox(NULL, _CONV("Could not find SCardForgetReaderGroupA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardForgetReaderGroupW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szGroupName))
            GetProcAddress(hOriginal,"SCardForgetReaderGroupW");
    if ( (!Original_SCardForgetReaderGroupW) ) {
        MessageBox(NULL, _CONV("Could not find SCardForgetReaderGroupW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardIntroduceReaderA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szDeviceName))
            GetProcAddress(hOriginal,"SCardIntroduceReaderA");
    if ( (!Original_SCardIntroduceReaderA) ) {
        MessageBox(NULL, _CONV("Could not find SCardIntroduceReaderA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardIntroduceReaderW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szDeviceName))
            GetProcAddress(hOriginal,"SCardIntroduceReaderW");
    if ( (!Original_SCardIntroduceReaderW) ) {
        MessageBox(NULL, _CONV("Could not find SCardIntroduceReaderW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardForgetReaderA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName))
            GetProcAddress(hOriginal,"SCardForgetReaderA");
    if ( (!Original_SCardForgetReaderA) ) {
        MessageBox(NULL, _CONV("Could not find SCardForgetReaderA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardForgetReaderW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName))
            GetProcAddress(hOriginal,"SCardForgetReaderW");
    if ( (!Original_SCardForgetReaderW) ) {
        MessageBox(NULL, _CONV("Could not find SCardForgetReaderW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardAddReaderToGroupA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szGroupName))
            GetProcAddress(hOriginal,"SCardAddReaderToGroupA");
    if ( (!Original_SCardAddReaderToGroupA) ) {
        MessageBox(NULL, _CONV("Could not find SCardAddReaderToGroupA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardAddReaderToGroupW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szGroupName))
            GetProcAddress(hOriginal,"SCardAddReaderToGroupW");
    if ( (!Original_SCardAddReaderToGroupW) ) {
        MessageBox(NULL, _CONV("Could not find SCardAddReaderToGroupW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardRemoveReaderFromGroupA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szReaderName,
    IN LPCSTR szGroupName))
            GetProcAddress(hOriginal,"SCardRemoveReaderFromGroupA");
    if ( (!Original_SCardRemoveReaderFromGroupA) ) {
        MessageBox(NULL, _CONV("Could not find SCardRemoveReaderFromGroupA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardRemoveReaderFromGroupW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szGroupName))
            GetProcAddress(hOriginal,"SCardRemoveReaderFromGroupW");
    if ( (!Original_SCardRemoveReaderFromGroupW) ) {
        MessageBox(NULL, _CONV("Could not find SCardRemoveReaderFromGroupW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardIntroduceCardTypeA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardIntroduceCardTypeW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardSetCardTypeProviderNameA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardSetCardTypeProviderNameW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardForgetCardTypeA = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCSTR szCardName))
            GetProcAddress(hOriginal,"SCardForgetCardTypeA");
    if ( (!Original_SCardForgetCardTypeA) ) {
        MessageBox(NULL, _CONV("Could not find SCardForgetCardTypeA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardForgetCardTypeW = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName))
            GetProcAddress(hOriginal,"SCardForgetCardTypeW");
    if ( (!Original_SCardForgetCardTypeW) ) {
        MessageBox(NULL, _CONV("Could not find SCardForgetCardTypeW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardFreeMemory = 
        (LONG (__stdcall *)(
    IN SCARDCONTEXT hContext,
    IN LPCVOID pvMem))
            GetProcAddress(hOriginal,"SCardFreeMemory");
    if ( (!Original_SCardFreeMemory) ) {
        MessageBox(NULL, _CONV("Could not find SCardFreeMemory procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardAccessStartedEvent = 
        (HANDLE (__stdcall *)(void))
            GetProcAddress(hOriginal,"SCardAccessStartedEvent");
    if ( (!Original_SCardAccessStartedEvent) ) {
        MessageBox(NULL, _CONV("Could not find SCardAccessStartedEvent procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardReleaseStartedEvent = 
        (void (__stdcall *)(void))
            GetProcAddress(hOriginal,"SCardReleaseStartedEvent");
    if ( (!Original_SCardReleaseStartedEvent) ) {
        MessageBox(NULL, _CONV("Could not find SCardReleaseStartedEvent procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardLocateCardsA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardLocateCardsW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardLocateCardsByATRA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardLocateCardsByATRW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardGetStatusChangeA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardGetStatusChangeW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardCancel = 
        (LONG (__stdcall *)(
    IN      SCARDCONTEXT hContext))
            GetProcAddress(hOriginal,"SCardCancel");
    if ( (!Original_SCardCancel) ) {
        MessageBox(NULL, _CONV("Could not find SCardCancel procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardConnectA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardConnectW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardReconnect procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardDisconnect = 
        (LONG (__stdcall *)(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwDisposition))
            GetProcAddress(hOriginal,"SCardDisconnect");
    if ( (!Original_SCardDisconnect) ) {
        MessageBox(NULL, _CONV("Could not find SCardDisconnect procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardBeginTransaction = 
        (LONG (__stdcall *)(
    IN      SCARDHANDLE hCard))
            GetProcAddress(hOriginal,"SCardBeginTransaction");
    if ( (!Original_SCardBeginTransaction) ) {
        MessageBox(NULL, _CONV("Could not find SCardBeginTransaction procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardEndTransaction = 
        (LONG (__stdcall *)(
    IN      SCARDHANDLE hCard,
    IN      DWORD dwDisposition))
            GetProcAddress(hOriginal,"SCardEndTransaction");
    if ( (!Original_SCardEndTransaction) ) {
        MessageBox(NULL, _CONV("Could not find SCardEndTransaction procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardCancelTransaction = 
        (LONG (__stdcall *)(
    IN      SCARDHANDLE hCard))
            GetProcAddress(hOriginal,"SCardCancelTransaction");
    if ( (!Original_SCardCancelTransaction) ) {
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("Could not find SCardCancelTransaction procedure address\n"));
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
        MessageBox(NULL, _CONV("Could not find SCardState procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardStatusA procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardStatusW procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardTransmit procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardControl procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL, _CONV("Could not find SCardGetAttrib procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
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
        MessageBox(NULL,_CONV("Could not find SCardSetAttrib procedure address"), _CONV("error"),MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    Original_SCardUIDlgSelectCardA = 
        (LONG (__stdcall *)(
    LPOPENCARDNAMEA_EX))
            GetProcAddress(hOriginal,"SCardUIDlgSelectCardA");
    if ( (!Original_SCardUIDlgSelectCardA) ) {
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("Could not find SCardUIDlgSelectCardA procedure address\n"));
    }

    Original_SCardUIDlgSelectCardW = 
        (LONG (__stdcall *)(
    LPOPENCARDNAMEW_EX))
            GetProcAddress(hOriginal,"SCardUIDlgSelectCardW");
    if ( (!Original_SCardUIDlgSelectCardW) ) {
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("Could not find SCardUIDlgSelectCardW procedure address\n"));
    }

    Original_GetOpenCardNameA = 
        (LONG (__stdcall *)(LPOPENCARDNAMEA))
            GetProcAddress(hOriginal,"GetOpenCardNameA");
    if ( (!Original_GetOpenCardNameA) ) {
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("Could not find GetOpenCardNameA procedure address\n"));
    }

    Original_GetOpenCardNameW = 
        (LONG (__stdcall *)(LPOPENCARDNAMEW))
            GetProcAddress(hOriginal,"GetOpenCardNameW");
    if ( (!Original_GetOpenCardNameW) ) {
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("Could not find GetOpenCardNameW procedure address\n"));
    }

    Original_SCardDlgExtendedError  = 
        (LONG (__stdcall *)(void))
            GetProcAddress(hOriginal,"SCardDlgExtendedError ");
    if ( (!Original_SCardDlgExtendedError ) ) {
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("Could not find SCardDlgExtendedError procedure address\n"));
    }




	return TRUE;
}

int CWinscardApp::ConnectSCSAT04(SCSAT04_CONFIG* pSCSATConfig) {
    string_type     message;
    
    string_type sIP(pSCSATConfig->IP);
    //pSCSATConfig->pSocket = new SocketClient(sIP, atoi(pSCSATConfig->port.c_str()));
	pSCSATConfig->pSocket = new SocketClient(sIP, type_to_int(pSCSATConfig->port.c_str(), NULL, 10));
    string_type l = pSCSATConfig->pSocket->ReceiveLine(SCSAT_SOCKET_TIMEOUT);
    message = string_format(_CONV("\n> SCSAT connect ... %s"), l.c_str());
    CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);

    // INIT SCSAT CONNECTION
    pSCSATConfig->pSocket->SendLine(_CONV("get init 1000"));
    l = pSCSATConfig->pSocket->ReceiveResponse(SCSAT_SOCKET_ENDSEQ, SCSAT_SOCKET_TIMEOUT);
    message = string_format(_CONV("\n:: %s"), l.c_str());
    CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
    
    return STAT_OK;
}

int CWinscardApp::SCSAT_CreateAndReceiveSamples(SCSAT04_CONFIG* pSCSATConfig, string_type* pNewFilePath) {
    int             status = STAT_OK;
    string_type         message;
    string_type         sampleFilePath;
    SCSAT_MEASURE_INFO  measureInfo;

    
    // store info about measure
    measureInfo.baseOffset = m_scsat04Config.baseReadOffset;
    measureInfo.frequency = (m_scsat04Config.readRatio == 0) ? SCSAT_MAX_SAMPLING_FREQUENCY : (SCSAT_MAX_SAMPLING_FREQUENCY / m_scsat04Config.readRatio);
    
    sampleFilePath = string_format(_CONV("dataout.datx"));
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
    string_type tmp;
    tmp = string_format(_CONV("%d"), measureInfo.numSamples);
    CCommonFnc::SCSAT_EnsureFileHeader(sampleFilePath, &measureInfo);
    WritePrivateProfileString(SCSAT_MEASURE_SECTION.c_str(), SCSAT_MEASURE_NUMSAMPLES.c_str(), tmp.c_str(), sampleFilePath.c_str());
    
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
                    string_type message; message = string_format(_CONV("   rule applied: %s\n"), iter->ruleName);
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
    string_type     message;
    string_type     value;    
    
    if (pSCSATConfig->pSocket != NULL) {
        try {
            // FORMAT APDU STRING
            CCommonFnc::BYTE_ConvertFromArrayToHexString((BYTE*) pbSendBuffer, cbSendLength, &value);
            message = string_format(_CONV("%s %s"), SCSAT_GET_APDU, value);
			string_type l(message);
            pSCSATConfig->pSocket->SendLine(l);
            //message.Insert(0, "\n::-> ");
			message.insert(0, _CONV("\n::-> "));
            CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
            
            // SLEEP LONGER, IF MORE DATA WILL BE RETURNED BY SYSTEM 00 0c 00 00 xx CALL            
            if (memcmp(pbSendBuffer, GET_APDU1, sizeof(GET_APDU1)) == 0 || memcmp(pbSendBuffer, GET_APDU2, sizeof(GET_APDU2)) == 0) {
                _sleep(pbSendBuffer[4] * 20);       // LC * 20ms
            }
            else _sleep(500);
             
            
            // OBTAIN RESPONSE, PARSE BACK 
            l = pSCSATConfig->pSocket->ReceiveResponse(SCSAT_SOCKET_ENDSEQ, SCSAT_SOCKET_TIMEOUT);
            message = string_format(_CONV("\n::<- %s"), l.c_str());
            //message.Replace("\n", " ");
			replace(message.begin(), message.end(), '\n', ' ');
            CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
            
            // CHECK IF RESPONSE IS CORRECT
            string_type response = l.c_str();
            //response.MakeLower();
			char c; int i = 0;
			while (response[i])
			{
				c = response[i];
				putchar(tolower(c));
				i++;
			}
            if (response.find(SCSAT_GET_APDU_FAIL) == string_type::npos) {
                // RESPONSE CORRECT
				int position = response.find(_CONV("\n"));
	            if (position == string_type::npos) {
					position = -1;
	            }
		        value = response.substr(position + 1, string_type::npos);
				
				string_type tempVal = _CONV("");
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
            message = string_format(_CONV("\nSCSAT_SCardTransmit(), SendLine(%s), fail with (%s)"), message, s);
            CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
            status = SCARD_F_UNKNOWN_ERROR;
        } 
        catch (...) {
            message = string_format(_CONV("\nSCSAT_SCardTransmit(), SendLine(%s), fail with (unhandled exception)"), message);
            CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);
            status = SCARD_F_UNKNOWN_ERROR;
        }
    }

    return status;
}

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
        if ((GetPrivateProfileString(ruleName.c_str(), _CONV("LOG_FUNCTIONS_CALLS"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
            m_winscardConfig.bLOG_FUNCTIONS_CALLS = (type_to_int(buffer, NULL, 10) == 0) ? FALSE : TRUE;
        }
        if ((GetPrivateProfileString(ruleName.c_str(), _CONV("READER_ORDERED_FIRST"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
            m_winscardConfig.sREADER_ORDERED_FIRST = buffer;
        }
        
    }
        
    if (compareWithNoCase(ruleName.c_str(), _CONV("SCSAT04")) == 0) {
        // SCSAT04 CONFIGURATION RULE
        if ((GetPrivateProfileString(ruleName.c_str(), _CONV("REDIRECT"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.bRedirect = (type_to_int(buffer, NULL, 10) == 0) ? FALSE : TRUE;
        }
        if ((GetPrivateProfileString(ruleName.c_str(), _CONV("IP"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.IP = buffer;
        }
        if ((GetPrivateProfileString(ruleName.c_str(), _CONV("PORT"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.port = buffer;
        }
        if ((GetPrivateProfileString(ruleName.c_str(), _CONV("MEASURE_APDU"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.measureApduLen = sizeof(m_scsat04Config.measureApdu);
            CCommonFnc::BYTE_ConvertFromHexStringToArray(buffer, m_scsat04Config.measureApdu, &(m_scsat04Config.measureApduLen));
        }
        if ((GetPrivateProfileString(ruleName.c_str(), _CONV("MEASURE_BYTE_COUNTER"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.measureApduByteCounter = type_to_int(buffer, NULL, 10);
        }
        if ((GetPrivateProfileString(ruleName.c_str(), _CONV("MEASURE_BYTE_DELAY"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.measureApduByteDelay = type_to_int(buffer, NULL, 10);
        }
        if ((GetPrivateProfileString(ruleName.c_str(), _CONV("READ_RATIO"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.readRatio = type_to_int(buffer, NULL, 10);
        }
        if ((GetPrivateProfileString(ruleName.c_str(), _CONV("NUM_SAMPLES"), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
            m_scsat04Config.numSamples = type_to_int(buffer, NULL, 10);
        }
        
        
        
    }
    if (compareWithNoCase(ruleName.substr(0, (int) type_length(_CONV("RULE"))).c_str(), _CONV("RULE")) == 0) {
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
            while((GetPrivateProfileString(ruleName.c_str(), valueName.c_str(), _CONV(""), buffer, cBuffer, filePath.c_str())) > 0) {
                ruleString = buffer; ruleString += _CONV(" ");
                
                // FIND HISTORY ELEMENT, WILL BE SAME FOR ALL OTHER ELEMENTAREY RULES
                if ((pos = ruleString.find(_CONV("t="))) != string_type::npos) {
                    //singleRule.history = atoi(ruleString.substr(pos + (int) type_length(_CONV("t="))).c_str());
					singleRule.history = type_to_int(ruleString.substr(pos + (int)type_length(_CONV("t="))).c_str(), NULL, 10);
                    ruleString.erase(pos, ruleString.find(_CONV(";"), pos) - pos + 1); // remove from rule string
                }                    
                // FIND DIRECTION ELEMENT (IN/OUT), WILL BE SAME FOR ALL OTHER ELEMENTAREY RULES
                if ((pos = ruleString.find(_CONV("in="))) != string_type::npos) {
                    singleRule.apduDirection = type_to_int(ruleString.substr(pos + (int) type_length(_CONV("in="))).c_str(), NULL, 10);
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
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("="))+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.matchRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), _CONV("INS")) == 0) {
                        singleRule.element = INS_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("="))+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.matchRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), _CONV("P1")) == 0) {
                        singleRule.element = P1_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("="))+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.matchRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), _CONV("P2")) == 0) {
                        singleRule.element = P2_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("="))+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.matchRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), _CONV("LC")) == 0) {
                        singleRule.element = LC_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("="))+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.matchRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.substr(0, (int) type_length(_CONV("DATA"))).c_str(), _CONV("DATA")) == 0) {
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
                            singleRule.valid = TRUE;rule.matchRules.push_back(singleRule);
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
                    singleRule.apduDirection = type_to_int(ruleString.substr(pos + (int) type_length(_CONV("in="))).c_str(), NULL, 10);
                    //ruleString.Delete(pos, ruleString.find(";", pos) - pos + 1); // remove from rule string
					ruleString.erase(pos, ruleString.find(_CONV(";"), pos) - pos + 1);
                }                    
                pos2 = 0;
                while ((pos = ruleString.find(_CONV(";"), pos2)) != string_type::npos) {
                    rulePart = ruleString.substr(pos2, pos - pos2 + 1);
                    
                    elemName = rulePart.substr(0, rulePart.find(_CONV("=")));
                    
                    if (compareWithNoCase(elemName.c_str(), _CONV("CLA")) == 0) {
                        singleRule.element = CLA_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("="))+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.actionRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), _CONV("INS")) == 0) {
                        singleRule.element = INS_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("="))+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.actionRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), _CONV("P1")) == 0) {
                        singleRule.element = P1_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("="))+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.actionRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), _CONV("P2")) == 0) {
                        singleRule.element = P2_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("="))+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.actionRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), _CONV("LC")) == 0) {
                        singleRule.element = LC_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("="))+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.actionRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.c_str(), _CONV("LE")) == 0) {
                        singleRule.element = LE_ELEM;    
                        CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("="))+1, 2), &(singleRule.value));
                        singleRule.valid = TRUE;rule.actionRules.push_back(singleRule);
                    } 
                    if (compareWithNoCase(elemName.substr(0, (int) type_length(_CONV("DATA"))).c_str(), _CONV("DATA")) == 0) {
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
    lcs     valuesList;
    lcs::iterator   iter;
	string_type filePath;
    
    memset(buffer, 0, cBuffer);
  
    CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("#########################################\n"));
    
    // OBTAIN FULL FILE PATH FOR RULES FILE
    CFile   file;
    if (file.Open(RULE_FILE.c_str(), CFile::modeRead)) {
        filePath = file.GetFilePath();
        file.Close();
        
		string_type message;
    	message = string_format(_CONV("Rules file found: %s\n"), filePath);
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, message);

        // OBTAIN SECTION NAMES
        if ((cReaded = GetPrivateProfileString(NULL, NULL, _CONV(""), buffer, cBuffer, filePath.c_str())) != 0) {
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
        CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("Rules file NOT found\n"));
    }

	WINSCARD_RULES_LOG = string_format(_CONV("%swinscard_rules_log.txt"), m_winscardConfig.sLOG_BASE_PATH);
	WINSCARD_LOG = string_format(_CONV("%swinscard_log.txt"), m_winscardConfig.sLOG_BASE_PATH);
	

    return status;
}

