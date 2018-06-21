/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.output.data;

import java.util.Objects;
import lombok.Getter;
import lombok.val;

/**
 *
 * @author Andrej
 */
public class OutputMessage {
    public final String message;
    public final int identifier;
    
    /**
     * Gets count of occurrence of message
     * 
     * @return count of occurrence of message
     */
    @Getter private int count;
    
    /**
     * Creates new instance of OutputMessage
     * 
     * @param message packet message
     * @param identifier node identifier
     */
    public OutputMessage(String message, int identifier) {
        this.message = message;
        this.identifier = identifier;
        count = 1;
    }
    
    /**
     * Increases message number of occurences
     * 
     * @param amount amount to be increased by
     */
    public void increaseCount(int amount) {
        count += amount;
    }
    
    @Override
    public boolean equals(Object obj) {
        if (obj == null) {
            return false;
        }
        
        if (obj == this) {
            return true;
        }
        
        if (!(obj instanceof OutputMessage)) {
            return false;
        }

        val msg = (OutputMessage)obj;
        return (msg.message == null ? message == null : msg.message.equals(message));
    }

    @Override
    public int hashCode() {
        int hash = 3;
        hash = 97 * hash + Objects.hashCode(this.message);
        return hash;
    }
}
