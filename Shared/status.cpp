#include "status.h"

const char* ErrorToString(int error) {
    switch (error) {
            // BASE ERROR STATUS
        case STAT_OK:                                   return "SUCCESS";   
        case STAT_NOT_IMPLEMENTED_YET:                  return "NOT_IMPLEMENTED_YET";
        case STAT_SESSION_NOT_OPEN:                     return "SESSION_NOT_OPEN";
        case STAT_NOT_ENOUGHT_MEMORY:                   return "NOT_ENOUGHT_MEMORY";
        case STAT_CHANNEL_SECURITY_NOT_ACHIEVED:        return "CHANNEL_SECURITY_NOT_ACHIEVED";
        case STAT_CHANNEL_DESTROY_FAIL:                 return "CHANNEL_DESTROY_FAIL";
        case STAT_CHANNEL_NOT_EXISTS:                   return "CHANNEL_NOT_EXISTS"; 
        case STAT_INCORRECT_HWT:                        return "INCORRECT_HWT";
        case STAT_INCORRECT_SWA:                        return "INCORRECT_SWA";
        case STAT_DATA_CORRUPTED:                       return "DATA_CORRUPTED";
        case STAT_DATA_INCORRECT_LENGTH:                return "DATA_INCORRECT_LENGTH";
        case STAT_KEY_SCHEDULE_FAIL:                    return "KEY_SCHEDULE_FAIL"; 
        case STAT_CIPHER_INIT_FAIL:                     return "CIPHER_INIT_FAIL"; 
        case STAT_ENCRYPT_FAIL:                         return "ENCRYPT_FAIL"; 
        case STAT_DECRYPT_FAIL:                         return "DECRYPT_FAIL"; 
        case STAT_FILE_OPEN_FAIL:                       return "FILE_OPEN_FAIL"; 
        case STAT_SEAUT_FRESHNESS_FAIL:                 return "SEAUT_FRESHNESS_FAIL";
        case STAT_NO_CONNECTION:                        return "NO_CONNECTION";
        case STAT_NOT_ENOUGHT_DATA_TYPE:                return "NOT_ENOUGHT_DATA_TYPE";
        case STAT_LICENCE_TOO_LONG:                     return "STAT_LICENCE_TOO_LONG";      
        case STAT_RESPONSE_DATA_LENGTH_BAD:             return "STAT_RESPONSE_DATA_LENGTH_BAD";
        case STAT_USERDATA_BAD:                         return "STAT_USERDATA_BAD";
        case STAT_KEY_LENGTH_BAD:                       return "STAT_KEY_LENGTH_BAD";
        case STAT_DATA_TOO_LONG:                        return "STAT_DATA_TOO_LONG";
        case STAT_INI_DATA_WRITE_FAIL:                  return "STAT_INI_DATA_WRITE_FAIL";                             
        case STAT_INI_DATA_READ_FAIL:                   return "STAT_INI_DATA_READ_FAIL";
        case STAT_CODING_NOT_BIJECT:                    return "STAT_CODING_NOT_BIJECT";               
        case STAT_CODING_ALREADY_ASSIGNED:              return "STAT_CODING_ALREADY_ASSIGNED"; 
        case STAT_CONFIG_SCRIPT_INCOMPLETE:             return "STAT_CONFIG_SCRIPT_INCOMPLETE";
        case STAT_IOC_CORRUPTED:                        return "STAT_IOC_CORRUPTED";           
        case STAT_LOAD_AUTH_FNC_FAIL:                   return "STAT_LOAD_AUTH_FNC_FAIL";      
        case STAT_LOAD_LIBRARY_FAIL:                    return "STAT_LOAD_LIBRARY_FAIL";       
        case STAT_SCARD_ERROR:                          return "STAT_SCARD_ERROR";             
        case STAT_FULL_PATH_FAIL:                       return "STAT_FULL_PATH_FAIL";
        case STAT_UNKNOWN_SCARD_PROTOCOL:               return "STAT_UNKNOWN_SCARD_PROTOCOL";
        case STAT_OPERATION_CANCELED:                   return "STAT_OPERATION_CANCELED";
        case STAT_STRING_NOT_FOUND:                     return "STAT_STRING_NOT_FOUND";
        case STAT_COORDINATES_EXCEEDS:                  return "STAT_COORDINATES_EXCEEDS";  
        case STAT_NODE_NOT_EXISTS:                      return "STAT_NODE_NOT_EXISTS";
        case STAT_NO_SUCH_NEIGHBOUR:                    return "STAT_NO_SUCH_NEIGHBOUR";  
        case STAT_PORT_DRIVER_INIT_FAIL:                return "STAT_PORT_DRIVER_INIT_FAIL";
        case STAT_SOFT_KILLED:                          return "STAT_SOFT_KILLED";       
        case STAT_DEPLOY_INTEGRITY_BAD:                 return "STAT_DEPLOY_INTEGRITY_BAD";
        case STAT_BS_SELECTRULEUNKNOWN:                 return "STAT_BS_SELECTRULEUNKNOWN";
        case STAT_BS_ROUTESTRATEGYUNKNOWN:              return "STAT_BS_ROUTESTRATEGYUNKNOWN";
        case STAT_WRONG_NONCE:                          return "STAT_WRONG_NONCE";

        case STAT_DATA_NOT_ALIGNED:                     return "STAT_DATA_NOT_ALIGNED";
        case STAT_INTERNAL_ERROR:                       return "STAT_INTERNAL_ERROR";
        case STAT_DIVISION_BY_ZERO:                     return "STAT_DIVISION_BY_ZERO";
        case STAT_DATA_INCORRECT_LIMITS:                return "STAT_DATA_INCORRECT_LIMITS";
        case STAT_VALUE_NOT_READ:                       return "STAT_VALUE_NOT_READ";
        case STAT_VALUE_NOT_SET:                        return "STAT_VALUE_NOT_SET";
        case STAT_UNKNOWN_VALUE:                        return "STAT_UNKNOWN_VALUE"; 
        case STAT_DLG_NOT_EXISTS:                       return "STAT_DLG_NOT_EXISTS";   
		case STAT_CAP_UPLOAD_FAILED:					return "STAT_CAP_UPLOAD_FAILED";

        case STAT_CANCEL:                               return "STAT_CANCEL";
                                                           
            // GEMPLUS CARDCOMM ERRORS
        case STAT_CCR_UNKNOWN:                          return "STAT_CCR_UNKNOWN";
        case STAT_CCR_INVALID_HANDLE:                   return "STAT_CCR_INVALID_HANDLE";
        case STAT_CCR_CARD_NOT_PRESENT:                 return "STAT_CCR_CARD_NOT_PRESENT";
        case STAT_CCR_SESSION_NOT_OPENED:               return "STAT_CCR_SESSION_NOT_OPENED";
        case STAT_CCR_INVALID_SLOT_ID:                  return "STAT_CCR_INVALID_SLOT_ID";
        case STAT_CCR_FUNCTION_FAILED:                  return "STAT_CCR_FUNCTION_FAILED";
        case STAT_CCR_CANCELLED:                        return "STAT_CCR_CANCELLED";
                                               
            // ISO7816 ERRORS                           
        case SW_FILE_FULL:                              return "SW_FILE_FULL";
        case SW_UNKNOWN:  				                return "SW_UNKNOWN";
        case SW_CLA_NOT_SUPPORTED: 		                return "SW_CLA_NOT_SUPPORTED";
        case SW_INS_NOT_SUPPORTED: 		                return "SW_INS_NOT_SUPPORTED";
        case SW_CORRECT_LENGTH_00: 		                return "SW_CORRECT_LENGTH_00";
        case SW_WRONG_P1P2:   			                return "SW_WRONG_P1P2";
        case SW_LC_INCONSISTENT_WITH_P1P2:              return "SW_LC_INCONSISTENT_WITH_P1P2";
        case SW_INCORRECT_P1P2:			                return "SW_INCORRECT_P1P2"; 
        case SW_RECORD_NOT_FOUND: 		                return "SW_RECORD_NOT_FOUND";	
        case SW_FILE_NOT_FOUND: 		                return "SW_CLA_NOT_SUPPORTED";
        case SW_FUNC_NOT_SUPPORTED:		                return "SW_FUNC_NOT_SUPPORTED";
        case SW_WRONG_DATA:   			                return "SW_WRONG_DATA";
        case SW_APPLET_SELECT_FAILED: 	                return "SW_APPLET_SELECT_FAILED";
        case SW_COMMAND_NOT_ALLOWED: 	                return "SW_COMMAND_NOT_ALLOWED";
        case SW_CONDITIONS_NOT_SATISFIED:               return "SW_CONDITIONS_NOT_SATISFIED";
        case SW_DATA_INVALID: 			                return "SW_DATA_INVALID";
        case SW_FILE_INVALID: 			                return "SW_FILE_INVALID";
        case SW_SECURITY_STATUS_NOT_SATISFIED:          return "SW_SECURITY_STATUS_NOT_SATISFIED";
        case SW_WRONG_LENGTH:         			        return "SW_WRONG_LENGTH";
        case SW_BYTES_REMAINING_00:       		        return "SW_BYTES_REMAINING_00";
        case SW_NO_ERROR:             			        return "SW_NO_ERROR";
            
            // JCSTATUS
        case SW_JCDOMAIN_ALGORITHM_NOT_SUPPORTED:       return "SW_JCDOMAIN_ALGORITHM_NOT_SUPPORTED";
        case SW_JCDOMAIN_APPLET_INVALIDATED:            return "SW_JCDOMAIN_APPLET_INVALIDATED";
        case SW_JCDOMAIN_AUTHENTICATION_FAILED:         return "SW_JCDOMAIN_AUTHENTICATION_FAILED";
        case SW_JCDOMAIN_AUTHORIZATION_FAILED:          return "SW_JCDOMAIN_AUTHORIZATION_FAILED";
        case SW_JCDOMAIN_CHECKSUM_FAILED:               return "SW_JCDOMAIN_CHECKSUM_FAILED";
        case SW_JCDOMAIN_DECRYPTION_FAILED:             return "SW_JCDOMAIN_DECRYPTION_FAILED";
        case SW_JCDOMAIN_INSTALLATION_FAILED:           return "SW_JCDOMAIN_INSTALLATION_FAILED";
        case SW_JCDOMAIN_INVALID_STATE:                 return "SW_JCDOMAIN_INVALID_STATE";
        case SW_JCDOMAIN_NO_SPECIFIC_DIAGNOSIS:         return "SW_JCDOMAIN_NO_SPECIFIC_DIAGNOSIS";
        case SW_JCDOMAIN_REFERENCE_DATA_NOT_FOUND:      return "SW_JCDOMAIN_REFERENCE_DATA_NOT_FOUND";
        case SW_JCDOMAIN_REGISTRATION_FAILED:           return "SW_JCDOMAIN_REGISTRATION_FAILED";
        case SW_JCDOMAIN_SIGNATURE_CHECK_FAILED:        return "SW_JCDOMAIN_SIGNATURE_CHECK_FAILED";
        case SW_JCDOMAIN_SM_INCORRECT:                  return "SW_JCDOMAIN_SM_INCORRECT";
        case SW_JCDOMAIN_SM_MISSING:                    return "SW_JCDOMAIN_SM_MISSING";
                        
                                                        
            // MY OWN APPLET ERRORS                     
        case SW_SECURITY_NOT_SATISFIED:			        return "SW_SECURITY_NOT_SATISFIED";
        case SW_USAGE_UPDATE_FAIL:	                    return "SW_USAGE_UPDATE_FAIL";
        case SW_USAGE_NOT_ALLOWED:	                    return "SW_USAGE_NOT_ALLOWED";
        case SW_ALG_UUID_NOT_UNIQUE:                    return "SW_ALG_UUID_NOT_UNIQUE";
        case SW_ALG_CONTAINER_FULL:                     return "SW_ALG_CONTAINER_FULL";
        case SW_ALG_UUID_NOT_FOUND:                     return "SW_ALG_UUID_NOT_FOUND";
        case SW_SWA_UUID_NOT_UNIQUE:                    return "SW_SWA_UUID_NOT_UNIQUE";
        case SW_SWA_CONTAINER_FULL:                     return "SW_SWA_CONTAINER_FULL";
        case SW_BAD_KEY:                                return "SW_BAD_KEY";
        case SW_ALG_TYPE_UNKNOWN:                       return "SW_ALG_TYPE_UNKNOWN";
        case SW_ALG_UUID_BAD_LENGTH:                    return "SW_ALG_UUID_BAD_LENGTH";
        case SW_SWA_UUID_BAD_LENGTH:                    return "SW_SWA_UUID_BAD_LENGTH";
        case SW_HWT_UUID_BAD:                           return "SW_HWT_UUID_BAD";
        case SW_SWA_UUID_BAD:                           return "SW_SWA_UUID_BAD";
        case SW_XML_UNEXPECTED_ELEM:                    return "SW_XML_UNEXPECTED_ELEM";
        case SW_XML_UNEXPECTED_END:                     return "SW_XML_UNEXPECTED_END";
        case SW_LICENCE_OUTDATED:                       return "SW_LICENCE_OUTDATED";
        case SW_SWA_PERMIT_ALG_FULL:                    return "SW_SWA_PERMIT_ALG_FULL";
        case SW_ALG_SWA_NOT_PERMITED:                   return "SW_ALG_SWA_NOT_PERMITED";
        case SW_LICENCE_OUTOFORDER:                     return "SW_LICENCE_OUTOFORDER";
        case SW_SEAUT_STOP_MESSAGE_BAD:                 return "SW_SEAUT_STOP_MESSAGE_BAD";
        case SW_SWA_AUTH_REQUIRED:                      return "SW_SWA_AUTH_REQUIRED";
        case SW_ENCRYPT_FAIL:                           return "SW_ENCRYPT_FAIL";
        case SW_SWA_NOT_FOUND:                          return "SW_SWA_NOT_FOUND";
        case SW_ALG_NOT_FOUND:                          return "SW_ALG_NOT_FOUND";
        case SW_XMLPARSER_INSUFFICIENT_DATA:            return "SW_XMLPARSER_INSUFFICIENT_DATA";
        case SW_XMLPARSER_INVALID_DATA:                 return "SW_XMLPARSER_INVALID_DATA";
        case SW_XMLPARSER_UNKNOWN_ERROR:                return "SW_XMLPARSER_UNKNOWN_ERROR";
        case SW_IV_BAD:                                 return "SW_IV_BAD";
        case SW_CIPHER_DATA_LENGTH_BAD:                 return "SW_CIPHER_DATA_LENGTH_BAD";
        case SW_MESSAGE_LENGTH_BAD:                     return "SW_MESSAGE_LENGTH_BAD";
        case SW_SEAUT_STEP_BAD:                         return "SW_SEAUT_STEP_BAD";
        case SW_SEAUT_FRESHNESS_FAIL:                   return "SW_SEAUT_FRESHNESS_FAIL";
        case SW_KEY_LENGTH_BAD:                         return "SW_KEY_LENGTH_BAD";
        case SW_ALG_INPUT_BAD:                          return "SW_ALG_INPUT_BAD";
        case SW_BAD_PIN:                                return "SW_BAD_PIN";
        case SW_BAD_TEST_DATA_LEN:                      return "SW_BAD_TEST_DATA_LEN"; 
        
        case SW_SENSITIVE_ATTEMPTS_BLOCKED:             return "SW_SENSITIVE_ATTEMPTS_BLOCKED";
        case SW_SENSITIVE_ATTEMPTS_TOO_HIGH:            return "SW_SENSITIVE_ATTEMPTS_TOO_HIGH";
        case SW_INVALID_ACTIVE_STATE:                   return "SW_INVALID_ACTIVE_STATE"; 
        case SW_WRONG_MAGIC_VALUE:                      return "SW_WRONG_MAGIC_VALUE";          
        case SW_UNKNOWN_OPERATION:                      return "SW_UNKNOWN_OPERATION";          
        case SW_NOT_IMPLEMENTED_YET:                    return "SW_NOT_IMPLEMENTED_YET";        
        case SW_UNKNOWN_INTERNAL_STATE:                 return "SW_UNKNOWN_INTERNAL_STATE";     
        case SW_WRONG_INTERNAL_STATE:                   return "SW_WRONG_INTERNAL_STATE";       
        case SW_RSA_DECRYPT_BAD:                        return "SW_RSA_DECRYPT_BAD";            
        case SW_WRONG_HASH_LENGTH:                      return "SW_WRONG_HASH_LENGTH";          
        case SW_HASH_DIFFERS:                           return "SW_HASH_DIFFERS";               
        case SW_UNKNOWN_LICENCE_TYPE:                   return "SW_UNKNOWN_LICENCE_TYPE";       
        case SW_WRONG_APPLET_SN:                        return "SW_WRONG_APPLET_SN";            
        case SW_LICENCE_ALREADY_USED:                   return "SW_LICENCE_ALREADY_USED";       
        case SW_LICENCE_LENGTH_BAD:                     return "SW_LICENCE_LENGTH_BAD";         
        case SW_NO_CREDIT_LEFT:                         return "SW_NO_CREDIT_LEFT";             
        case SW_WRONG_LICENCE_TAG:                      return "SW_WRONG_LICENCE_TAG";          
        case SW_LICENCE_PREVIOUS_EXPECTED:              return "SW_LICENCE_PREVIOUS_EXPECTED"; 
        case SW_NO_SUCH_APPLICATION_PROFILE:            return "SW_NO_SUCH_APPLICATION_PROFILE";
        case SW_WRONG_MAC:                              return "SW_WRONG_MAC";                 
        case SW_WRONG_PADDING:                          return "SW_WRONG_PADDING";             
        case SW_INTERNAL_LENGTH_WRONG:                  return "SW_INTERNAL_LENGTH_WRONG";     
        case SW_MASTER_BOARD_SLOT_BAD:                  return "SW_MASTER_BOARD_SLOT_BAD";
        case SW_LICENCE_MAX_COUNTER_REACHED:            return "SW_LICENCE_MAX_COUNTER_REACHED";
        case SW_HASH_DIFFERS2:                          return "SW_HASH_DIFFERS2";


        default: {
            if ((error & 0xFF00) == SW_BYTES_REMAINING_00)  {
                switch (error & 0x00FF) {
                    case 0: return "SW_BYTES_REMAINING_00";
                    case 1: return "SW_BYTES_REMAINING_01";
                    case 2: return "SW_BYTES_REMAINING_02";
                    case 3: return "SW_BYTES_REMAINING_03";
                    case 4: return "SW_BYTES_REMAINING_04";
                    case 5: return "SW_BYTES_REMAINING_05";
                    case 6: return "SW_BYTES_REMAINING_06";
                    case 7: return "SW_BYTES_REMAINING_07";
                    case 8: return "SW_BYTES_REMAINING_08";
                    case 9: return "SW_BYTES_REMAINING_09";
                    case 10: return "SW_BYTES_REMAINING_10";
                    case 11: return "SW_BYTES_REMAINING_11";
                    case 12: return "SW_BYTES_REMAINING_12";
                    case 13: return "SW_BYTES_REMAINING_13";
                    case 14: return "SW_BYTES_REMAINING_14";
                    case 15: return "SW_BYTES_REMAINING_15";
                    default: return "SW_BYTES_REMAINING_XX";
                }
            }
            if ((error & 0xFF00) == SW_CORRECT_LENGTH_00)   {
                switch (error & 0x00FF) {
                    case 0: return "SW_CORRECT_LENGTH_00";
                    case 1: return "SW_CORRECT_LENGTH_01";
                    case 2: return "SW_CORRECT_LENGTH_02";
                    case 3: return "SW_CORRECT_LENGTH_03";
                    case 4: return "SW_CORRECT_LENGTH_04";
                    case 5: return "SW_CORRECT_LENGTH_05";
                    case 6: return "SW_CORRECT_LENGTH_06";
                    case 7: return "SW_CORRECT_LENGTH_07";
                    case 8: return "SW_CORRECT_LENGTH_08";
                    case 9: return "SW_CORRECT_LENGTH_09";
                    case 10: return "SW_CORRECT_LENGTH_10";
                    case 11: return "SW_CORRECT_LENGTH_11";
                    case 12: return "SW_CORRECT_LENGTH_12";
                    case 13: return "SW_CORRECT_LENGTH_13";
                    case 14: return "SW_CORRECT_LENGTH_14";
                    case 15: return "SW_CORRECT_LENGTH_15";
                    default: return "SW_CORRECT_LENGTH_XX";
                }
            }            
                                                        
            // NO SPECIAL RULE MATCH                                            
            return "'unknown'";   
        }
    }
}  

