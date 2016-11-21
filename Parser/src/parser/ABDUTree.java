/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser;

import java.io.PrintWriter;
import java.nio.ByteBuffer;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Queue;
import java.util.Set;
import java.util.Stack;
import javafx.util.Pair;
import javax.xml.bind.DatatypeConverter;

/**
 *
 * @author Andrej
 */
public class ABDUTree {
    private int packetsCount;
    private final ABDUNode receivedRoot;
    private final ABDUNode root;
    public final short header;
    
    public ABDUTree(byte[] stream) {
        ByteBuffer wrapped = ByteBuffer.wrap(Arrays.copyOfRange(stream, 0, 2));
        header = wrapped.getShort();
        root = new ABDUNode(wrapped.array());
        root.addChild(new ABDUNode(Arrays.copyOfRange(stream, 2, stream.length)));
        receivedRoot = new ABDUNode(wrapped.array());
        receivedRoot.setCount(0);
        packetsCount = 1;
    }
    
    public ABDUTree(byte[] header, byte[] data) {
        ByteBuffer wrapped = ByteBuffer.wrap(header);
        this.header = wrapped.getShort();
        root = new ABDUNode(wrapped.array());
        root.addChild(new ABDUNode(data));
        receivedRoot = new ABDUNode(wrapped.array());
        receivedRoot.setCount(0);
        packetsCount = 1;
    }
    
    public void merge(byte[] stream) {
        merge(root, stream);
        packetsCount++;
    }
    
    public void addReceivedData(byte[] data) {
        merge(receivedRoot, data);
    }
    
    public int getPacketsCount() {
        return packetsCount;
    }
    
    public void printTransmitted(PrintWriter writer) {
        writer.println("digraph transmitted {");
        printLabels(root, writer);
        print(root, writer);
        writer.println("}");
    }
    
    public void printReceived(PrintWriter writer) {
        writer.println("digraph received {");
        printLabels(receivedRoot, writer);
        print(receivedRoot, writer);
        writer.println("}");
    }
    
    public void simplifyNodes() {
        simplifyNodes(root);
        simplifyNodes(receivedRoot);
    }
    
    public void printTransmittedFlow(PrintWriter writer) {
        writer.println("digraph transmitted {");
        printFlow(root, writer);
        writer.println("}");
    }
    
    public void printReceivedFlow(PrintWriter writer) {
        writer.println("digraph received {");
        printFlow(receivedRoot, writer);
        writer.println("}");
    }
    
    private void printFlow(ABDUNode node, PrintWriter writer) {
        List<ABDUNode> nodes = new ArrayList<>();
        List<ABDUNode> childNodes = new ArrayList<>();
        StringBuilder labels = new StringBuilder();
        StringBuilder graph = new StringBuilder();
        nodes.add(node);
        int index = 0;
        while(!nodes.isEmpty()) {
            Map<String, Integer> map = new HashMap<>();
            for (int i = 0; i < nodes.size(); i++) {
                node = nodes.get(i);
                childNodes.addAll(node.getChildNodes());
                String val = DatatypeConverter.printHexBinary(node.getData());
                Integer count = map.get(val);
                if (count != null) {
                    map.put(val, count + node.getCount());
                    continue;
                }

                map.put(val, node.getCount());
            }
            
            Entry<String, Integer> max = Collections.max(map.entrySet(), (Entry<String, Integer> o1, Entry<String, Integer> o2) -> o1.getValue().compareTo(o2.getValue()));
            String hexColor = String.format("#%06X", 0xFFFFFF & java.awt.Color.HSBtoRGB((float)max.getValue() / packetsCount / 3f, 1f, 1f));
            labels.append(String.format("\t%d [label=\"%s\" style=filled fillcolor=\"%s\"];%s", index, max.getKey(), hexColor, System.lineSeparator()));
            graph.append(String.format("%d -> ", index++));
            
            nodes = childNodes;
            childNodes = new ArrayList<>();
        }
        
        writer.println(labels.toString());
        writer.println(String.format("\t%s;", graph.substring(0, graph.length() - 4)));
    }
    
    private void simplifyNodes(ABDUNode node) {
        Queue<ABDUNode> queue = new ArrayDeque<>();
        queue.add(node);
        while(!queue.isEmpty()) {
            node = queue.remove();
            Collection<ABDUNode> childNodes = node.getChildNodes();
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
                
                lastNode.setCount(node.getCount());
                lastNode.addChildren(childNodes);
                node.setData(firstNode.getData());
                node.setChildren(firstNode.getChildNodes());
            }
            
            queue.addAll(childNodes);
        }
    }
    
    private void print(ABDUNode node, PrintWriter writer) {
        Stack<ABDUNode> stack = new Stack<>();
        stack.add(node);
        
        List<ABDUNode> toPrint = new ArrayList<>();
        while(!stack.isEmpty()) {
            node = stack.pop();
            toPrint.add(node);
            if (node.hasChildNodes()) {
                stack.addAll(node.getChildNodes());
                continue;
            }
            
            writer.print("\t");
            for (int i = 0; i < toPrint.size() - 1; i++) {
                writer.print(toPrint.get(i).identifier + " -> ");
            }
            
            writer.println(toPrint.get(toPrint.size() - 1).identifier + ";");
            toPrint.clear();
            if (!stack.isEmpty()) {
                toPrint.add(stack.peek().getParentNode());
            }
        }
    }
    
    private void printLabels(ABDUNode node, PrintWriter writer) {
        Queue<ABDUNode> queue = new ArrayDeque<>();
        queue.add(node);
        while(!queue.isEmpty()) {
            node = queue.remove();
            writer.println(String.format("\t%d [label=\"%s\"]", node.identifier, DatatypeConverter.printHexBinary(node.getData())));
            queue.addAll(node.getChildNodes());
        }
    }
    
    private void merge(ABDUNode node, byte[] stream) {
        node.incrementCount();
        
        ABDUNode currentNode = node.findChildNode(stream[0]);
        if (currentNode == null) {
            node.addChild(new ABDUNode(stream));
            return;
        }
        
        byte[] data = currentNode.getData();
        int currentIndex = 0;
        for (int i = 0; i < stream.length; i++) {
            if (currentIndex >= data.length) {
                ABDUNode nextNode = currentNode.findChildNode(stream[i]);
                if (nextNode == null) {
                    nextNode = new ABDUNode(Arrays.copyOfRange(stream, i, stream.length));
                    currentNode.addChild(nextNode);
                    break;
                }
                
                currentNode.incrementCount();
                currentIndex = 0;
                currentNode = nextNode;
                data = currentNode.getData();
            }
            
            if (data[currentIndex] != stream[i]) {
                if (currentIndex != 0) {
                    currentNode.divide(currentIndex);
                }
                ABDUNode nextNode = new ABDUNode(Arrays.copyOfRange(stream, i, stream.length));
                currentNode.addChild(nextNode);
                break;
            }
            
            currentIndex++;
        }
        
        currentNode.incrementCount();
    }
}
