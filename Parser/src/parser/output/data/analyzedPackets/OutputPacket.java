/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.output.data.analyzedPackets;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 *
 * @author Andrej
 */
public class OutputPacket {
    private OutputMessage transmittedMessage;
    private List<OutputMessage> receivedMessages;
    
    /**
     * Creates new instance of OutputPacket
     * 
     * @param transmittedMessage transmitted message from packet
     */
    public OutputPacket(OutputMessage transmittedMessage) {
        this.transmittedMessage = transmittedMessage;
        receivedMessages = new ArrayList<>();
    }
    
    public OutputMessage getTransmittedMessage() {
        return transmittedMessage;
    }
    
    public void setTransmittedMessage(OutputMessage msg) {
        transmittedMessage = msg;
    }
    
    public List<OutputMessage> getReceivedMessages() {
        return receivedMessages;
    }
    
    public void setReceivedMessages(List<OutputMessage> msgs) {
        receivedMessages = msgs;
    }
    
    /**
     * Adds received message to this packet
     * 
     * @param msg message to be add
     */
    public void addReceivedMessage(OutputMessage msg) {
        int index = receivedMessages.indexOf(msg);
        if (index == -1) {
            receivedMessages.add(msg);
            return;
        }
        
        receivedMessages.get(index).increaseCount(msg.getCount());
    }
    
    /**
     * Adds multiple received messages to this packet
     * 
     * @param msgs messages to be add
     */
    public void addReceivedMessages(List<OutputMessage> msgs) {
        msgs.forEach(msg -> addReceivedMessage(msg));
    }
    
    @Override
    public boolean equals(Object obj) {
        if (obj == null) {
            return false;
        }
        
        if (obj == this) {
            return true;
        }
        
        if (!(obj instanceof OutputPacket)) {
            return false;
        }

        OutputPacket packet = (OutputPacket)obj;
        return (packet.transmittedMessage == null ? transmittedMessage == null : packet.transmittedMessage.equals(transmittedMessage));
    }

    @Override
    public int hashCode() {
        int hash = 3;
        hash = 29 * hash + Objects.hashCode(this.transmittedMessage);
        return hash;
    }

}
