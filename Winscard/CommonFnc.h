#ifndef COMMONFNC_H
#define COMMONFNC_H

#include "..\Shared\globals.h"

#if defined (UNICODE) && defined (_WIN32)
typedef size_t(*f)(const char_type*);
typedef char_type*(*g)(char_type*, const char_type*);
typedef int(*k)(const char_type*, const char_type*);
typedef long (*l)(const char_type*, char_type**, int);
typedef errno_t(*n)(const char_type*, char_type*, size_t, char_type*, size_t, char_type*, size_t, char_type*, size_t);
static f type_length = wcslen;
static g type_copy = wcscpy;
static k type_compare = wcscmp;
static l type_to_int = wcstol;
static n type_path_split = _wsplitpath_s;
#else 
typedef size_t(*f)(const char_type*);
typedef char_type*(*g)(char_type*, const char_type*);
typedef int(*k)(const char_type*, const char_type*);
typedef long(*l)(const char_type*, char_type**, int);
typedef errno_t(*n)(const char_type*, char_type*, size_t, char_type*, size_t, char_type*, size_t, char_type*, size_t);
static f type_length = strlen;
static g type_copy = strcpy;
static k type_compare = strcmp;
static l type_to_int = strtol;
static n type_path_split = _splitpath_s;
#endif

class CCommonFnc {
public:
    static int File_GetAvailableFileName(string_type baseFile, string_type* pFreeFileName);
    static int File_AppendString(string_type filePath, string_type data);
    static int CCommonFnc::File_SaveMatrixIntFileOffset(int startFileOffset, string_type filePath, INT_DATA_BLOB* pBlob, int startOffset, int endOffset, BOOL bSaveBinary);
    static int CCommonFnc::File_SaveMatrixInt(string_type filePath, INT_DATA_BLOB* pBlob, int startOffset, int endOffset, int startFileOffset, BOOL bSaveBinary);

    static int BYTE_ConvertFromHexStringToArray(string_type hexaString, BYTE* pArray, BYTE* pbArrayLen);
    static int BYTE_ConvertFromHexStringToArray(string_type hexaString, BYTE* pArray, DWORD* pbArrayLen);
    static int BYTE_ConvertFromHexNumToByte(string_type hexaString, BYTE* pByte);
    static int BYTE_ConvertFromArrayToHexString(BYTE* pArray, DWORD pbArrayLen, string_type* pHexaString);

    static int APDU_ConvertToString(CARDAPDU* pAPDU, string_type* pString, BOOL toSendAPDU);

    static int String_ParseNullSeparatedArray(BYTE* array, DWORD arraySize, lcs* pValueString);
    static int String_ParseNullSeparatedArray(WCHAR* array, DWORD arraySize, lcs* pValueString);

    static int SCSAT_SaveSamples(string_type filePath, SAMPLE_PLOT* pSample, int startOffset = 0, int endOffset = -1);
    static int SCSAT_GetPowerSamplesFileOffset(string_type fileName, int* pOffset);
	static int SCSAT_EnsureFileHeader(string_type filePath, SCSAT_MEASURE_INFO* pInfo);

	static int Sample_GenerateSampleUniqueID(__int64* id);
    static int Sample_Free(SAMPLE_PLOT* pSample);
};

#endif
