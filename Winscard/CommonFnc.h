#ifndef COMMONFNC_H
#define COMMONFNC_H

#include "../Shared/globals.h"

class CCommonFnc {
public:
#if defined(_WIN32)
	static int File_GetAvailableFileName(string_type baseFile, string_type* pFreeFileName);
#endif
    static int File_AppendString(string_type filePath, std::string data);
	static int File_AppendString(string_type filePath, std::wstring data);
#if defined(_WIN32)
    static int File_SaveMatrixIntFileOffset(size_t startFileOffset, string_type filePath, INT_DATA_BLOB* pBlob, size_t startOffset, size_t endOffset, BOOL bSaveBinary);
    static int File_SaveMatrixInt(string_type filePath, INT_DATA_BLOB* pBlob, size_t startOffset, size_t endOffset, size_t startFileOffset, BOOL bSaveBinary);
#endif
    static int BYTE_ConvertFromHexStringToArray(string_type hexaString, BYTE* pArray, BYTE* pbArrayLen);
    static int BYTE_ConvertFromHexStringToArray(string_type hexaString, BYTE* pArray, DWORD* pbArrayLen);
    static int BYTE_ConvertFromHexNumToByte(string_type hexaString, BYTE* pByte);
    static int BYTE_ConvertFromArrayToHexString(const BYTE* pArray, DWORD pbArrayLen, string_type* pHexaString);

    static int APDU_ConvertToString(CARDAPDU* pAPDU, string_type* pString, BOOL toSendAPDU);
	
    static int String_ParseNullSeparatedArray(BYTE* array, DWORD arraySize, ls* pValueString);
    static int String_ParseNullSeparatedArray(WCHAR* array, DWORD arraySize, lws* pValueString);
};

size_t compareWithNoCase(const char_type* str1, const char_type* str2);
string_type getCurrentTimeString();

#endif
