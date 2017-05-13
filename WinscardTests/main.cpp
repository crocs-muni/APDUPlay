#define CATCH_CONFIG_MAIN

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif


#include "catch.hpp"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <winscard.h>
#include <string>
#include <sstream>
#include "./../Shared/globals.h"


#ifdef __linux__ 
#include <dirent.h>
static void* (*load_func)(void*, const char*) = dlsym;

std::string findRuleFileLinux()
{
	dirp = opendir(".");
	dirent* dp;
	std::string strSearch = "winscard_rules";

	while (dirp) {
		errno = 0;
		if ((dp = readdir(dirp)) != NULL) {
			if (strncmp(dp->d_name, strSearch.c_str(), strSearch.length()) == 0) {
				std::string name = dp->d_name;
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

	return OPEN_ERROR;
}
#else
typedef FARPROC(STDCALL *q) (HMODULE, LPCSTR);
static q load_func = GetProcAddress;

std::string findRuleFileWindows()
{
	std::string strSearch = "./winscard_rules*";
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

std::string findRuleFile()
{
#ifdef __linux__ 
	return findRuleFileLinux();
#else
	return findRuleFileWindows();
#endif
}

using namespace std;
/*#define SCard
#define STDCALL __stdcall*/

static SCard LONG(STDCALL *Original_SCardTransmit)(
	IN SCARDHANDLE hCard,
	IN LPCSCARD_IO_REQUEST pioSendPci,
	IN LPCBYTE pbSendBuffer,
	IN DWORD cbSendLength,
	IN OUT LPSCARD_IO_REQUEST pioRecvPci,
	OUT LPBYTE pbRecvBuffer,
	IN OUT LPDWORD pcbRecvLength
	);

TEST_CASE("Winscard tests", "[winscard_tests]")
{
	SECTION("Aplly rules test")
	{

#ifdef __linux__ 
		void* hOriginal = dlopen("/libpcsclite.so", RTLD_LAZY);
		char *delimeter = ": ";
#else 
		HMODULE hOriginal = LoadLibrary(_CONV("./Winscard.dll"));
		char *delimeter = "";
#endif 
		CHECK(hOriginal != NULL);

#if defined(_WIN32) 
		Original_SCardTransmit =
			(long(STDCALL *)(SCARDHANDLE, LPCSCARD_IO_REQUEST, LPCBYTE, DWORD, LPSCARD_IO_REQUEST, LPBYTE, LPDWORD))
#else
		Original_SCardTransmit =
			(long(STDCALL *)(SCARDHANDLE, LPCSCARD_IO_REQUEST, const unsigned char *, unsigned long, LPSCARD_IO_REQUEST, unsigned char *, unsigned long *))
#endif
	    load_func(hOriginal, "SCardTransmit");

		CHECK(Original_SCardTransmit != NULL);

		ofstream myfile;
		myfile.open("winscard_rules.txt");
		
		myfile << "[WINSCARD]\n";
		myfile << "LOG_EXCHANGED_APDU = 1\n";
		myfile << "MODIFY_APDU_BY_RULES = 1\n";
		myfile << "[RULE1]\n";
		myfile << "MATCH1=in=1,cla=80,ins=ca,p1=9f,p2=17,data0=90 00,\n";
		myfile << "ACTION=in=1,cla=80,ins=cb,p1=9f,p2=17,data0=97 00,\n";
		myfile << "USAGE = 1\n";
		myfile << "APDUIN = 1\n";

		myfile.close();

		SCARD_IO_REQUEST pioRecvPci;

		DWORD dwActiveProtocol, dwSendLength, dwRecvLength;
		BYTE pbRecvBuffer[16];
		BYTE pbSendBuffer[] = { (BYTE)0x80, (BYTE)0xca,(BYTE)0x9f, (BYTE)0x17, (BYTE)0x02 , (BYTE)0x90 , (BYTE)0x00 };

		dwSendLength = sizeof(pbSendBuffer);
		dwRecvLength = sizeof(pbRecvBuffer);

		SCARD_IO_REQUEST pioSendPci;

		pioSendPci.dwProtocol = SCARD_PROTOCOL_T0;
		pioSendPci.cbPciLength = sizeof(pioSendPci);

		LONG ret = Original_SCardTransmit(NULL,
			&pioSendPci,
			pbSendBuffer, dwSendLength,
			NULL, pbRecvBuffer, &dwRecvLength);

		std::string strFile = findRuleFile();

		ifstream rulefile;
		rulefile.open(strFile.c_str());
		string line;
		char* search = "80 cb 9f 17 02 97 00";

		bool found = false;

		unsigned int curLine = 0;
		while (getline(rulefile, line)) {
			if (line.find(search, 0) != string::npos) {
				found = true;
				rulefile.close();
				break;
			}
		}

		CHECK(found == true);
	}
}
