/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser;

import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 *
 * @author Andrej
 */
public class ABDUNode {
    private static int nextIdentifier = 0;
    
    private byte[] data;
    private ABDUNode parent;
    private final Map<Byte, ABDUNode> childNodes;
    private int count;
    
    public final int identifier;
    
    public ABDUNode(byte[] data) {
        this.data = data;
        childNodes = new HashMap<>();
        identifier = nextIdentifier++;
        count = 1;
    }
    
    public void incrementCount() {
        count++;
    }
    
    public void setCount(int count) {
        this.count = count;
    }
    
    public int getCount() {
        return count;
    }
    
    public void setData(byte[] data) {
        this.data = data;
    }
    
    public byte[] getData() {
        return data;
    }
    
    public void divide(int index) {
        // index cannot be higher than the length of data array
        // every node need to have value
        if (index >= data.length && index < 1) {
            throw new IllegalArgumentException("Index out of range of data array");
        }
        
        byte[] subArray = Arrays.copyOfRange(data, 0, index);
        ABDUNode node = new ABDUNode(subArray);
        
        parent.addChild(node);
        data = Arrays.copyOfRange(data, index, data.length);
        node.addChild(this);
    }
    
    public void addChild(ABDUNode node) {
        node.parent = this;
        childNodes.put(node.getData()[0], node);
    }
    
    public void addChildren(Collection<ABDUNode> nodes) {
        nodes.forEach((node) -> {
            addChild(node);
        });
    }
    
    public void setChildren(Collection<ABDUNode> nodes) {
        childNodes.clear();
        addChildren(nodes);
    }
    
    public boolean hasChildNodes() {
        return !childNodes.isEmpty();
    }
    
    public ABDUNode findChildNode(byte item) {
        return childNodes.get(item);
    }
    
    public Collection<ABDUNode> getChildNodes() {
        return childNodes.values();
    }
    
    public ABDUNode getParentNode() {
        return parent;
    }
}
