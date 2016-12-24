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
	#ifdef __linux__
		LoadRules();
		if (theApp.m_winscardConfig.bLOG_EXCHANGED_APDU) CCommonFnc::File_AppendString(WINSCARD_LOG, _CONV("[begin]\r\n"));
		initialize();
	#endif
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

	return initialize();
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

int CWinscardApp::LoadRule(const char_type* section_name, dictionary* dict/*string_type filePath*/) {
	int status = STAT_OK;
	char_type buffer[10000];
	DWORD cBuffer = 10000;
	string_type valueName;
	string_type rulePart;
	string_type ruleString;
	string_type elemName;
	string_type subValue;
	string_type help;
	APDU_RULE rule;
	APDU_SINGLE_RULE singleRule;
	char_type sec_and_key[256];
	const char_type* char_value;
	int value;
	string_type section_name_string = section_name;

	if (compareWithNoCase(section_name, _CONV("WINSCARD")) == 0)
	{
		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":AUTO_REQUEST_DATA")), 2)) != 2)
		{
			m_winscardConfig.bAUTO_REQUEST_DATA = value;
		}

		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":FORCE_CONNECT_SHARED_MODE")), 2)) != 2)
		{
			m_winscardConfig.bFORCE_CONNECT_SHARED_MODE = value;
		}

		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":FORCE_APDU_NONZERO_INPUT_DATA")), 2)) != 2)
		{
			m_winscardConfig.bFORCE_APDU_NONZERO_INPUT_DATA = value;
		}

		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":LOG_EXCHANGED_APDU")), 2)) != 2)
		{
			m_winscardConfig.bLOG_EXCHANGED_APDU = value;
		}

		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":LOG_BASE_PATH")), 2)) != 2)
		{
			m_winscardConfig.sLOG_BASE_PATH = value;
		}

		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":MODIFY_APDU_BY_RULES")), 2)) != 2)
		{
			m_winscardConfig.bMODIFY_APDU_BY_RULES = value;
		}

		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":LOG_FUNCTIONS_CALLS")), 2)) != 2)
		{
			m_winscardConfig.bLOG_FUNCTIONS_CALLS = value;
		}

		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":READER_ORDERED_FIRST")), 2)) != 2)
		{
			m_winscardConfig.sREADER_ORDERED_FIRST = value;
		}
	}

	if (compareWithNoCase(section_name, _CONV("SCSAT04")) == 0)
	{
		// SCSAT04 CONFIGURATION RULE

		type_copy(sec_and_key, section_name);
		if ((value = iniparser_getboolean(dict, type_cat(sec_and_key, _CONV(":REDIRECT")), 2)) != 2)
		{
			m_winscardConfig.bAUTO_REQUEST_DATA = value;
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":IP")), "");
		if (type_length(char_value) != 0)
		{
			m_scsat04Config.IP = char_value;
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":PORT")), "");
		if (type_length(char_value) != 0)
		{
			m_scsat04Config.IP = char_value;
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":MEASURE_APDU")), "");
		if (type_length(char_value) != 0)
		{
			m_scsat04Config.measureApduLen = sizeof(m_scsat04Config.measureApdu);
			CCommonFnc::BYTE_ConvertFromHexStringToArray(char_value, m_scsat04Config.measureApdu, &(m_scsat04Config.measureApduLen));
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":MEASURE_BYTE_COUNTER")), "");
		if (type_length(char_value) != 0)
		{
			m_scsat04Config.measureApduByteCounter = type_to_int(char_value, NULL, 10);
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":MEASURE_BYTE_DELAY")), "");
		if (type_length(char_value) != 0)
		{
			m_scsat04Config.measureApduByteDelay = type_to_int(char_value, NULL, 10);
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":READ_RATIO")), "");
		if (type_length(char_value) != 0)
		{
			m_scsat04Config.readRatio = type_to_int(char_value, NULL, 10);
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":NUM_SAMPLES")), "");
		if (type_length(char_value) != 0)
		{
			m_scsat04Config.numSamples = type_to_int(char_value, NULL, 10);
		}
	}
	
	if (compareWithNoCase(section_name_string.substr(0, (int) type_length(_CONV("RULE"))).c_str(), _CONV("RULE")) == 0)
	{
		// COMMON RULE

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":USAGE")), "");
		if (type_length(char_value) != 0)
		{
			rule.usage = type_to_int(char_value, NULL, 10);
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":APDUIN")), "");
		if (type_length(char_value) != 0)
		{
			rule.direction = type_to_int(char_value, NULL, 10);
		}

		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, _CONV(":DELAY")), "");
		if (type_length(char_value) != 0)
		{
			rule.msDelay = type_to_int(char_value, NULL, 10);
		}

		// SET RULE NAME FOR FUTURE IDENTIFICATION
		rule.ruleName = section_name_string;

		// LOAD MATCH RULES
		int counter = 1;
		int pos = 0;
		int pos2 = 0;
		valueName = string_format(_CONV(":MATCH%d"), counter);
		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, valueName.c_str()), "");
		while (type_length(char_value) != 0)
		{
			ruleString = char_value;
			ruleString += _CONV(" ");

			// FIND HISTORY ELEMENT, WILL BE SAME FOR ALL OTHER ELEMENTAREY RULES
			if ((pos = ruleString.find(_CONV("t="))) != string_type::npos)
			{
				//singleRule.history = atoi(ruleString.substr(pos + (int) type_length(_CONV("t="))).c_str());
				singleRule.history = type_to_int(ruleString.substr(pos + (int)type_length(_CONV("t="))).c_str(), NULL, 10);
				ruleString.erase(pos, ruleString.find(_CONV(";"), pos) - pos + 1); // remove from rule string
			}
			// FIND DIRECTION ELEMENT (IN/OUT), WILL BE SAME FOR ALL OTHER ELEMENTAREY RULES
			if ((pos = ruleString.find(_CONV("in="))) != string_type::npos)
			{
				singleRule.apduDirection = type_to_int(ruleString.substr(pos + (int) type_length(_CONV("in="))).c_str(), NULL, 10);
				ruleString.erase(pos, ruleString.find(_CONV(";"), pos) - pos + 1); // remove from rule string
			}

			// PARSE RULE AND CREATE ELEMENTARY RULES FOREACH BYTE
			pos2 = 0;
			while ((pos = ruleString.find(_CONV(";"), pos2)) != -1)
			{
				rulePart = ruleString.substr(pos2, pos - pos2 + 1);

				//elemName = rulePart.Left(rulePart.Find("="));
				elemName = rulePart.substr(0, rulePart.find(_CONV("=")));

				if (compareWithNoCase(elemName.c_str(), _CONV("CLA")) == 0)
				{
					singleRule.element = CLA_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.matchRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("INS")) == 0)
				{
					singleRule.element = INS_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.matchRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("P1")) == 0)
				{
					singleRule.element = P1_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.matchRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("P2")) == 0)
				{
					singleRule.element = P2_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.matchRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("LC")) == 0)
				{
					singleRule.element = LC_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.matchRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.substr(0, (int) type_length(_CONV("DATA"))).c_str(), _CONV("DATA")) == 0)
				{
					// DATA CAN BE WRITTEN IN MORE VALUES AT ONCE, STARTING ON POSITION DATAx
					// CREATE SEPARATE ELEMENT FOR EACH
					int offset = type_to_int(elemName.substr(section_name_string.find_first_of(_CONV("0123456789")), 0).c_str(), NULL, 10);
					// GO OVER ALL MATCH DATA
					string_type data = rulePart.substr(rulePart.find(_CONV("=")) + 1);
					//data.Replace(";", "");
					data.erase(remove(data.begin(), data.end(), ';'), data.end());
					BYTE dataBuffer[300];
					DWORD dataBufferLen = 300;
					CCommonFnc::BYTE_ConvertFromHexStringToArray(data, dataBuffer, &dataBufferLen);
					for (DWORD i = 0; i < dataBufferLen; i++)
					{
						if (singleRule.apduDirection == INPUT_APDU) singleRule.element = offset + OFFSET_CDATA;
						else singleRule.element = offset;
						singleRule.value = dataBuffer[i];
						singleRule.valid = TRUE;
						rule.matchRules.push_back(singleRule);
						// increase offset for next element
						offset++;
					}
				}

				pos2 = pos + 1;
			}

			counter++;
			valueName = string_format(_CONV("MATCH%d"), counter);
			type_copy(sec_and_key, section_name);
			char_value = iniparser_getstring(dict, type_cat(sec_and_key, valueName.c_str()), "");
		}

		// LOAD ACTION RULES
		counter = 1;
		pos = 0;
		type_copy(sec_and_key, section_name);
		char_value = iniparser_getstring(dict, type_cat(sec_and_key, ":ACTION"), "");
		if (type_length(char_value) != 0)
		{
			ruleString = buffer;
			ruleString += _CONV(" ");
			// PARSE RULE AND CREATE ELEMENTARY RULES FOREACH BYTE
			singleRule.clear();
			// FIND DIRECTION ELEMENT (IN/OUT), WILL BE SAME FOR ALL OTHER ELEMENTARY RULES
			if ((pos = ruleString.find(_CONV("in="))) != string_type::npos)
			{
				singleRule.apduDirection = type_to_int(ruleString.substr(pos + (int) type_length(_CONV("in="))).c_str(), NULL, 10);
				//ruleString.Delete(pos, ruleString.find(";", pos) - pos + 1); // remove from rule string
				ruleString.erase(pos, ruleString.find(_CONV(";"), pos) - pos + 1);
			}
			pos2 = 0;
			while ((pos = ruleString.find(_CONV(";"), pos2)) != string_type::npos)
			{
				rulePart = ruleString.substr(pos2, pos - pos2 + 1);

				elemName = rulePart.substr(0, rulePart.find(_CONV("=")));

				if (compareWithNoCase(elemName.c_str(), _CONV("CLA")) == 0)
				{
					singleRule.element = CLA_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.actionRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("INS")) == 0)
				{
					singleRule.element = INS_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.actionRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("P1")) == 0)
				{
					singleRule.element = P1_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.actionRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("P2")) == 0)
				{
					singleRule.element = P2_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.actionRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("LC")) == 0)
				{
					singleRule.element = LC_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.actionRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.c_str(), _CONV("LE")) == 0)
				{
					singleRule.element = LE_ELEM;
					CCommonFnc::BYTE_ConvertFromHexNumToByte(rulePart.substr(rulePart.find(_CONV("=")) + 1, 2), &(singleRule.value));
					singleRule.valid = TRUE;
					rule.actionRules.push_back(singleRule);
				}
				if (compareWithNoCase(elemName.substr(0, (int) type_length(_CONV("DATA"))).c_str(), _CONV("DATA")) == 0)
				{
					// DATA CAN BE WRITTEN IN MORE VALUES AT ONCE, STARTING ON POSITION DATAx
					// CREATE SEPARATE ELEMENT FOR EACH
					int offset = type_to_int(elemName.substr(section_name_string.find_first_of(_CONV("0123456789"))).c_str(), NULL, 10);
					// GO OVER ALL MATCH DATA
					string_type data = rulePart.substr(rulePart.find(_CONV("=")) + 1);
					//data.Replace(";", "");
					data.erase(remove(data.begin(), data.end(), ';'), data.end());
					BYTE dataBuffer[300];
					DWORD dataBufferLen = 300;
					CCommonFnc::BYTE_ConvertFromHexStringToArray(data, dataBuffer, &dataBufferLen);
					for (DWORD i = 0; i < dataBufferLen; i++)
					{
						if (singleRule.apduDirection == INPUT_APDU) singleRule.element = offset + OFFSET_CDATA;
						else singleRule.element = offset;
						singleRule.value = dataBuffer[i];
						singleRule.valid = TRUE;
						rule.actionRules.push_back(singleRule);
						// increase offset for next element
						offset++;
					}
				}

				pos2 = pos + 1;
			}
		}

		rulesList.push_back(rule);
	}

	return status;
}

int CWinscardApp::LoadRules() {
	int status = STAT_OK;
	string_type filePath;

	CCommonFnc::File_AppendString(WINSCARD_RULES_LOG, _CONV("#########################################\n"));

	dictionary* dict = iniparser_load((const char*) RULE_FILE.c_str());

	int number_of_sections = iniparser_getnsec(dict);
 
	for (int i = 0; i < number_of_sections; ++i)
	{
		const char* section_name = iniparser_getsecname(dict, i);
		LoadRule(section_name, dict);
	}

	m_bRulesActive = TRUE;

	WINSCARD_RULES_LOG = string_format(_CONV("%swinscard_rules_log.txt"), m_winscardConfig.sLOG_BASE_PATH);
	WINSCARD_LOG = string_format(_CONV("%swinscard_log.txt"), m_winscardConfig.sLOG_BASE_PATH);

	iniparser_freedict(dict);

    /*memset(buffer, 0, cBuffer);
  
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
	*/

    return status;
}

