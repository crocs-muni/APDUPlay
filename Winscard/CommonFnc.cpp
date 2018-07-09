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


#include "stdafx.h"
#include "CommonFnc.h"
#include <math.h>
#include <memory>
#include <fstream>
#include <cstring>
#include <iostream>
#include <chrono>

#if defined (_WIN32)
#include <filesystem>
#endif

string_type getCurrentTimeString()
{
	time_t end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::string temp = ctime(&end_time);
	std::replace(temp.begin(), temp.end(), ' ', '-');
	std::replace(temp.begin(), temp.end(), ':', '_');
	temp.erase(std::remove(temp.begin(), temp.end(), '\n'), temp.end());
	
	return string_type(temp.begin(), temp.end());
}

size_t compareWithNoCase(const char_type* str1, const char_type* str2) {
	
	if (type_length(str1) != type_length(str2))
	{
#if defined (_WIN32)
		return max(type_length(str1), type_length(str2));
#else
		return std::max(type_length(str1), type_length(str2));
#endif
	}

	char_type *str1_2 = new char_type[type_length(str1) + 1];
	char_type *str2_2 = new char_type[type_length(str2) + 1];

	for (size_t i = 0; i <= type_length(str1); ++i)
	{
#if defined (_WIN32) && defined (UNICODE)
		str1_2[i] = towupper(str1[i]);
		str2_2[i] = towupper(str2[i]);
#else
		str1_2[i] = toupper(str1[i]);
		str2_2[i] = toupper(str2[i]);
#endif
	}

	int result = type_compare(str1_2, str2_2);

	delete[] str1_2;
	delete[] str2_2;
	return result;
}

/*int CCommonFnc::File_AppendString(CString filePath, CString data) {
    int             status = STAT_OK;
    CFile           file;

    if (file.Open(filePath, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate)) {
        file.SeekToEnd();
        file.Write((LPCTSTR) data, data.GetLength());
        file.Close();
    }
    else status = STAT_FILE_OPEN_FAIL;   

    return status;

}*/

/*int CCommonFnc::File_AppendString(string_type filePath, string_type data) {
	int             status = STAT_OK;
	ofstream_type file;
	file.open(filePath, std::fstream::out | std::fstream::app);

	if (file.is_open()) {
		file.write((LPCTSTR)data.c_str(), data.length());
		file.close();
	}
	else {
        fprintf(stderr, "could not open the file with path: %s\n", filePath.c_str());
        status = STAT_FILE_OPEN_FAIL;
    }

	return status;
}*/


int CCommonFnc::File_AppendString(string_type filePath, std::string data) {
	int             status = STAT_OK;
	ofstream_type file;
	file.open(filePath, std::fstream::out | std::fstream::app);

	if (file.is_open()) {
		file.write((LPCTSTR)data.c_str(), data.length());
		file.close();
	}
	else {
		fprintf(stderr, "could not open the file with path: %s\n", filePath.c_str());
		status = STAT_FILE_OPEN_FAIL;
	}

	return status;
}


int CCommonFnc::File_AppendString(string_type filePath, std::wstring data) {
	int             status = STAT_OK;
	ofstream_type file;
	file.open(filePath, std::fstream::out | std::fstream::app);

	if (file.is_open()) {
		file.write((LPCTSTR)data.c_str(), data.length());
		file.close();
	}
	else {
		fprintf(stderr, "could not open the file with path: %s\n", filePath.c_str());
		status = STAT_FILE_OPEN_FAIL;
	}

	return status;
}

