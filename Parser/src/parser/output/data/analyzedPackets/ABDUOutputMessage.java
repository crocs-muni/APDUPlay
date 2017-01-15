/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.output.data.analyzedPackets;

import java.util.Objects;

/**
 *
 * @author Andrej
 */
public class ABDUOutputMessage {
    public final String message;
    public final int identifier;
    
    public ABDUOutputMessage(String message, int identifier) {
        this.message = message;
        this.identifier = identifier;
    }
    
    @Override
    public boolean equals(Object obj) {
        if (obj == null) {
            return false;
        }
        
        if (obj == this) {
            return true;
        }
        
        if (!(obj instanceof ABDUOutputMessage)) {
            return false;
        }

        ABDUOutputMessage msg = (ABDUOutputMessage)obj;
        return (msg.message == null ? message == null : msg.message.equals(message));
    }

    @Override
    public int hashCode() {
        int hash = 3;
        hash = 97 * hash + Objects.hashCode(this.message);
        return hash;
    }
}
