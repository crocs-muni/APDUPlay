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
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
/**
 *
 * @author Andrej
 */
public class Logger implements ILogger {
    
    private final String fileName;
    private final DateTimeFormatter defaultdateTimeFormatter = DateTimeFormatter.ofPattern("yyyy-MM-dd hh:mm:ss");
    private DateTimeFormatter dateTimeFormatter;
    
    /**
     * Creates new instance of Logger
     * 
     * @param fileName absolute or relative path to desired output file
     */
    public Logger(String fileName) {
        this.fileName = fileName;
        this.dateTimeFormatter = defaultdateTimeFormatter;
    }
    
    /**
     * Creates new instance of Logger
     * 
     * @param fileName absolute or relative path to desired output file
     * @param formatter DateTimeFormatter to be used for date in logs
     */
    public Logger(String fileName, DateTimeFormatter formatter) {
        this.fileName = fileName;
        this.dateTimeFormatter = formatter != null ? formatter : defaultdateTimeFormatter;
    }
    
    /**
     * Sets date time formatter
     * 
     * @param formatter DateTimeFormatter to be used for date in logs
     */
    public void setDateTimeFormatter(DateTimeFormatter formatter) {
        this.dateTimeFormatter = formatter != null ? formatter : defaultdateTimeFormatter;
    }
    
    /**
     * Sets date time pattern
     * 
     * @param pattern used to format date in logs
     */
    public void setDateTimePattern(String pattern) {
        this.dateTimeFormatter = DateTimeFormatter.ofPattern(pattern);
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
            writer.print(String.format("[%s] ", LocalDateTime.now().format(dateTimeFormatter)));
            writer.print(info);
            writer.print(": ");
            writer.println(message);
        } catch (IOException ex) {
            
        }
    }
}
