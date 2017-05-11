/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package tools;

import lombok.val;


/**
 *
 * @author Andrej
 */
public class StringUtil {
    
    /**
     * Wraps text
     * @param str text to be wrapped
     * @param wrapAfter number of chars after which the new line will be appended
     * @return wrapped text
     */
    public static String wrapText(String str, int wrapAfter) {
        if (wrapAfter <= 0)
            return str;
        
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < str.length(); i += wrapAfter) {
            if (str.length() < i + wrapAfter) {
                break;
            }
            
            sb.append(str.substring(i, i + wrapAfter));
            
            if (str.length() != i + wrapAfter) {
                sb.append("\n");
            }
        }
        
        val missingChars = str.length() % wrapAfter;
        if (missingChars > 0) {
            sb.append(str.substring(str.length() - missingChars));
        }
        
        return sb.toString();
    }
}
