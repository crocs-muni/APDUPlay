/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.data;

/**
 *
 * @author Andrej
 */
public class Packet {
    private Node transmittedLeafNode;
    private Node receivedLeafNode;
    private int responseTime;
    private int ac;
      
    /**
     * Creates new instance with specified leafnode
     * 
     * @param transmittedLeafNode leaf node of transmitted data stream
     */
    public Packet(Node transmittedLeafNode) {
        this.transmittedLeafNode = transmittedLeafNode;
    }
    
    /**
     * Sets the leaf node of transmitted data stream
     * 
     * @param node leaf node of transmitted data stream
     */
    public void setTransmittedLeafNode(Node node) {
        this.transmittedLeafNode = node;
    }
    
    /**
     * Gets the leaf node of transmitted data stream
     * 
     * @return leaf node of transmitted data stream
     */
    public Node getTransmittedLeafNode() {
        return transmittedLeafNode;
    }
    
    /**
     * Sets the leaf node of received data stream
     * 
     * @param node leaf node of received data stream
     */
    public void setReceivedLeafNode(Node node) {
        this.receivedLeafNode = node;
    }
    
    /**
     * Gets the leaf node of received data stream
     * 
     * @return leaf node of received data stream
     */
    public Node getReceivedLeafNode() {
        return receivedLeafNode;
    }
    
    /**
     * Sets respnose time between transmitting and receiving data
     * 
     * @param time response time to be set
     */
    public void setResponseTime(int time) {
        responseTime = time;
    }
    
    /**
     * Gets respnose time between transmitting and receiving data
     * 
     * @return respnose time between transmitting and receiving data
     */
    public int getResponseTime() {
        return responseTime;
    }
    
    /**
     * Sets ac of transmitted and received data
     * 
     * @param ac ac of transmitted and received data
     */
    public void setAc(int ac) {
        this.ac = ac;
    }
    
    /**
     * Gets ac of transmitted and received data
     * 
     * @return ac of transmitted and received data
     */
    public int getAc() {
        return ac;
    }
}
