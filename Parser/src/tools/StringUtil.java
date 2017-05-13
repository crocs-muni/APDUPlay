/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package tools;



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
     * @return wrapped text
     */
    public static String wrapText(String str, int wrapAfter) {
        if (wrapAfter <= 0) {
            return str;
        }
        
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < str.length(); i += wrapAfter) {
            if (str.length() < i + wrapAfter) {
                break;
            }
            
            sb.append(str.substring(i, i + wrapAfter));
            
            if (str.length() != i + wrapAfter) {
                trimEnd(sb).append("\n");
            }
        }
        
        int missingChars = str.length() % wrapAfter;
        if (missingChars > 0) {
            sb.append(str.substring(str.length() - missingChars));
        }
        
        return sb.toString();
    }
    
    /**
     * Trims trailing whitespace of StringBuilder
     * 
     * @param sb StringBuilder to be trimmed
     * @return this StringBuilder
     */
    public static StringBuilder trimEnd(StringBuilder sb)
    {
        if (sb == null || sb.length() == 0) {
            return sb;
        }

        int i = sb.length();
        for (; i > 0; i--) {
            if (!Character.isWhitespace(sb.charAt(i - 1))) {
                break;
            }
        }

        if (i < sb.length()) {
            sb.delete(i, sb.length());
        }
        
        return sb;
    }
}
