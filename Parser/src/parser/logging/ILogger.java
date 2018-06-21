/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.logging;

/**
 *
 * @author Andrej
 */
public interface ILogger {
    /**
     * Logs error message
     * 
     * @param message message to log
     */
    void error(String message);
    
    /**
     * Logs warning message
     * 
     * @param message message to log
     */
    void warning(String message);
}
