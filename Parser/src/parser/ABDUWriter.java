/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser;

import parser.settings.ABDUSettings;
import java.io.File;
import java.io.PrintWriter;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Queue;
import java.util.Stack;
import javax.xml.bind.DatatypeConverter;
import parser.settings.ABDUOutputType;

/**
 *
 * @author Andrej
 */
public class ABDUWriter {
    
    private final ABDUSettings settings;
    private final ABDULogger logger;
    
    public ABDUWriter(ABDUSettings settings, ABDULogger logger) {
        this.settings = settings;
        this.logger = logger;
    }
    
    public void write(Collection<ABDUTree> packets) {
        File file = new File(settings.getOutputDirectory());
        file.mkdirs();

        if (settings.simpleNodes()) {
            packets.forEach((tree) -> tree.simplifyNodes());
        }
        
        if (!settings.separatePackets()) {
            writeInOneFile(packets, file.getAbsolutePath());
        } else {
            writeSeparated(packets, file.getAbsolutePath());
        }
    }
    
    private void writeInOneFile(Collection<ABDUTree> packets, String directoryPath) {
        try (PrintWriter writer = new PrintWriter(directoryPath + "/packets.dot", "UTF-8")) {
                writer.println("digraph packets {");
                packets.forEach((tree) -> {
                    if ((settings.getOutputTypeMask() & ABDUOutputType.NODES) != 0) {
                        printTransmitted(tree, writer);
                        printReceived(tree, writer);
                    }
                });
                writer.println("}");
            }
            catch (Exception ex) {
                logger.error(ex.getMessage());
            }
    }
    
    private void writeSeparated(Collection<ABDUTree> packets, String directoryPath) {
        packets.forEach((tree) -> {
            tree.simplifyNodes();
            try (PrintWriter writer = new PrintWriter(directoryPath + "/" + tree.header + "_transmitted.dot", "UTF-8")) {
                writer.println("digraph transmitted {");
                printTransmitted(tree, writer);
                writer.println("}");
            }
            catch (Exception ex) {
                logger.error(ex.getMessage());
            }
            
            try (PrintWriter writer = new PrintWriter(directoryPath + "/" + tree.header + "_received.dot", "UTF-8")) {
                writer.println("digraph received {");
                printTransmitted(tree, writer);
                writer.println("}");
            }
            catch (Exception ex) {
                logger.error(ex.getMessage());
            }
        });
    }
    
    private void printTransmitted(ABDUTree tree, PrintWriter writer) {
        printLabels(tree.root, writer);
        print(tree.root, writer);
    }
    
    private void printReceived(ABDUTree tree, PrintWriter writer) {
        printLabels(tree.receivedRoot, writer);
        print(tree.receivedRoot, writer);
    }
    
    private void printTransmittedFlow(ABDUTree tree, PrintWriter writer) {
        printFlow(tree.root, tree.getPacketsCount(), writer);
    }
    
    private void printReceivedFlow(ABDUTree tree, PrintWriter writer) {
        printFlow(tree.receivedRoot, tree.getPacketsCount(), writer);
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
    
    private void printFlow(ABDUNode node, int packetsCount, PrintWriter writer) {
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
}
