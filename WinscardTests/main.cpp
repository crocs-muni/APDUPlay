#define CATCH_CONFIG_MAIN

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif


#include "catch.hpp"
#if defined(_WIN32)
#include <windows.h>
#endif
#include <iostream>
#include <fstream>
#include <winscard.h>
#include <string>
#include <sstream>
#include "./../Shared/globals.h"

const size_t MAX_APDU_LEN = 300;

std::string GetLogsPath()
{
	return "./";
}


#ifdef __linux__
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <pwd.h>

static void* (*load_func)(void*, const char*) = dlsym;

std::string FindlogFileLinux(std::string strSearch)
{
	DIR* dirp = opendir(GetLogsPath().c_str());
	dirent* dp;

	while (dirp) {
		errno = 0;
		if ((dp = readdir(dirp)) != NULL) {
			if (strncmp(dp->d_name, strSearch.c_str(), strSearch.length()) == 0) {
				std::string name = dp->d_name;
                printf("found %s\n", name.c_str());
				closedir(dirp);
				return name;
			}
		}
		else {
			if (errno == 0) {
				closedir(dirp);
				return "";
			}
			closedir(dirp);
			return "";
		}
	}

	return "";
}
#else
typedef FARPROC(STDCALL *q) (HMODULE, LPCSTR);
static q load_func = GetProcAddress;

std::string FindlogFileWindows(std::string strSearch)
{
	strSearch = "./" + strSearch + "*";
	WIN32_FIND_DATAA ffd;
	HANDLE hFind = FindFirstFileA(strSearch.c_str(), &ffd);
	std::string strFile = "";

	do
	{
		strFile = ffd.cFileName;
	} while (FindNextFileA(hFind, &ffd) != 0);
	return strFile;
}
#endif

std::string FindlogFile(std::string strSearch)
{
#ifdef __linux__ 
	return FindlogFileLinux(strSearch);
#else
	return FindlogFileWindows(strSearch);
#endif
}

using namespace std;
/*#define SCard
#define STDCALL __stdcall*/

std::vector<char> HexToBytes(const std::string& hex) {
	// Remove spaces (if any)
	std::string hex2;
	for (size_t i = 0; i < hex.size(); i++) {
		if (hex.at(i) != ' ') {
			hex2.insert(hex2.size(), 1, hex.at(i));
		}
	}

	std::vector<char> bytes;
	for (unsigned int i = 0; i < hex2.length(); i += 2) {
		std::string byteString = hex2.substr(i, 2);
		char byte = (char)strtol(byteString.c_str(), NULL, 16);
		bytes.push_back(byte);
	}

	return bytes;
}

void HexToBytes(const std::string& hex, BYTE dataArray[], size_t* dataArrayLen) {
	std::vector<char> converted = HexToBytes(hex);
	if (converted.size() > *dataArrayLen) {
		return;
	}
	for (size_t i = 0; i < converted.size(); i++) {
		dataArray[i] = converted[i];
	}
	*dataArrayLen = converted.size();
}

