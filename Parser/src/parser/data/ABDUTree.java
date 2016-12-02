/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.data;

import java.nio.ByteBuffer;
import java.util.ArrayDeque;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import javax.xml.bind.DatatypeConverter;

/**
 *
 * @author Andrej
 */
public class ABDUTree {
    private int packetsCount;
    private ABDUPacket lastPacket;
    
    public final ABDUNode receivedRoot;
    public final ABDUNode root;
    public final String header;
    public final List<ABDUPacket> streamPackets;
    
    /**
     * Creates new instance of ABDUTree which represents packets data stream as a tree
     * 
     * @param header    packet header
     * @param data      packet data
     */
    public ABDUTree(byte[] header, byte[] data) {
        ByteBuffer wrapped = ByteBuffer.wrap(header);
        this.header = DatatypeConverter.printHexBinary(header);
        root = new ABDUNode(wrapped.array());
        ABDUNode node = new ABDUNode(data);
        root.addChild(node);
        receivedRoot = new ABDUNode(wrapped.array());
        streamPackets = new LinkedList<>();
        init();
    }
    
    /**
     * Merges byte stream into this packet tree
     * 
     * @param stream byte stream to merge
     */
    public void merge(byte[] stream) {
        ABDUNode node = merge(root, stream);
        lastPacket = new ABDUPacket(node);
        streamPackets.add(lastPacket);
        packetsCount++;
    }
    
    /**
     * Adds received data into this packet tree
     * 
     * @param data byte stream do add
     */
    public void addReceivedData(byte[] data) {
        ABDUNode node = merge(receivedRoot, data);
        if (node != null && lastPacket != null) {
            lastPacket.setReceivedLeafNode(node);
        }
    }
    
    public void setAdditionalInfoForLastPacket(int responseTime, int ac) {
        if (lastPacket != null) {
            lastPacket.setResponseTime(responseTime);
            lastPacket.setAc(ac);
        }
    }
    
    /**
     * Gets number of packets processed by this tree
     * 
     * @return number of processed packets
     */
    public int getPacketsCount() {
        return packetsCount;
    }
    
    /**
     * Divides tree nodes with multiple bytes to nodes with exactly 1 byte (except root/header node)
     */
    public void simplifyNodes() {
        simplifyNodes(root);
        simplifyNodes(receivedRoot);
    }
    
    private void init() {
        receivedRoot.setCount(0);
        packetsCount = 1;
        
        lastPacket = new ABDUPacket(root.getChildNodes().iterator().next());
        streamPackets.add(lastPacket);
    }
    
    private void simplifyNodes(ABDUNode node) {
        Queue<ABDUNode> queue = new ArrayDeque<>();
        queue.addAll(node.getChildNodes());
        while(!queue.isEmpty()) {
            node = queue.remove();
            byte[] data = node.getData();
            if (data.length > 1) {
                ABDUNode firstNode = new ABDUNode(Arrays.copyOfRange(data, 0, 1));
                ABDUNode lastNode = firstNode;
                for (int i = 1; i < data.length; i++) {
                    ABDUNode n = new ABDUNode(Arrays.copyOfRange(data, i, i + 1));
                    lastNode.setCount(node.getCount());
                    lastNode.addChild(n);
                    lastNode = n;
                }
                
                node.getParentNode().addChild(firstNode);
                node.setData(lastNode.getData());
                lastNode.getParentNode().addChild(node);
            }
            
            queue.addAll(node.getChildNodes());
        }
    }
    
    private ABDUNode merge(ABDUNode node, byte[] stream) {
        node.incrementCount();
        
        ABDUNode currentNode = node.findChildNode(stream[0]);
        if (currentNode == null) {
            currentNode = new ABDUNode(stream);
            node.addChild(currentNode);
            return currentNode;
        }
        
        byte[] data = currentNode.getData();
        int currentIndex = 0;
        ABDUNode lastNode = null;
        for (int i = 0; i < stream.length; i++) {
            if (currentIndex >= data.length) {
                lastNode = currentNode.findChildNode(stream[i]);
                if (lastNode == null) {
                    lastNode = new ABDUNode(Arrays.copyOfRange(stream, i, stream.length));
                    currentNode.addChild(lastNode);
                    break;
                }
                
                currentNode.incrementCount();
                currentIndex = 0;
                currentNode = lastNode;
                data = currentNode.getData();
            }
            
            if (data[currentIndex] != stream[i]) {
                currentNode.divide(currentIndex);
                lastNode = new ABDUNode(Arrays.copyOfRange(stream, i, stream.length));
                currentNode.getParentNode().addChild(lastNode);
                break;
            }
            
            currentIndex++;
        }
        
        currentNode.incrementCount();
        return lastNode != null ? lastNode : currentNode;
    }
}
