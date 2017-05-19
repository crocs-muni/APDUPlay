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
     * 
     * @param str text to be wrapped
     * @param wrapAfter number of chars after which the new line will be appended
     * @param firstWrap first wrap point
     * @return wrapped text with remaining wrap after
     */
    public static Pair<String, Integer> wrapText(String str, int wrapAfter, int firstWrap) {
        if (wrapAfter <= 0) {
            return Pair.of(str, wrapAfter);
        }
        
        if (firstWrap < 0) {
            firstWrap = wrapAfter;
        }
        
        val sb = new StringBuilder();
        if (str.length() < firstWrap) {
            sb.append(str);
            return Pair.of(sb.toString(), firstWrap - str.length()); 
        }
        
        sb.append(str.substring(0, firstWrap));
        if (str.length() != firstWrap) {
            sb.append("<br/>");
        }
        
        for (int i = firstWrap; i < str.length(); i += wrapAfter) {
            if (str.length() < i + wrapAfter) {
                break;
            }
            
            sb.append(str.substring(i, i + wrapAfter));
            
            if (str.length() != i + wrapAfter) {
                sb.append("<br/>");
            }
        }
        
        val missingChars = (str.length() - firstWrap) % wrapAfter;
        if (missingChars > 0) {
            sb.append(str.substring(str.length() - missingChars));
            return Pair.of(sb.toString(), wrapAfter - missingChars); 
        }
        
        return Pair.of(sb.toString(), 0);
    }
}
