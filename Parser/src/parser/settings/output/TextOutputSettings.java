/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.settings.output;

import lombok.Getter;
import lombok.Setter;

/**
 *
 * @author Andrej
 */
public class TextOutputSettings {
    
    /**
     * Represents separator between bytes in data stream
     * 
     * @param bytesSeparator bytes separator to be set
     * @return bytes separator
     */
    @Getter @Setter private String bytesSeparator;
    
    /**
     * Represents separator for byte with multiple values if not analyzed as random
     * 
     * @param byteEnumerationSeparator byte enumeration separator to be set
     * @return byte enumeration separator
     */
    @Getter @Setter private String byteEnumerationSeparator;
    
    /**
     * Represents the value of random byte in output
     * 
     * @param randomByteValue byte value to be set as random
     * @return value indicating that the byte is random
     */
    @Getter @Setter private String randomByteValue;
    
    /**
     * Represents the value of byte that has no value as the data streams differ in length
     * 
     * @param emptyByteValue byte value to be set as none
     * @return value indicating that the byte is missing
     */
    @Getter @Setter private String emptyByteValue;
    
    /**
     * Defines if empty byte should be included in the output
     * 
     * @param emptyByteIncluded
     * @return true if empty byte should be included in the output, otherwise false
     */
    @Getter @Setter private boolean emptyByteIncluded;
}
