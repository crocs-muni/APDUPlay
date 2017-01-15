/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.output;

import parser.output.data.analyzedPackets.ABDUOutputPacket;
import parser.output.data.analyzedPackets.ABDUOutputTree;
import parser.settings.ABDUSettings;
import java.io.File;
import java.io.PrintWriter;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Queue;
import java.util.Stack;
import parser.ABDULogger;
import parser.data.ABDUNode;
import parser.data.ABDUPacket;
import parser.data.ABDUTree;
import parser.output.data.analyzedPackets.ABDUOutputMessage;
import parser.settings.graph.ABDUGraphSettings;

/**
 *
 * @author Andrej
 */
public class ABDUWriter {
    
    private static int flowIndex = 0;
    private final ABDUSettings settings;
    private final ABDULogger logger;
    
    /**
     * Creates new instance of ABDUWriter
     * 
     * @param settings
     * @param logger
     */
    public ABDUWriter(ABDUSettings settings, ABDULogger logger) {
        this.settings = settings;
        this.logger = logger;
    }
    
    /**
     * Writes packets into a file
     * 
     * @param packets packets to write
     */
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
    
    private List<ABDUOutputFunction> getOutputFunctions() {
        List<ABDUOutputFunction> functions = new ArrayList<>();
        for(int type : ABDUOutputType.TYPES) {
            if ((settings.getOutputTypeMask() & type) != 0) {
                switch(type) {
                    case ABDUOutputType.NODES:
                        functions.add(new ABDUOutputFunction((tree, writer) -> printTransmitted(tree, writer), (tree, writer) -> printReceived(tree, writer)));
                        break;
                    case ABDUOutputType.FLOW:
                        functions.add(new ABDUOutputFunction((tree, writer) -> printTransmittedFlow(tree, writer), (tree, writer) -> printReceivedFlow(tree, writer)));
                        break;
                    case ABDUOutputType.PACKETS:
                        functions.add(new ABDUOutputFunction((tree, writer) -> printPackets(tree, writer), null));
                        break;
                    case ABDUOutputType.PACKETS_ANALYZED:
                        functions.add(new ABDUOutputFunction((tree, writer) -> printAnalyzedPackets(tree, writer), null));
                        break;
                }
            }
        }
        
        return functions;
    }
    
    private void writeInOneFile(Collection<ABDUTree> packets, String directoryPath) {
        List<ABDUOutputFunction> outputFunctions = getOutputFunctions();
        
        for (int i = 0; i < outputFunctions.size(); i++) {
            if (outputFunctions.get(i).hasTransmittedFunction()) {
                try (PrintWriter writer = new PrintWriter(String.format("%s/packets_transmitted(%d).dot", directoryPath, i), "UTF-8")) {
                    writer.println("digraph packets {");
                    printGraphSettings(writer);
                    for (ABDUTree tree : packets) {
                        outputFunctions.get(i).invokeTransmitted(tree, writer);
                    }
                    writer.println("}");
                }
                catch (Exception ex) {
                    logger.error(ex.getMessage());
                }
            }
            
            if (outputFunctions.get(i).hasReceivedFunction()) {
                try (PrintWriter writer = new PrintWriter(String.format("%s/packets_received(%d).dot",directoryPath, i), "UTF-8")) {
                    writer.println("digraph packets {");
                    printGraphSettings(writer);
                    for (ABDUTree tree : packets) {
                        outputFunctions.get(i).invokeReceived(tree, writer);
                    }
                    writer.println("}");
                }
                catch (Exception ex) {
                    logger.error(ex.getMessage());
                }
            }
        }
    }
    
    private void writeSeparated(Collection<ABDUTree> packets, String directoryPath) {
        List<ABDUOutputFunction> outputFunctions = getOutputFunctions();
        for (int i = 0; i < outputFunctions.size(); i++) {
            for (ABDUTree tree : packets) {
                tree.simplifyNodes();
                
                if (outputFunctions.get(i).hasTransmittedFunction()) {
                    try (PrintWriter writer = new PrintWriter(String.format("%s/%s_transmitted(%d).dot", directoryPath, tree.header, i), "UTF-8")) {
                        writer.println("digraph transmitted {");
                        printGraphSettings(writer);
                        printTransmitted(tree, writer);
                        writer.println("}");
                    }
                    catch (Exception ex) {
                        logger.error(ex.getMessage());
                    }
                }

                if (outputFunctions.get(i).hasReceivedFunction()) {
                    try (PrintWriter writer = new PrintWriter(String.format("%s/%s_received(%d).dot", directoryPath, tree.header, i), "UTF-8")) {
                        writer.println("digraph received {");
                        printGraphSettings(writer);
                        printTransmitted(tree, writer);
                        writer.println("}");
                    }
                    catch (Exception ex) {
                        logger.error(ex.getMessage());
                    }
                }
            }
        }
    }
    
