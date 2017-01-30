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
    
    public Packet() { }
    
    public Packet(Node transmittedLeafNode) {
        this.transmittedLeafNode = transmittedLeafNode;
    }
    
    public void setTransmittedLeafNode(Node node) {
        this.transmittedLeafNode = node;
    }
    
    public Node getTransmittedLeafNode() {
        return transmittedLeafNode;
    }
    
    public void setReceivedLeafNode(Node node) {
        this.receivedLeafNode = node;
    }
    
    public Node getReceivedLeafNode() {
        return receivedLeafNode;
    }
    
    public void setResponseTime(int time) {
        responseTime = time;
    }
    
    public int getResponseTime() {
        return responseTime;
    }
    
    public void setAc(int ac) {
        this.ac = ac;
    }
    
    public int getAc() {
        return ac;
    }
}
