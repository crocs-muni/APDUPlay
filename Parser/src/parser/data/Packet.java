/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.data;

import lombok.Getter;
import lombok.Setter;

/**
 *
 * @author Andrej
 */
public class Packet {
    
    /**
     * Represents the leaf node of transmitted data stream
     * 
     * @param transmittedLeafNode leaf node of transmitted data stream
     * @return leaf node of transmitted data stream
     */
    @Getter @Setter private Node transmittedLeafNode;
    
    /**
     * Represents the leaf node of received data stream
     * 
     * @param receivedLeafNode leaf node of received data stream
     * @return leaf node of received data stream
     */
    @Getter @Setter private Node receivedLeafNode;
    
    /**
     * Represents respnose time between transmitting and receiving data
     * 
     * @param time response time to be set
     * @return respnose time between transmitting and receiving data
     */
    @Getter @Setter private int responseTime;
    
    /**
     * Represents ac of transmitted and received data
     * 
     * @param ac ac of transmitted and received data
     * @return ac of transmitted and received data
     */
    @Getter @Setter private int ac;
      
    /**
     * Creates new instance with specified leafnode
     * 
     * @param transmittedLeafNode leaf node of transmitted data stream
     */
    public Packet(Node transmittedLeafNode) {
        this.transmittedLeafNode = transmittedLeafNode;
    }
}