#if defined(_WIN32)
int CCommonFnc::File_GetAvailableFileName(string_type baseFile, string_type* pFreeFileName) {
    int             status = STAT_OK;
    char_type            fileName[MAX_PATH];
	string_type          fileNameStr;
	char_type            drive[_MAX_DRIVE];
	char_type            dir[_MAX_DIR];
	char_type            fname[_MAX_FNAME];
	char_type            ext[_MAX_EXT];
	const char_type* help;
    DWORD           index = 1;

    // FIND FIRST FREE INDEX
	type_path_split(baseFile.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
    index = 1;
	string_type zeroes = _CONV("0000");
    //sprintf_s(fileName, MAX_PATH, "%s%s%s%s%d%s", drive, dir, fname, (LPCTSTR) zeroes.c_str(), index, ext);
	fileNameStr = string_format(_CONV("%s%s%s%s%d%s"), drive, dir, fname, (LPCTSTR)zeroes.c_str(), index, ext);
	help = fileNameStr.c_str();
	type_copy(fileName, help);
    while (GetFileAttributes(fileName) != -1) {
        index++;
        if (index < 10) zeroes = _CONV("0000");
        else if (index < 100) zeroes = _CONV("000");
        else if (index < 1000) zeroes = _CONV("00");
        else if (index < 10000) zeroes = _CONV("0");
        else zeroes = _CONV("");
        //sprintf_s(fileName, MAX_PATH, "%s%s%s%s%d%s", drive, dir, fname, (LPCTSTR) zeroes.c_str(), index, ext);
		fileNameStr = string_format(_CONV("%s%s%s%s%d%s"), drive, dir, fname, (LPCTSTR)zeroes.c_str(), index, ext);
		help = fileNameStr.c_str();
		type_copy(fileName, help);
    }    
    // INDEX FOUND
    //pFreeFileName->Format("%s%s%s%s%d%s", drive, dir, fname, (LPCTSTR) zeroes.c_str(), index, ext);
	*pFreeFileName = string_format(_CONV("%s%s%s%s%d%s"), drive, dir, fname, (LPCTSTR)zeroes.c_str(), index, ext);

    return status;
}
#endif

#if defined(_WIN32)
int CCommonFnc::File_SaveMatrixIntFileOffset(size_t startFileOffset, string_type filePath, INT_DATA_BLOB* pBlob, size_t startOffset, size_t endOffset, BOOL bSaveBinary) {
    return File_SaveMatrixInt(filePath, pBlob, startOffset, endOffset, startFileOffset, bSaveBinary);
}

int CCommonFnc::File_SaveMatrixInt(string_type filePath, INT_DATA_BLOB* pBlob, size_t startOffset, size_t endOffset, size_t startFileOffset, BOOL bSaveBinary) {
    int     status = STAT_OK;
	size_t     i;
	fstream_type file;
	string_type value;
	string_type values = _CONV("");
    
#define		NUM_VALUES_PER_WRITE	1000    
    
    if (endOffset == -1) endOffset = pBlob->dwActLen;
    
	file.open(filePath, std::fstream::in | std::fstream::out | std::fstream::app);


    if (file.is_open()) {
        file.seekg(startFileOffset, std::ios_base::cur);
        
        if (bSaveBinary) {
			// SAVE AS BINARY CHUNK
			file.write((LPTSTR)(pBlob->pData + startOffset), (endOffset - startOffset) * sizeof(int));
        }
        else {
			for (i = startOffset; i < endOffset; i++) { 
				//value.Format("%d\r\n", pBlob->pData[i]);
				value = string_format(_CONV("%d\r\n"), pBlob->pData[i]);
				values += value;
	            
				if ((i % NUM_VALUES_PER_WRITE) == 0) {
					file.write((LPCTSTR) values.c_str(), values.length());
					values = _CONV("");
				}
			}
	        
			// WRITE REMAINING VALUES
			file.write((LPCTSTR) values.c_str(), values.length());
		}
        file.close();
    }
    else {
        status = STAT_FILE_OPEN_FAIL;
    }

    return status;
}
#endif

int CCommonFnc::BYTE_ConvertFromHexStringToArray(string_type hexaString, BYTE* pArray, BYTE* pbArrayLen) {
    int     status = STAT_OK;
    DWORD   arrayLen = *pbArrayLen;    
    
    status = BYTE_ConvertFromHexStringToArray(hexaString, pArray, &arrayLen);
    if (arrayLen > 0xFF) status = STAT_NOT_ENOUGHT_DATA_TYPE;
    else *pbArrayLen = (BYTE) arrayLen;

    return status;
}

int CCommonFnc::BYTE_ConvertFromHexStringToArray(string_type hexaString, BYTE* pArray, DWORD* pbArrayLen) {
    int         status = STAT_OK;
    size_t       pos = 0;
	size_t       pos2 = 0;
	string_type     hexNum;
	size_t       num;
    BYTE*       pTempArray = NULL;
	size_t       tempArrayPos = 0;

    // Trim leading and trailing spaces
	hexaString.erase(hexaString.find_last_not_of(_CONV(" ")) + 1);
	size_t startpos = hexaString.find_first_not_of(_CONV(" "));
	if (string_type::npos != startpos) {
		hexaString = hexaString.substr(startpos);
	}

	// Replace spaces inside hex string
	std::string hexaStringNoSpace;
	for (size_t i = 0; i < hexaString.size(); i++) {
		if (hexaString.at(i) != ' ') {
			hexaStringNoSpace.insert(hexaStringNoSpace.size(), 1, hexaString.at(i));
		}
	}


    if (status == STAT_OK) {
        pTempArray = new BYTE[hexaStringNoSpace.length()];
        memset(pTempArray, 0, hexaStringNoSpace.length());

        pos = 0;
		while (pos < hexaStringNoSpace.length()) {
			hexNum = hexaStringNoSpace.substr(pos, 2);
			if (hexNum.length() > 0) {
				num = type_to_int((LPCTSTR)hexNum.c_str(), NULL, 16);

				if (num == 0xFF) pTempArray[tempArrayPos] = 0xFF;
				else pTempArray[tempArrayPos] = (BYTE)num & 0xFF;

				tempArrayPos++;
			}
			pos += 2;
		}

        if (tempArrayPos > *pbArrayLen) {
            status = STAT_NOT_ENOUGHT_MEMORY;
        }  
        else {
            memcpy(pArray, pTempArray, tempArrayPos);
        }
        *pbArrayLen = (DWORD) tempArrayPos;

        if (pTempArray) delete[] pTempArray;
    }
    return status;
}

int CCommonFnc::BYTE_ConvertFromHexNumToByte(string_type hexNum, BYTE* pByte) {
    DWORD num = type_to_int((LPCTSTR) hexNum.c_str(), NULL, 16);

    if (num == 0xFF) *pByte = 0xFF;
    else *pByte = (BYTE) num & 0xFF;
    
    return STAT_OK;
}

int CCommonFnc::APDU_ConvertToString(CARDAPDU* pAPDU, string_type* pString, BOOL toSendAPDU) {
    int         status = STAT_OK;
	string_type     message;
	string_type     ioData;
    
    if (toSendAPDU) {
        // APDU to SmartCard
        CCommonFnc::BYTE_ConvertFromArrayToHexString(pAPDU->DataIn, pAPDU->lc, &ioData);
        
        // FORMAT: INS CLA P1 P2 LC input_data Le 
        //message.Format("-> %.2x %.2x %.2x %.2x %.2x %s %.2x", pAPDU->cla, pAPDU->ins, pAPDU->p1, pAPDU->p2, pAPDU->lc, (LPCTSTR) ioData, pAPDU->le);
		message = string_format(_CONV("-> %.2x %.2x %.2x %.2x %.2x %s %.2x"), pAPDU->cla, pAPDU->ins, pAPDU->p1, pAPDU->p2, pAPDU->lc, (LPCTSTR)ioData.c_str(), pAPDU->le);
    }
    else {
        // APDU from SmartCard
           
        CCommonFnc::BYTE_ConvertFromArrayToHexString(pAPDU->DataOut, pAPDU->le, &ioData);
        
        // FORMAT: output_data SW
        //message.Format("<- %s %.2x %.2x", (LPCTSTR) ioData, HIGHBYTE(pAPDU->sw), LOWBYTE(pAPDU->sw));
		message = string_format(_CONV("<- %s %.2x %.2x"), (LPCTSTR)ioData.c_str(), HIGHBYTE(pAPDU->sw), LOWBYTE(pAPDU->sw));
    }

    *pString = message;

    return status;
}

int CCommonFnc::BYTE_ConvertFromArrayToHexString(const BYTE* pArray, DWORD pbArrayLen, string_type* pHexaString) {
    int         status = STAT_OK;
	string_type     hexNum;
    DWORD       i;

    *pHexaString = _CONV("");
    for (i = 0; i < pbArrayLen; i++) {
        //hexNum.Format("%.2x", pArray[i]);
		hexNum = string_format(_CONV("%.2x"), pArray[i]);
        hexNum += _CONV(" ");

        *pHexaString += hexNum;
    }

    //pHexaString->TrimRight(" ");
	pHexaString->erase(pHexaString->find_last_not_of(_CONV(" ")) + 1);

    return status;
}

int CCommonFnc::String_ParseNullSeparatedArray(BYTE* array, DWORD arraySize, ls* pValueString) {
    int     status = STAT_OK;
    DWORD   pos;
	std::string itemName;

	pos = 0;
	itemName = "";
	
	if (arraySize > 0) {
		while ((array[pos] != '\0' || array[pos + 1] != '\0') && pos <= (arraySize - 1)) {	// -1 belong to special end zero
			if (array[pos] == '\0') {	// end of one item
				if (itemName != "") pValueString->push_back(itemName);

				itemName = "";
			} 
			else itemName += array[pos];

			pos++;

		}	//end while
	}

    return status;
}

int CCommonFnc::String_ParseNullSeparatedArray(WCHAR* array, DWORD arraySize, lws* pValueString) {
    int     status = STAT_OK;
    DWORD   pos;
	std::wstring itemName;

	pos = 0;
	itemName = L"";
	
	if (arraySize > 0) {
		while ((array[pos] != L'\0' || array[pos + 1] != L'\0') && pos <= (arraySize - 1)) {	// -1 belong to special end zero
			if (array[pos] == L'\0') {	// end of one item
				if (itemName.length() != 0) pValueString->push_back(itemName);

				itemName = L"";
			} 
			else itemName += array[pos];

			pos++;

		}	//end while
	}

    return status;
}
