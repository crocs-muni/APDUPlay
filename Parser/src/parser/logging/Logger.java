/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.logging;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
/**
 *
 * @author Andrej
 */
public class Logger implements ILogger {
    
    private final String fileName;
    
    public Logger(String fileName) {
        this.fileName = fileName;
    }
    
    @Override
    public void error(String message) {
        writeMessage("ERROR", message);
    }
    
    @Override
    public void warning(String message) {
        writeMessage("WARNING", message);
    }
    
    private void writeMessage(String info, String message) {
        try (PrintWriter writer = new PrintWriter(new BufferedWriter(new FileWriter(fileName, true)))) {
            writer.print(info);
            writer.print(": ");
            writer.println(message);
        } catch (IOException ex) {
            
        }
    }
}
