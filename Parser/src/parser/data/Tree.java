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
import lombok.val;

/**
 *
 * @author Andrej
 */
public class Tree {
    private int packetsCount;
    private Packet lastPacket;
    
    public final Node receivedRoot;
    public final Node root;
    public final String header;
    public final List<Packet> streamPackets;
    
    /**
     * Creates new instance of Tree which represents packets data stream as a tree
     * 
     * @param header    packet header
     * @param data      packet data
     */
    public Tree(byte[] header, byte[] data) {
        val wrapped = ByteBuffer.wrap(header);
        this.header = DatatypeConverter.printHexBinary(header);
        root = new Node(wrapped.array());
        val node = new Node(data);
        root.addChild(node);
        receivedRoot = new Node(wrapped.array());
        streamPackets = new LinkedList<>();
        init();
    }
    
    /**
     * Merges byte stream into this packet tree
     * 
     * @param stream byte stream to merge
     */
    public void merge(byte[] stream) {
        val node = merge(root, stream);
        lastPacket = new Packet(node);
        streamPackets.add(lastPacket);
        packetsCount++;
    }
    
    /**
     * Adds received data into this packet tree
     * 
     * @param data byte stream do add
     */
    public void addReceivedData(byte[] data) {
        val node = merge(receivedRoot, data);
        if (node != null && lastPacket != null) {
            lastPacket.setReceivedLeafNode(node);
        }
    }
    
    /**
     * Sets respnose time and ac for tree to last packet added
     * 
     * @param responseTime response time to be set
     * @param ac ac to be set
     */
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
        
        lastPacket = new Packet(root.getChildNodes().iterator().next());
        streamPackets.add(lastPacket);
    }
    
    private void simplifyNodes(Node node) {
        Queue<Node> queue = new ArrayDeque<>();
        queue.addAll(node.getChildNodes());
        while(!queue.isEmpty()) {
            node = queue.remove();
            val data = node.getData();
            if (data.length > 1) {
                Node firstNode = new Node(Arrays.copyOfRange(data, 0, 1));
                Node lastNode = firstNode;
                for (int i = 1; i < data.length; i++) {
                    Node n = new Node(Arrays.copyOfRange(data, i, i + 1));
                    lastNode.setCount(node.getCount());
                    lastNode.addChild(n);
                    lastNode = n;
                }
                
                node.getParent().addChild(firstNode);
                node.setData(lastNode.getData());
                lastNode.getParent().addChild(node);
            }
            
            queue.addAll(node.getChildNodes());
        }
    }
    
    private Node merge(Node node, byte[] stream) {
        node.incrementCount();
        
        Node currentNode = node.findChildNode(stream[0]);
        if (currentNode == null) {
            currentNode = new Node(stream);
            node.addChild(currentNode);
            return currentNode;
        }
        
        byte[] data = currentNode.getData();
        int currentIndex = 0;
        Node lastNode = null;
        for (int i = 0; i < stream.length; i++) {
            if (currentIndex >= data.length) {
                lastNode = currentNode.findChildNode(stream[i]);
                if (lastNode == null) {
                    lastNode = new Node(Arrays.copyOfRange(stream, i, stream.length));
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
                lastNode = new Node(Arrays.copyOfRange(stream, i, stream.length));
                currentNode.getParent().addChild(lastNode);
                break;
            }
            
            currentIndex++;
        }
        
        currentNode.incrementCount();
        return lastNode != null ? lastNode : currentNode;
    }
}
