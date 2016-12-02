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
public class ABDUPacket {
    private ABDUNode transmittedLeafNode;
    private ABDUNode receivedLeafNode;
    private int responseTime;
    private int ac;
    
    public ABDUPacket() { }
    
    public ABDUPacket(ABDUNode transmittedLeafNode) {
        this.transmittedLeafNode = transmittedLeafNode;
    }
    
    public void setTransmittedLeafNode(ABDUNode node) {
        this.transmittedLeafNode = node;
    }
    
    public ABDUNode getTransmittedLeafNode() {
        return transmittedLeafNode;
    }
    
    public void setReceivedLeafNode(ABDUNode node) {
        this.receivedLeafNode = node;
    }
    
    public ABDUNode getReceivedLeafNode() {
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
