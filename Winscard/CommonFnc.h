#ifndef COMMONFNC_H
#define COMMONFNC_H

#include "..\Shared\globals.h"

class CCommonFnc {
public:
	template <class ... Args>
	static std::string string_format(const std::string& format, Args ... args);
    static int File_GetAvailableFileName(CString baseFile, CString* pFreeFileName);
    static int File_AppendString(CString filePath, CString data);
    static int CCommonFnc::File_SaveMatrixIntFileOffset(int startFileOffset, CString filePath, INT_DATA_BLOB* pBlob, int startOffset, int endOffset, BOOL bSaveBinary);
    static int CCommonFnc::File_SaveMatrixInt(CString filePath, INT_DATA_BLOB* pBlob, int startOffset, int endOffset, int startFileOffset, BOOL bSaveBinary);

    static int BYTE_ConvertFromHexStringToArray(CString hexaString, BYTE* pArray, BYTE* pbArrayLen);
    static int BYTE_ConvertFromHexStringToArray(CString hexaString, BYTE* pArray, DWORD* pbArrayLen);
    static int BYTE_ConvertFromHexNumToByte(CString hexaString, BYTE* pByte);
    static int BYTE_ConvertFromArrayToHexString(BYTE* pArray, DWORD pbArrayLen, CString* pHexaString);

    static int APDU_ConvertToString(CARDAPDU* pAPDU, CString* pString, BOOL toSendAPDU);

    static int String_ParseNullSeparatedArray(BYTE* array, DWORD arraySize, lcs* pValueString);
    static int String_ParseNullSeparatedArray(WCHAR* array, DWORD arraySize, lcs* pValueString);

    static int SCSAT_SaveSamples(CString filePath, SAMPLE_PLOT* pSample, int startOffset = 0, int endOffset = -1);
    static int SCSAT_GetPowerSamplesFileOffset(CString fileName, int* pOffset);
	static int SCSAT_EnsureFileHeader(CString filePath, SCSAT_MEASURE_INFO* pInfo);

	static int Sample_GenerateSampleUniqueID(__int64* id);
    static int Sample_Free(SAMPLE_PLOT* pSample);
};

#endif
