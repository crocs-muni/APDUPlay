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

int CCommonFnc::File_AppendString(std::string filePath, std::string data) {
	int             status = STAT_OK;
	std::ofstream file;
	file.open(filePath, std::fstream::out | std::fstream::app);

	if (file.is_open()) {
		file.write((LPCTSTR)data.c_str(), data.length());
		file.close();
	}
	else status = STAT_FILE_OPEN_FAIL;

	return status;
}

int CCommonFnc::File_GetAvailableFileName(string baseFile, string* pFreeFileName) {
    int             status = STAT_OK;
    char            fileName[MAX_PATH];
    char            drive[_MAX_DRIVE];
    char            dir[_MAX_DIR];
    char            fname[_MAX_FNAME];
    char            ext[_MAX_EXT];
    DWORD           index = 1;

    // FIND FIRST FREE INDEX
    _splitpath_s((LPCTSTR) baseFile.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
    index = 1;
    string zeroes = "0000";
    sprintf_s(fileName, MAX_PATH, "%s%s%s%s%d%s", drive, dir, fname, (LPCTSTR) zeroes.c_str(), index, ext);
    while (GetFileAttributes(fileName) != -1) {
        index++;
        if (index < 10) zeroes = "0000";
        else if (index < 100) zeroes = "000";
        else if (index < 1000) zeroes = "00";
        else if (index < 10000) zeroes = "0";
        else zeroes = "";
        sprintf_s(fileName, MAX_PATH, "%s%s%s%s%d%s", drive, dir, fname, (LPCTSTR) zeroes.c_str(), index, ext);
    }    
    // INDEX FOUND
    //pFreeFileName->Format("%s%s%s%s%d%s", drive, dir, fname, (LPCTSTR) zeroes.c_str(), index, ext);
	*pFreeFileName = string_format("%s%s%s%s%d%s", drive, dir, fname, (LPCTSTR)zeroes.c_str(), index, ext);

    return status;
}

int CCommonFnc::File_SaveMatrixIntFileOffset(int startFileOffset, string filePath, INT_DATA_BLOB* pBlob, int startOffset, int endOffset, BOOL bSaveBinary) {
    return File_SaveMatrixInt(filePath, pBlob, startOffset, endOffset, startFileOffset, bSaveBinary);
}

int CCommonFnc::File_SaveMatrixInt(string filePath, INT_DATA_BLOB* pBlob, int startOffset, int endOffset, int startFileOffset, BOOL bSaveBinary) {
    int     status = STAT_OK;
    int     i;
	fstream file;
    string value;
    string values = "";
    
#define		NUM_VALUES_PER_WRITE	1000    
    
    if (endOffset == -1) endOffset = pBlob->dwActLen;
    
	file.open(filePath, std::fstream::in | std::fstream::out | std::fstream::app);


    if (file.is_open()) {
        file.seekg(startFileOffset, ios_base::cur);
        
        if (bSaveBinary) {
			// SAVE AS BINARY CHUNK
			size_t size = (endOffset - startOffset) * sizeof(int);
			char* buff = new char[size];
			memcpy(buff, pBlob->pData + startOffset, size);
			file.write(buff, size);
        }
        else {
			for (i = startOffset; i < endOffset; i++) { 
				//value.Format("%d\r\n", pBlob->pData[i]);
				value = string_format("%d\r\n", pBlob->pData[i]);
				values += value;
	            
				if ((i % NUM_VALUES_PER_WRITE) == 0) {
					file.write((LPCTSTR) values.c_str(), values.length());
					values = "";
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

int CCommonFnc::BYTE_ConvertFromHexStringToArray(string hexaString, BYTE* pArray, BYTE* pbArrayLen) {
    int     status = STAT_OK;
    DWORD   arrayLen = *pbArrayLen;    
    
    status = BYTE_ConvertFromHexStringToArray(hexaString, pArray, &arrayLen);
    if (arrayLen > 0xFF) status = STAT_NOT_ENOUGHT_DATA_TYPE;
    else *pbArrayLen = (BYTE) arrayLen;

    return status;
}

int CCommonFnc::BYTE_ConvertFromHexStringToArray(string hexaString, BYTE* pArray, DWORD* pbArrayLen) {
    int         status = STAT_OK;
    DWORD       pos = 0;
    DWORD       pos2 = 0;
    string     hexNum;
    DWORD       num;
    BYTE*       pTempArray = NULL;
    DWORD       tempArrayPos = 0;

    // EAT SPACES
    //hexaString.TrimLeft(); hexaString.TrimRight();
	hexaString.erase(hexaString.find_last_not_of(" ") + 1);
	size_t startpos = hexaString.find_first_not_of(" ");
	if (string::npos != startpos) {
		hexaString = hexaString.substr(startpos);
	}
    hexaString += " ";
    hexaString.length();

    if (status == STAT_OK) {
        pTempArray = new BYTE[hexaString.length()];
        memset(pTempArray, 0, hexaString.length());

        pos = pos2 = 0;
        /*while ((pos = hexaString.Find(' ', pos2)) != -1) {
            hexNum = hexaString.Mid(pos2, pos - pos2);
            hexNum.TrimLeft(); hexNum.TrimRight();
            if (hexNum.GetLength() > 0) {
                num = strtol((LPCTSTR) hexNum, NULL, 16);
        
                if (num == 0xFF) pTempArray[tempArrayPos] = 0xFF;
                else pTempArray[tempArrayPos] = (BYTE) num & 0xFF;
                
                tempArrayPos++;
            }
            pos2 = pos + 1;
        }*/
		while ((pos = hexaString.find(' ', pos2)) != string::npos) {
			hexNum = hexaString.substr((pos2, pos - pos2));
			hexNum.erase(hexNum.find_last_not_of(" ") + 1);
			size_t startpos2 = hexNum.find_first_not_of(" ");
			if (string::npos != startpos2) {
				hexNum = hexNum.substr(startpos2);
			}
			if (hexNum.length() > 0) {
				num = strtol((LPCTSTR)hexNum.c_str(), NULL, 16);

				if (num == 0xFF) pTempArray[tempArrayPos] = 0xFF;
				else pTempArray[tempArrayPos] = (BYTE)num & 0xFF;

				tempArrayPos++;
			}
			pos2 = pos + 1;
		}

        if (tempArrayPos > *pbArrayLen) {
            status = STAT_NOT_ENOUGHT_MEMORY;
        }  
        else {
            memcpy(pArray, pTempArray, tempArrayPos);
        }
        *pbArrayLen = tempArrayPos;

        if (pTempArray) delete[] pTempArray;
    }

    return status;
}

int CCommonFnc::BYTE_ConvertFromHexNumToByte(string hexNum, BYTE* pByte) {
    DWORD num = strtol((LPCTSTR) hexNum.c_str(), NULL, 16);

    if (num == 0xFF) *pByte = 0xFF;
    else *pByte = (BYTE) num & 0xFF;
    
    return STAT_OK;
}

int CCommonFnc::APDU_ConvertToString(CARDAPDU* pAPDU, string* pString, BOOL toSendAPDU) {
    int         status = STAT_OK;
    string     message;
    string     ioData;
    
    if (toSendAPDU) {
        // APDU to SmartCard
        CCommonFnc::BYTE_ConvertFromArrayToHexString(pAPDU->DataIn, pAPDU->lc, &ioData);
        
        // FORMAT: INS CLA P1 P2 LC input_data Le 
        //message.Format("-> %.2x %.2x %.2x %.2x %.2x %s %.2x", pAPDU->cla, pAPDU->ins, pAPDU->p1, pAPDU->p2, pAPDU->lc, (LPCTSTR) ioData, pAPDU->le);
		message = string_format("-> %.2x %.2x %.2x %.2x %.2x %s %.2x", pAPDU->cla, pAPDU->ins, pAPDU->p1, pAPDU->p2, pAPDU->lc, (LPCTSTR)ioData.c_str(), pAPDU->le);
    }
    else {
        // APDU from SmartCard
           
        CCommonFnc::BYTE_ConvertFromArrayToHexString(pAPDU->DataOut, pAPDU->le, &ioData);
        
        // FORMAT: output_data SW
        //message.Format("<- %s %.2x %.2x", (LPCTSTR) ioData, HIGHBYTE(pAPDU->sw), LOWBYTE(pAPDU->sw));
		message = string_format("<- %s %.2x %.2x", (LPCTSTR)ioData.c_str(), HIGHBYTE(pAPDU->sw), LOWBYTE(pAPDU->sw));
    }

    *pString = message;

    return status;
}

int CCommonFnc::BYTE_ConvertFromArrayToHexString(BYTE* pArray, DWORD pbArrayLen, string* pHexaString) {
    int         status = STAT_OK;
    string     hexNum;
    DWORD       i;

    *pHexaString = "";
    for (i = 0; i < pbArrayLen; i++) {
        //hexNum.Format("%.2x", pArray[i]);
		hexNum = string_format("%.2x", pArray[i]);
        hexNum += " ";

        *pHexaString += hexNum;
    }

    //pHexaString->TrimRight(" ");
	pHexaString->erase(pHexaString->find_last_not_of(" ") + 1);

    return status;
}

int CCommonFnc::String_ParseNullSeparatedArray(BYTE* array, DWORD arraySize, lcs* pValueString) {
    int     status = STAT_OK;
    DWORD   pos;
    CString itemName;

	pos = 0;
	itemName = "";
	
	if (arraySize > 0) {
		while (pos <= (arraySize-1)) {	// -1 belong to special end zero
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

int CCommonFnc::String_ParseNullSeparatedArray(WCHAR* array, DWORD arraySize, lcs* pValueString) {
    int     status = STAT_OK;
    DWORD   pos;
    CString itemName;

	pos = 0;
	itemName = "";
	
	if (arraySize > 0) {
		while (pos <= (arraySize-1)) {	// -1 belong to special end zero
			if (array[pos] == '\0') {	// end of one item
				if (itemName != _T("")) pValueString->push_back(itemName);

				itemName = _T("");
			} 
			else itemName += array[pos];

			pos++;

		}	//end while
	}

    return status;
}

int CCommonFnc::SCSAT_SaveSamples(string filePath, SAMPLE_PLOT* pSample, int startOffset, int endOffset) {
    int     status = STAT_OK;
    
    MoveFile(filePath.c_str(), (filePath + ".bak").c_str());
    DeleteFile(filePath.c_str());
    // SAVE HEADER
    string tmp;
    pSample->measureInfo.formatToString(&tmp);
    CCommonFnc::File_AppendString(filePath, tmp);
    
    int samplesFileOffset = 0;
    if ((status = CCommonFnc::SCSAT_GetPowerSamplesFileOffset(filePath, &samplesFileOffset)) == STAT_OK) {
        if ((status = CCommonFnc::File_SaveMatrixIntFileOffset(samplesFileOffset, filePath, &(pSample->dataBlob), startOffset, endOffset, pSample->measureInfo.bSaveBinary)) == STAT_OK) {
            // store number of written samples
            if (endOffset == -1) endOffset = pSample->dataBlob.dwActLen;
            pSample->measureInfo.numSamples = endOffset - startOffset + 2;
			tmp = string_format("%d", pSample->measureInfo.numSamples);
            //tmp.Format("%d", pSample->measureInfo.numSamples);
            WritePrivateProfileString(SCSAT_MEASURE_SECTION, SCSAT_MEASURE_NUMSAMPLES, tmp.c_str(), filePath.c_str());
        }      
    }
    
    if (status == STAT_OK) {
        DeleteFile((filePath + ".bak").c_str());
    }

    return status;
}

int CCommonFnc::SCSAT_GetPowerSamplesFileOffset(string fileName, int* pOffset) {
    int     status = STAT_OK;
    ifstream file;
	file.open(fileName, std::fstream::in);

    if (file.is_open()) {
        // FIND 'SCSAT_MEASURE_POWERTRACE' VALUE - SEARCH USING SLIGHTLY OVERLAPPED BLOCKS
        CString dataBlock;
        #define BLOCK_READ_LEN  1000
        char    block[BLOCK_READ_LEN + 1];    
        DWORD   len = BLOCK_READ_LEN;
        DWORD   traceOffset = 0;
        DWORD   baseOffset = 0;
        while (traceOffset == 0) {  
            memset(block, 0, sizeof(block));  
            file.read(block, BLOCK_READ_LEN); 
			len = file.gcount();
            dataBlock = block;
            if ((dataBlock.Find(SCSAT_MEASURE_POWERTRACE) != -1)) {
                // we found SCSAT_MEASURE_POWERTRACE string, save its offset
                *pOffset = baseOffset + dataBlock.Find(SCSAT_MEASURE_POWERTRACE) + strlen(SCSAT_MEASURE_POWERTRACE) + 2; // offset just after POWERTRACE=
                break;    
            }
            
            if (len != BLOCK_READ_LEN) break; // we are at the end of file
            
            // go back slightly not to miss SCSAT_MEASURE_POWERTRACE string on block border
            file.seekg(-(int) strlen(SCSAT_MEASURE_POWERTRACE), ios_base::cur);
            baseOffset += (DWORD) (BLOCK_READ_LEN - strlen(SCSAT_MEASURE_POWERTRACE));
        }
        file.close();
    }
    else status = STAT_FILE_OPEN_FAIL;
   
    return status;
}

int CCommonFnc::SCSAT_EnsureFileHeader(string filePath, SCSAT_MEASURE_INFO* pInfo) {
	int status = STAT_OK;
	char iniValue[MAX_INI_VALUE_CHAR];

	// CHECK IF HEADER EXISTS
    BOOL    bNewFormat = FALSE; 
    int     fileLength = 0;
	fstream file;
	file.open(filePath, std::fstream::in | std::fstream::out);
   
	if (file.is_open()) {
        char    buffer[100];
        memset(buffer, 0, sizeof(buffer));
        file.read(buffer, 100);
		file.seekg(0, ios_base::end);
		fileLength = file.tellg();
		file.seekg(0, ios_base::beg);
        string header = buffer; string part = header.substr(1, (int) strlen(SCSAT_MEASURE_SECTION));
		
		char *partchar = new char[part.length() + 1];
		char *SCSAT_MEASURE_SECTION_char = new char[strlen(SCSAT_MEASURE_SECTION) + 1];
		strcpy(partchar, part.c_str());
		strcpy(SCSAT_MEASURE_SECTION_char, SCSAT_MEASURE_SECTION);
		toupper(*partchar);
		toupper(*SCSAT_MEASURE_SECTION_char);
		if(strcmp(partchar, SCSAT_MEASURE_SECTION_char) == 0) bNewFormat = TRUE;
        //if (part.CompareNoCase(SCSAT_MEASURE_SECTION) == 0) bNewFormat = TRUE;
        else bNewFormat = FALSE;
        
    	// WRITE IF NOT 
        if (!bNewFormat) {
	        string tmp;
			if (pInfo->sampleUniqueID == 0) {
				CCommonFnc::Sample_GenerateSampleUniqueID(&(pInfo->sampleUniqueID));
			}
            pInfo->formatToString(&tmp);
			file.seekg(0, ios_base::beg);
			file.seekp(0, ios_base::beg);
	        file.write((LPCTSTR) tmp.c_str(), tmp.length());
        }
        
        file.close();
    
		// if the sampleUniqueID is not present
		
		if(bNewFormat) {
			GetPrivateProfileString(SCSAT_MEASURE_SECTION, SCSAT_MEASURE_SAMPLEUNIQUEID, "", iniValue, MAX_INI_VALUE_CHAR, filePath.c_str());
			if (strlen(iniValue) == 0) {
				if (pInfo->sampleUniqueID == 0) {
				CCommonFnc::Sample_GenerateSampleUniqueID(&(pInfo->sampleUniqueID));
			    }
				string tmp;
				tmp = string_format("%lld", pInfo->sampleUniqueID);
				//tmp.AtlUtil::Format("%lld", pInfo->sampleUniqueID);
                WritePrivateProfileString(SCSAT_MEASURE_SECTION,SCSAT_MEASURE_SAMPLEUNIQUEID, tmp.c_str(), filePath.c_str());
			
			}	
		}
	}
	return status;
}
int CCommonFnc::Sample_GenerateSampleUniqueID(__int64* id) {
    __int64   a,b;
    
	srand((DWORD) read_tsc());
	//srand(time(NULL));
	a = 0;
	for(int i=0;i<8;i++){
	   b = rand()%256;
	   a = (a<<8)|b;
	}
	if (a<0) {
		*id = ~a;
	} else {
		*id = a;
	}
  
	return STAT_OK;
}
int CCommonFnc::Sample_Free(SAMPLE_PLOT* pSample) {
    int     status = STAT_OK;
    
	if (pSample != NULL) {
		if (pSample->dataBlob.pData) {
			delete[] pSample->dataBlob.pData;
			pSample->dataBlob.pData=NULL;
		}
		if (pSample->microshiftsDataBlob.pData) {
			delete[] pSample->microshiftsDataBlob.pData;
			pSample->microshiftsDataBlob.pData=NULL;
		}
		if (pSample->busBlob.pData) { 
			delete[] pSample->busBlob.pData;
			pSample->busBlob.pData=NULL;
		}
	}

    return status;
}