std::string BytesToHex(BYTE* data, size_t dataLen) {
	std::stringstream ss;
	char oneByte[3];
	for (size_t i = 0; i < dataLen; ++i) {
		sprintf_s(oneByte, sizeof(oneByte), "%.2x", data[i]);
		ss << oneByte;
	}
	return ss.str();
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

static SCard LONG(STDCALL *Original_SCardConnect)(
	IN		SCARDCONTEXT hContext,
	IN		LPCSTR szReader,
	IN		DWORD dwShareMode,
	IN		DWORD dwPreferredProtocols,
	OUT		LPSCARDHANDLE phCard,
	OUT		LPDWORD pdwActiveProtocol);

static SCard LONG(STDCALL *Original_SCardListReaders)(
	IN      SCARDCONTEXT hContext,
	IN      LPCSTR mszGroups,
	OUT     LPSTR mszReaders,
	IN OUT  LPDWORD pcchReaders
	);
static SCard LONG(STDCALL *Original_SCardListReadersW)(
	IN      SCARDCONTEXT hContext,
	IN      LPCWSTR mszGroups,
	OUT     LPWSTR mszReaders,
	IN OUT  LPDWORD pcchReaders
	);
static SCard LONG(STDCALL *Original_SCardEstablishContext)(
	IN  DWORD dwScope,
	IN  LPCVOID pvReserved1,
	IN  LPCVOID pvReserved2,
	OUT LPSCARDCONTEXT phContext
	);
static SCard LONG(STDCALL *Original_SCardReleaseContext)(
	IN  SCARDCONTEXT hContext
	);

int SendAPDU(string_type apdu, SCARDHANDLE  hCard, DWORD scProtocol) {
	int status = 0;
	size_t dwSendLength = MAX_APDU_LEN;
	DWORD dwRecvLength = MAX_APDU_LEN;
	BYTE pbRecvBuffer[MAX_APDU_LEN];
	BYTE pbSendBuffer[MAX_APDU_LEN];
	HexToBytes(apdu, pbSendBuffer, &dwSendLength);

	SCARD_IO_REQUEST pioSendPci;
	pioSendPci.dwProtocol = scProtocol;
	pioSendPci.cbPciLength = sizeof(pioSendPci);

	cout << "Sending APDU: ";
	cout << BytesToHex(pbSendBuffer, dwSendLength) << endl;
	status = Original_SCardTransmit(hCard, &pioSendPci, pbSendBuffer, (DWORD)dwSendLength, NULL, pbRecvBuffer, &dwRecvLength);
	CHECK(status == SCARD_S_SUCCESS);
	cout << "Received response: ";
	cout << BytesToHex(pbRecvBuffer, dwRecvLength) << endl;

	cout << endl;

	return status;
}

// Loads all function ptrs from dll into global variables for use in tests
void LoadFunctionPtrs() {

	//
	// Load required functions from dll
	//

#ifdef __linux__
	void* hOriginal = dlopen("./../../cmake-build-debug/libpcsclite.so", RTLD_LAZY);

#else 
	HMODULE hOriginal = LoadLibrary(_CONV("./Winscard.dll"));
#endif 
	CHECK(hOriginal != NULL);

	Original_SCardEstablishContext = (long(STDCALL *)(DWORD, LPCVOID, LPCVOID, LPSCARDCONTEXT))
		load_func(hOriginal, "SCardEstablishContext");
	CHECK(Original_SCardEstablishContext != NULL);

	Original_SCardTransmit = (long(STDCALL *)(SCARDHANDLE, LPCSCARD_IO_REQUEST, LPCBYTE, DWORD, LPSCARD_IO_REQUEST, LPBYTE, LPDWORD))
		load_func(hOriginal, "SCardTransmit");
	CHECK(Original_SCardTransmit != NULL);

	Original_SCardConnect = (long(STDCALL *)(SCARDCONTEXT, LPCSTR, DWORD, DWORD, LPSCARDHANDLE, LPDWORD))
		load_func(hOriginal, "SCardConnectA");
	DWORD error = GetLastError();
	CHECK(Original_SCardConnect != NULL);

	Original_SCardListReaders = (long(STDCALL *)(SCARDCONTEXT, LPCSTR, LPSTR, LPDWORD))
		load_func(hOriginal, "SCardListReadersA");
	CHECK(Original_SCardListReaders != NULL);

}

TEST_CASE("Winscard tests", "[winscard_tests]")
{
	SECTION("Aplly rules test")
	{
		LoadFunctionPtrs();

		// Prepare configuration files
        ofstream myfile;
        std::string ruleFilePath = GetLogsPath() + "winscard_rules.txt";
        myfile.open(ruleFilePath);

        myfile << "[WINSCARD]\n";
        myfile << "LOG_EXCHANGED_APDU = 1\n";
        myfile << "MODIFY_APDU_BY_RULES = 1\n";
		myfile << "LOG_FUNCTIONS_CALLS = 1\n";
		myfile << "AUTO_REQUEST_DATA = 1\n";
		myfile << "LOG_BASE_PATH = ./\n";
		myfile << "VIRTUAL_READERS = Simona13,Simona2 \n";
		myfile << "\n";
		myfile << "[RULE1]\n";
        myfile << "MATCH1=in=1,cla=80,ins=ca,p1=9f,p2=17,data0=90 00,\n";
        myfile << "ACTION=in=1,cla=80,ins=cb,p1=9f,p2=17,data0=97 00,\n";
        myfile << "USAGE = 1\n";
        myfile << "APDUIN = 1\n";
		myfile << "\n";

		myfile << "[REMOTE]\n";
		myfile << "REDIRECT = 1\n";
		myfile << "IP = 127.0.0.1\n";
		myfile << "PORT = 4001\n";

		myfile.close();


		//
		// Connect to card
		//
		LONG		 status = SCARD_S_SUCCESS;
		SCARDCONTEXT cardContext;
		SCARDHANDLE  hCard;
		DWORD		 scProtocol;
		const size_t READERS_LEN = 1000;
		char         readers[READERS_LEN];
		DWORD		 len = READERS_LEN;

		// SCard context
		status = Original_SCardEstablishContext(SCARD_SCOPE_USER, 0, 0, &cardContext);
		CHECK(status == SCARD_S_SUCCESS);

		// Print available readers
		memset(readers, 0, READERS_LEN);
		status = Original_SCardListReaders(cardContext, NULL, (char*) &readers, &len);
		CHECK(status == SCARD_S_SUCCESS);
		if (status == SCARD_S_SUCCESS) {
			size_t pos = 0;
			while (pos < len) {
				cout << readers + pos << endl;
				pos += strlen(readers + pos) + 1;
			}
		}
		else {
			cout << "SCardListReaders failed, no print readers";
		}
		cout << endl;

		// Connect to specific reader
		string_type remoteReader = "Simona /111.222.123.033@07";
		cout << "Connecting to reader '" << remoteReader << "' ... ";
		status = Original_SCardConnect(cardContext, remoteReader.c_str(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &scProtocol);
		CHECK(status == SCARD_S_SUCCESS);
		if (status == SCARD_S_SUCCESS) cout << "success"; 
		else cout << "fail";
		cout << endl << endl;

		// Send APDU
		//SendAPDU("80 cb 9f 17 02 97 00", hCard, scProtocol);

		//SendAPDU("00 a4 00 00 00", hCard, scProtocol);

		SendAPDU("11 00 00 00 00", hCard, scProtocol);

		SendAPDU("22 00 00 00 00", hCard, scProtocol);

		SendAPDU("33 00 00 00 00", hCard, scProtocol);

		//
		// Verify expected content of resulting files
		//
		std::string strFile = GetLogsPath() + FindlogFile("winscard_rules_log");

		ifstream logFile;
		logFile.open(strFile.c_str());

        CHECK(logFile.is_open());

		string line;
		char* search = "80 cb 9f 17 02 97 00";

		bool found = false;

		unsigned int curLine = 0;
		while (getline(logFile, line)) {
			if (line.find(search, 0) != string::npos) {
				found = true;
				break;
			}
		}

        logFile.close();
		CHECK(found == true);
        std::remove(strFile.c_str());
        std::remove(ruleFilePath.c_str());
        std::remove((GetLogsPath() + FindlogFile("winscard_log")).c_str());
	}

	//char c;
	//std::cin >> c;
}
