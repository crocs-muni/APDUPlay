#include "stdafx.h"
/*#include "Interface.h"
#include <cstdio>

int nothing()
{
	void *handle;
	char *error;
	hOriginal = dlopen("libpcsclite.so", RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "%s\n", dlerror());
		return FALSE;
	}
	
	dlerror();    // Clear any existing errorz
	
	Original_SCardEstablishContext = (LONG(__stdcall *)(
		IN  DWORD dwScope,
		IN  LPCVOID pvReserved1,
		IN  LPCVOID pvReserved2,
		OUT LPSCARDCONTEXT phContext)) dlsym(hOriginal, "SCardEstablishContext");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardEstablishContext procedure address:  %s\n", error);
		return FALSE;
	}
	
	Original_SCardReleaseContext =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext))
		dlsym(hOriginal, "SCardReleaseContext");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardReleaseContext procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardIsValidContext =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext))
		dlsym(hOriginal, "SCardIsValidContext");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardIsValidContext procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListReaderGroupsA =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			OUT     LPSTR mszGroups,
			IN OUT  LPDWORD pcchGroups))
		dlsym(hOriginal, "SCardListReaderGroupsA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardListReaderGroupsA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListReaderGroupsW =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			OUT     LPWSTR mszGroups,
			IN OUT  LPDWORD pcchGroups))
		dlsym(hOriginal, "SCardListReaderGroupsW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardListReaderGroupsW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListReadersA =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCSTR mszGroups,
			OUT     LPSTR mszReaders,
			IN OUT  LPDWORD pcchReaders))
		dlsym(hOriginal, "SCardListReadersA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardListReadersA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListReadersW =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCWSTR mszGroups,
			OUT     LPWSTR mszReaders,
			IN OUT  LPDWORD pcchReaders))
		dlsym(hOriginal, "SCardListReadersW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardListReadersW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListCardsA =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCBYTE pbAtr,
			IN      LPCGUID rgquidInterfaces,
			IN      DWORD cguidInterfaceCount,
			OUT     LPSTR mszCards,
			IN OUT  LPDWORD pcchCards))
		dlsym(hOriginal, "SCardListCardsA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardListCardsA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListCardsW =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCBYTE pbAtr,
			IN      LPCGUID rgquidInterfaces,
			IN      DWORD cguidInterfaceCount,
			OUT     LPWSTR mszCards,
			IN OUT  LPDWORD pcchCards))
		dlsym(hOriginal, "SCardListCardsW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardListCardsW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListInterfacesA =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCSTR szCard,
			OUT     LPGUID pguidInterfaces,
			IN OUT  LPDWORD pcguidInterfaces))
		dlsym(hOriginal, "SCardListInterfacesA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardListInterfacesA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardListInterfacesW =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCWSTR szCard,
			OUT     LPGUID pguidInterfaces,
			IN OUT  LPDWORD pcguidInterfaces))
		dlsym(hOriginal, "SCardListInterfacesW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardListInterfacesW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetProviderIdA =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCSTR szCard,
			OUT     LPGUID pguidProviderId))
		dlsym(hOriginal, "SCardGetProviderIdA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardGetProviderIdA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetProviderIdW =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCWSTR szCard,
			OUT     LPGUID pguidProviderId))
		dlsym(hOriginal, "SCardGetProviderIdW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardGetProviderIdW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetCardTypeProviderNameA =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szCardName,
			IN DWORD dwProviderId,
			OUT LPSTR szProvider,
			IN OUT LPDWORD pcchProvider))
		dlsym(hOriginal, "SCardGetCardTypeProviderNameA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardGetCardTypeProviderNameA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetCardTypeProviderNameW =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szCardName,
			IN DWORD dwProviderId,
			OUT LPWSTR szProvider,
			IN OUT LPDWORD pcchProvider))
		dlsym(hOriginal, "SCardGetCardTypeProviderNameW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardGetCardTypeProviderNameW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardIntroduceReaderGroupA =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szGroupName))
		dlsym(hOriginal, "SCardIntroduceReaderGroupA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardIntroduceReaderGroupA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardIntroduceReaderGroupW =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szGroupName))
		dlsym(hOriginal, "SCardIntroduceReaderGroupW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardIntroduceReaderGroupW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardForgetReaderGroupA =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szGroupName))
		dlsym(hOriginal, "SCardForgetReaderGroupA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardForgetReaderGroupA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardForgetReaderGroupW =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szGroupName))
		dlsym(hOriginal, "SCardForgetReaderGroupW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardForgetReaderGroupW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardIntroduceReaderA =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szReaderName,
			IN LPCSTR szDeviceName))
		dlsym(hOriginal, "SCardIntroduceReaderA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardIntroduceReaderA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardIntroduceReaderW =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szReaderName,
			IN LPCWSTR szDeviceName))
		dlsym(hOriginal, "SCardIntroduceReaderW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardIntroduceReaderW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardForgetReaderA =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szReaderName))
		dlsym(hOriginal, "SCardForgetReaderA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardForgetReaderA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardForgetReaderW =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szReaderName))
		dlsym(hOriginal, "SCardForgetReaderW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardForgetReaderW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardAddReaderToGroupA =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szReaderName,
			IN LPCSTR szGroupName))
		dlsym(hOriginal, "SCardAddReaderToGroupA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardAddReaderToGroupA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardAddReaderToGroupW =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szReaderName,
			IN LPCWSTR szGroupName))
		dlsym(hOriginal, "SCardAddReaderToGroupW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardAddReaderToGroupW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardRemoveReaderFromGroupA =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szReaderName,
			IN LPCSTR szGroupName))
		dlsym(hOriginal, "SCardRemoveReaderFromGroupA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardRemoveReaderFromGroupA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardRemoveReaderFromGroupW =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szReaderName,
			IN LPCWSTR szGroupName))
		dlsym(hOriginal, "SCardRemoveReaderFromGroupW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardRemoveReaderFromGroupW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardIntroduceCardTypeA =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szCardName,
			IN LPCGUID pguidPrimaryProvider,
			IN LPCGUID rgguidInterfaces,
			IN DWORD dwInterfaceCount,
			IN LPCBYTE pbAtr,
			IN LPCBYTE pbAtrMask,
			IN DWORD cbAtrLen))
		dlsym(hOriginal, "SCardIntroduceCardTypeA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardIntroduceCardTypeA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardIntroduceCardTypeW =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szCardName,
			IN LPCGUID pguidPrimaryProvider,
			IN LPCGUID rgguidInterfaces,
			IN DWORD dwInterfaceCount,
			IN LPCBYTE pbAtr,
			IN LPCBYTE pbAtrMask,
			IN DWORD cbAtrLen))
		dlsym(hOriginal, "SCardIntroduceCardTypeW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardIntroduceCardTypeW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardSetCardTypeProviderNameA =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szCardName,
			IN DWORD dwProviderId,
			IN LPCSTR szProvider))
		dlsym(hOriginal, "SCardSetCardTypeProviderNameA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardSetCardTypeProviderNameA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardSetCardTypeProviderNameW =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szCardName,
			IN DWORD dwProviderId,
			IN LPCWSTR szProvider))
		dlsym(hOriginal, "SCardSetCardTypeProviderNameW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardSetCardTypeProviderNameW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardForgetCardTypeA =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCSTR szCardName))
		dlsym(hOriginal, "SCardForgetCardTypeA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardForgetCardTypeA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardForgetCardTypeW =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCWSTR szCardName))
		dlsym(hOriginal, "SCardForgetCardTypeW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardForgetCardTypeW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardFreeMemory =
		(LONG(__stdcall *)(
			IN SCARDCONTEXT hContext,
			IN LPCVOID pvMem))
		dlsym(hOriginal, "SCardFreeMemory");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardFreeMemory procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardAccessStartedEvent =
		(HANDLE(__stdcall *)(void))
		dlsym(hOriginal, "SCardAccessStartedEvent");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardAccessStartedEvent procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardReleaseStartedEvent =
		(void(__stdcall *)(void))
		dlsym(hOriginal, "SCardReleaseStartedEvent");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardReleaseStartedEvent procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardLocateCardsA =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCSTR mszCards,
			IN OUT  LPSCARD_READERSTATEA rgReaderStates,
			IN      DWORD cReaders))
		dlsym(hOriginal, "SCardLocateCardsA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardLocateCardsA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardLocateCardsW =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCWSTR mszCards,
			IN OUT  LPSCARD_READERSTATEW rgReaderStates,
			IN      DWORD cReaders))
		dlsym(hOriginal, "SCardLocateCardsW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardLocateCardsW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardLocateCardsByATRA =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      LPSCARD_ATRMASK rgAtrMasks,
			IN      DWORD cAtrs,
			IN OUT  LPSCARD_READERSTATEA rgReaderStates,
			IN      DWORD cReaders))
		dlsym(hOriginal, "SCardLocateCardsByATRA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardLocateCardsByATRA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardLocateCardsByATRW =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      LPSCARD_ATRMASK rgAtrMasks,
			IN      DWORD cAtrs,
			IN OUT  LPSCARD_READERSTATEW rgReaderStates,
			IN      DWORD cReaders))
		dlsym(hOriginal, "SCardLocateCardsByATRW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardLocateCardsByATRW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetStatusChangeA =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      DWORD dwTimeout,
			IN OUT  LPSCARD_READERSTATEA rgReaderStates,
			IN      DWORD cReaders))
		dlsym(hOriginal, "SCardGetStatusChangeA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardGetStatusChangeA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetStatusChangeW =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      DWORD dwTimeout,
			IN OUT  LPSCARD_READERSTATEW rgReaderStates,
			IN      DWORD cReaders))
		dlsym(hOriginal, "SCardGetStatusChangeW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardGetStatusChangeW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardCancel =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext))
		dlsym(hOriginal, "SCardCancel");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardCancel procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardConnectA =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCSTR szReader,
			IN      DWORD dwShareMode,
			IN      DWORD dwPreferredProtocols,
			OUT     LPSCARDHANDLE phCard,
			OUT     LPDWORD pdwActiveProtocol))
		dlsym(hOriginal, "SCardConnectA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardConnectA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardConnectW =
		(LONG(__stdcall *)(
			IN      SCARDCONTEXT hContext,
			IN      LPCWSTR szReader,
			IN      DWORD dwShareMode,
			IN      DWORD dwPreferredProtocols,
			OUT     LPSCARDHANDLE phCard,
			OUT     LPDWORD pdwActiveProtocol))
		dlsym(hOriginal, "SCardConnectW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardConnectW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardReconnect =
		(LONG(__stdcall *)(
			IN      SCARDHANDLE hCard,
			IN      DWORD dwShareMode,
			IN      DWORD dwPreferredProtocols,
			IN      DWORD dwInitialization,
			OUT     LPDWORD pdwActiveProtocol))
		dlsym(hOriginal, "SCardReconnect");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardReconnect procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardDisconnect =
		(LONG(__stdcall *)(
			IN      SCARDHANDLE hCard,
			IN      DWORD dwDisposition))
		dlsym(hOriginal, "SCardDisconnect");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardDisconnect procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardBeginTransaction =
		(LONG(__stdcall *)(
			IN      SCARDHANDLE hCard))
		dlsym(hOriginal, "SCardBeginTransaction");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardBeginTransaction procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardEndTransaction =
		(LONG(__stdcall *)(
			IN      SCARDHANDLE hCard,
			IN      DWORD dwDisposition))
		dlsym(hOriginal, "SCardEndTransaction");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardEndTransaction procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardCancelTransaction =
		(LONG(__stdcall *)(
			IN      SCARDHANDLE hCard))
		dlsym(hOriginal, "SCardCancelTransaction");
	if ((error = dlerror()) != NULL) {
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "Could not find SCardCancelTransaction procedure address\n");
	}

	Original_SCardState =
		(LONG(__stdcall *)(
			IN SCARDHANDLE hCard,
			OUT LPDWORD pdwState,
			OUT LPDWORD pdwProtocol,
			OUT LPBYTE pbAtr,
			IN OUT LPDWORD pcbAtrLen))
		dlsym(hOriginal, "SCardState");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardState procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardStatusA =
		(LONG(__stdcall *)(
			IN SCARDHANDLE hCard,
			OUT LPSTR szReaderName,
			IN OUT LPDWORD pcchReaderLen,
			OUT LPDWORD pdwState,
			OUT LPDWORD pdwProtocol,
			OUT LPBYTE pbAtr,
			IN OUT LPDWORD pcbAtrLen))
		dlsym(hOriginal, "SCardStatusA");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardStatusA procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardStatusW =
		(LONG(__stdcall *)(
			IN SCARDHANDLE hCard,
			OUT LPWSTR szReaderName,
			IN OUT LPDWORD pcchReaderLen,
			OUT LPDWORD pdwState,
			OUT LPDWORD pdwProtocol,
			OUT LPBYTE pbAtr,
			IN OUT LPDWORD pcbAtrLen))
		dlsym(hOriginal, "SCardStatusW");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardStatusW procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardTransmit =
		(LONG(__stdcall *)(
			IN SCARDHANDLE hCard,
			IN LPCSCARD_IO_REQUEST pioSendPci,
			IN LPCBYTE pbSendBuffer,
			IN DWORD cbSendLength,
			IN OUT LPSCARD_IO_REQUEST pioRecvPci,
			OUT LPBYTE pbRecvBuffer,
			IN OUT LPDWORD pcbRecvLength))
		dlsym(hOriginal, "SCardTransmit");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardTransmit procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardControl =
		(LONG(__stdcall *)(
			IN      SCARDHANDLE hCard,
			IN      DWORD dwControlCode,
			IN      LPCVOID lpInBuffer,
			IN      DWORD nInBufferSize,
			OUT     LPVOID lpOutBuffer,
			IN      DWORD nOutBufferSize,
			OUT     LPDWORD lpBytesReturned))
		dlsym(hOriginal, "SCardControl");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardControl procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardGetAttrib =
		(LONG(__stdcall *)(
			IN SCARDHANDLE hCard,
			IN DWORD dwAttrId,
			OUT LPBYTE pbAttr,
			IN OUT LPDWORD pcbAttrLen))
		dlsym(hOriginal, "SCardGetAttrib");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardGetAttrib procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardSetAttrib =
		(LONG(__stdcall *)(
			IN SCARDHANDLE hCard,
			IN DWORD dwAttrId,
			IN LPCBYTE pbAttr,
			IN DWORD cbAttrLen))
		dlsym(hOriginal, "SCardSetAttrib");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "Could not find SCardSetAttrib procedure address:  %s\n", error);
		return FALSE;
	}

	Original_SCardUIDlgSelectCardA =
		(LONG(__stdcall *)(
			LPOPENCARDNAMEA_EX))
		dlsym(hOriginal, "SCardUIDlgSelectCardA");
	if ((error = dlerror()) != NULL) {
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "Could not find SCardUIDlgSelectCardA procedure address\n");
	}

	Original_SCardUIDlgSelectCardW =
		(LONG(__stdcall *)(
			LPOPENCARDNAMEW_EX))
		dlsym(hOriginal, "SCardUIDlgSelectCardW");
	if ((error = dlerror()) != NULL) {
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "Could not find SCardUIDlgSelectCardW procedure address\n");
	}

	Original_GetOpenCardNameA =
		(LONG(__stdcall *)(LPOPENCARDNAMEA))
		dlsym(hOriginal, "GetOpenCardNameA");
	if ((error = dlerror()) != NULL) {
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "Could not find GetOpenCardNameA procedure address\n");
	}

	Original_GetOpenCardNameW =
		(LONG(__stdcall *)(LPOPENCARDNAMEW))
		dlsym(hOriginal, "GetOpenCardNameW");
	if ((error = dlerror()) != NULL) {
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "Could not find GetOpenCardNameW procedure address\n");
	}

	Original_SCardDlgExtendedError =
		(LONG(__stdcall *)(void))
		dlsym(hOriginal, "SCardDlgExtendedError ");
	if ((error = dlerror()) != NULL) {
		CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, "Could not find SCardDlgExtendedError procedure address\n");
	}

	dlclose(handle);
	return 0;

	
}*/
