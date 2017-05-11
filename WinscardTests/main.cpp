#include <windows.h>
#include <iostream>
#include <fstream>
#include <winscard.h>
#include <string>
#include <sstream>
#include <assert.h>

using namespace std;
#define SCard
#define STDCALL __stdcall

static SCard LONG(STDCALL *Original_SCardTransmit)(
	IN SCARDHANDLE hCard,
	IN LPCSCARD_IO_REQUEST pioSendPci,
	IN LPCBYTE pbSendBuffer,
	IN DWORD cbSendLength,
	IN OUT LPSCARD_IO_REQUEST pioRecvPci,
	OUT LPBYTE pbRecvBuffer,
	IN OUT LPDWORD pcbRecvLength
	);

int main()
{
	HINSTANCE winscardDLL = LoadLibrary("./Winscard.dll");

	if (!winscardDLL) {
		std::cout << "could not load the dynamic library" << std::endl;
		return EXIT_FAILURE;
	}

	Original_SCardTransmit =
		(long(STDCALL *)(SCARDHANDLE, LPCSCARD_IO_REQUEST, LPCBYTE, DWORD, LPSCARD_IO_REQUEST, LPBYTE, LPDWORD))
		GetProcAddress(winscardDLL, "SCardTransmit");

	if (!Original_SCardTransmit) {
		std::cout << "could not locate the function" << std::endl;
		return EXIT_FAILURE;
	}

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

	std::string strSearch = "./winscard_rules*";
	WIN32_FIND_DATAA ffd;
	HANDLE hFind = FindFirstFileA(strSearch.c_str(), &ffd);
	std::string strFile = "";

	do
	{
		strFile = ffd.cFileName;
	} while (FindNextFileA(hFind, &ffd) != 0);

	ifstream rulefile;
	rulefile.open(strFile.c_str());
	string line;
	char* search = "80 cb 9f 17 02 97 00";
	
	unsigned int curLine = 0;
	while (getline(rulefile, line)) { 
		if (line.find(search, 0) != string::npos) {
			rulefile.close();
			return EXIT_SUCCESS;
		}
	}

	return EXIT_FAILURE;

}