    private void printPackets(ABDUTree tree, PrintWriter writer) {
        writer.println(String.format("\t%d [label=\"%s\"];", tree.root.identifier, toHexBinaryString(tree.root.getData())));
        
        Map<String, Map<String, List<ABDUPacket>>> streams = new HashMap<>();
        tree.streamPackets.forEach((packet) -> {
            byte[] transmitted = getDataFromLeafNode(packet.getTransmittedLeafNode());
            byte[] received = getDataFromLeafNode(packet.getReceivedLeafNode());
            
            String transmittedStr = toHexBinaryString(Arrays.copyOfRange(transmitted, settings.getHeaderLength(), transmitted.length));
            String receivedStr = toHexBinaryString(Arrays.copyOfRange(received, settings.getHeaderLength(), received.length));
            
            Map<String, List<ABDUPacket>> val = streams.get(transmittedStr);
            if (val != null) {
                List<ABDUPacket> p = val.get(receivedStr);
                if (p == null) {
                    p = new LinkedList<>();
                    val.put(receivedStr, p);
                }
                p.add(packet);
            } else {
                val = new HashMap<>();
                List<ABDUPacket> p = new LinkedList<>();
                p.add(packet);
                val.put(receivedStr, p);
                streams.put(transmittedStr, val);
            }
        });
        
        streams.entrySet().forEach((item) -> {
            // To generate identifier
            ABDUNode transmittedNode = new ABDUNode(null);
            writer.println(String.format("\t%d [label=\"%s\"];", transmittedNode.identifier, item.getKey()));
            
            item.getValue().forEach((received, packets) -> {
                ABDUNode receivedNode = new ABDUNode(null);
                writer.println(String.format("\t%d [label=\"%s\"];", receivedNode.identifier, received));
                
                packets.forEach((packet) -> {
                    writer.println(String.format("\t%d -> %d [label=\"[ac=%d]\"];", tree.root.identifier, transmittedNode.identifier, packet.getAc()));
                    writer.println(String.format("\t%d -> %d [label=\"[ac=%d, time=%d]\"];", transmittedNode.identifier, receivedNode.identifier, packet.getAc(), packet.getResponseTime()));
                });
            });
        });
    }
    
    private void printAnalyzedPackets(ABDUTree tree, PrintWriter writer) {
        ABDUOutputTree outputTree = new ABDUOutputTree(toHexBinaryString(tree.root.getData()), tree.root.identifier);
        
        tree.streamPackets.forEach((packet) -> {
            byte[] transmitted = getDataFromLeafNode(packet.getTransmittedLeafNode());
            byte[] received = getDataFromLeafNode(packet.getReceivedLeafNode());
            
            String transmittedStr = toHexBinaryString(Arrays.copyOfRange(transmitted, settings.getHeaderLength(), transmitted.length));
            String receivedStr = toHexBinaryString(Arrays.copyOfRange(received, settings.getHeaderLength(), received.length));
            
            ABDUOutputPacket p = new ABDUOutputPacket(new ABDUOutputMessage(transmittedStr, packet.getTransmittedLeafNode().identifier));
            p.addReceivedMessage(new ABDUOutputMessage(receivedStr, packet.getReceivedLeafNode().identifier));
            
            outputTree.addPacket(p);
        });
        
        writer.println(outputTree.prepareOutput());
    }
    
    private byte[] getDataFromLeafNode(ABDUNode node) {
        List<byte[]> data = new ArrayList<>();
        
        int size = 0;
        while (node != null) {
            data.add(node.getData());    
            size += node.getData().length;
            node = node.getParentNode();
        }
        
        byte[] array = new byte[size];
        size = 0;
        for (int i = data.size() - 1; i >= 0; i--) {
            for (byte b : data.get(i)) {
                array[size++] = b;
            }
        }
        
        return array;
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
            writer.println(String.format("\t%d [label=\"%s\"];", node.identifier, toHexBinaryString(node.getData())));
            queue.addAll(node.getChildNodes());
        }
    }
    
    private void printFlow(ABDUNode node, int packetsCount, PrintWriter writer) {
        List<ABDUNode> nodes = new ArrayList<>();
        List<ABDUNode> childNodes = new ArrayList<>();
        StringBuilder labels = new StringBuilder();
        StringBuilder graph = new StringBuilder();
        nodes.add(node);
        while(!nodes.isEmpty()) {
            Map<String, Integer> map = new HashMap<>();
            for (int i = 0; i < nodes.size(); i++) {
                node = nodes.get(i);
                childNodes.addAll(node.getChildNodes());
                String val = toHexBinaryString(node.getData());
                Integer count = map.get(val);
                if (count != null) {
                    map.put(val, count + node.getCount());
                    continue;
                }

                map.put(val, node.getCount());
            }
            
            Entry<String, Integer> max = Collections.max(map.entrySet(), (Entry<String, Integer> o1, Entry<String, Integer> o2) -> o1.getValue().compareTo(o2.getValue()));
            String hexColor = String.format("#%06X", 0xFFFFFF & java.awt.Color.HSBtoRGB((float)max.getValue() / packetsCount / 3f, 1f, 1f));
            labels.append(String.format("\t%d [label=\"%s\" style=filled fillcolor=\"%s\"];%s", flowIndex, max.getKey(), hexColor, System.lineSeparator()));
            graph.append(String.format("%d -> ", flowIndex++));
            
            nodes = childNodes;
            childNodes = new ArrayList<>();
        }
        
        writer.println(labels.toString());
        writer.println(String.format("\t%s;", graph.substring(0, graph.length() - 4)));
    }
    
    private void printGraphSettings(PrintWriter writer) {
        ABDUGraphSettings graphSettings = settings.getGraphSettings();
        
        if (graphSettings.getRankDir() != null) {
            writer.println(String.format("\trankdir=%s;", graphSettings.getRankDir()));
        }
        
        if (graphSettings.getSize() != null) {
            writer.println(String.format("\tsize=\"%s\";", graphSettings.getSize()));
        }
        
        if (graphSettings.getNodeAttributes() != null) {
            writer.println(String.format("\tnode [%s];", graphSettings.getNodeAttributes()));
        }
    }
    
    private String toHexBinaryString(byte[] bytes) {
        String separator = settings.getBytesSeparator();
        if (separator == null) {
            separator = "";
        }
        
        StringBuilder str = new StringBuilder(bytes.length * (separator.length() + 2));
        for(byte b : bytes) {
            str.append(String.format("%02X%s", b, separator));
        }
        
        // remove last separator
        if (str.length() > 0) {
            str.setLength(str.length() - separator.length());
        }
        
        return str.toString();
    }
}
