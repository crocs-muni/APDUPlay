/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.output;

import parser.output.data.OutputPacket;
import parser.output.data.OutputTree;
import parser.settings.Settings;
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
import lombok.val;
import parser.data.Node;
import parser.data.Packet;
import parser.data.Tree;
import parser.logging.ILogger;
import parser.output.data.OutputMessage;
import tools.StringUtil;

/**
 *
 * @author Andrej
 */
public class Writer {
    
    private static int flowIndex = 0;
    private final Settings settings;
    private final ILogger logger;
    
    /**
     * Creates new instance of Writer
     * 
     * @param settings
     * @param logger
     */
    public Writer(Settings settings, ILogger logger) {
        this.settings = settings;
        this.logger = logger;
    }
    
    /**
     * Writes packets into a file
     * 
     * @param packets packets to write
     */
    public void write(Collection<Tree> packets) {
        val file = new File(settings.getOutputDirectory());
        file.mkdirs();

        if (settings.isSimpleNodes()) {
            packets.forEach((tree) -> tree.simplifyNodes());
        }
        
        if (!settings.isSeparatePackets()) {
            writeInOneFile(packets, file.getAbsolutePath());
        } else {
            writeSeparated(packets, file.getAbsolutePath());
        }
    }
    
    private List<OutputFunction> getOutputFunctions() {
        List<OutputFunction> functions = new ArrayList<>();
        for(int type : OutputType.TYPES) {
            if ((settings.getOutputTypeMask() & type) != 0) {
                switch(type) {
                    case OutputType.NODES:
                        functions.add(new OutputFunction((tree, writer) -> printTransmitted(tree, writer), (tree, writer) -> printReceived(tree, writer)));
                        break;
                    case OutputType.FLOW:
                        functions.add(new OutputFunction((tree, writer) -> printTransmittedFlow(tree, writer), (tree, writer) -> printReceivedFlow(tree, writer)));
                        break;
                    case OutputType.PACKETS:
                        functions.add(new OutputFunction((tree, writer) -> printPackets(tree, writer), null));
                        break;
                    case OutputType.PACKETS_ANALYZED:
                        functions.add(new OutputFunction((tree, writer) -> printAnalyzedPackets(tree, writer), null));
                        break;
                    case OutputType.PACKETS_ANALYZED_TEXT:
                        functions.add(new OutputFunction((tree, writer) -> printAnalyzedPacketsAsText(tree, writer), null, false));
                        break;
                }
            }
        }
        
        return functions;
    }
    
    private void writeInOneFile(Collection<Tree> packets, String directoryPath) {
        val outputFunctions = getOutputFunctions();
        
        for (int i = 0; i < outputFunctions.size(); i++) {
            if (outputFunctions.get(i).hasTransmittedFunction()) {
                try (PrintWriter writer = new PrintWriter(String.format("%s/packets_transmitted(%d).dot", directoryPath, i), "UTF-8")) {
                    if (outputFunctions.get(i).wrapGraphvizOutput) {
                        writer.println("digraph packets {");
                        printGraphSettings(writer);
                    }
                    
                    for (Tree tree : packets) {
                        outputFunctions.get(i).invokeTransmitted(tree, writer);
                    }
                    
                    if (outputFunctions.get(i).wrapGraphvizOutput) {
                        writer.println("}");
                    }
                }
                catch (Exception ex) {
                    logger.error(ex.getMessage());
                }
            }
            
            if (outputFunctions.get(i).hasReceivedFunction()) {
                try (PrintWriter writer = new PrintWriter(String.format("%s/packets_received(%d).dot",directoryPath, i), "UTF-8")) {
                    if (outputFunctions.get(i).wrapGraphvizOutput) {
                        writer.println("digraph packets {");
                        printGraphSettings(writer);
                    }
                    
                    for (Tree tree : packets) {
                        outputFunctions.get(i).invokeReceived(tree, writer);
                    }
                    
                    if (outputFunctions.get(i).wrapGraphvizOutput) {
                        writer.println("}");
                    }
                }
                catch (Exception ex) {
                    logger.error(ex.getMessage());
                }
            }
        }
    }
    
    private void writeSeparated(Collection<Tree> packets, String directoryPath) {
        val outputFunctions = getOutputFunctions();
        for (int i = 0; i < outputFunctions.size(); i++) {
            for (Tree tree : packets) {
               
                if (outputFunctions.get(i).hasTransmittedFunction()) {
                    try (PrintWriter writer = new PrintWriter(String.format("%s/%s_transmitted(%d).dot", directoryPath, tree.header, i), "UTF-8")) {
                        if (outputFunctions.get(i).wrapGraphvizOutput) {
                            writer.println("digraph transmitted {");
                            printGraphSettings(writer);
                        }
                        
                        outputFunctions.get(i).invokeTransmitted(tree, writer);
                        
                        if (outputFunctions.get(i).wrapGraphvizOutput) {
                            writer.println("}");
                        }
                    }
                    catch (Exception ex) {
                        logger.error(ex.getMessage());
                    }
                }

                if (outputFunctions.get(i).hasReceivedFunction()) {
                    try (PrintWriter writer = new PrintWriter(String.format("%s/%s_received(%d).dot", directoryPath, tree.header, i), "UTF-8")) {
                        if (outputFunctions.get(i).wrapGraphvizOutput) {
                            writer.println("digraph received {");
                            printGraphSettings(writer);
                        }
                        
                        outputFunctions.get(i).invokeReceived(tree, writer);
                        
                        if (outputFunctions.get(i).wrapGraphvizOutput) {
                            writer.println("}");
                        }
                    }
                    catch (Exception ex) {
                        logger.error(ex.getMessage());
                    }
                }
            }
        }
    }
    
    private void printPackets(Tree tree, PrintWriter writer) {
        writer.println(String.format("\t%d [label=\"%s\"];", tree.root.identifier, toHexBinaryString(tree.root.getData())));
        
        Map<String, Map<String, List<Packet>>> streams = new HashMap<>();
        tree.streamPackets.forEach((packet) -> {
            val transmitted = getDataFromLeafNode(packet.getTransmittedLeafNode());
            val received = getDataFromLeafNode(packet.getReceivedLeafNode());
            
            String transmittedStr = toHexBinaryString(Arrays.copyOfRange(transmitted, settings.getHeaderLength(), transmitted.length));
            String receivedStr = toHexBinaryString(Arrays.copyOfRange(received, settings.getHeaderLength(), received.length));
            
            Map<String, List<Packet>> value = streams.get(transmittedStr);
            if (value != null) {
                List<Packet> p = value.get(receivedStr);
                if (p == null) {
                    p = new LinkedList<>();
                    value.put(receivedStr, p);
                }
                p.add(packet);
            } else {
                value = new HashMap<>();
                List<Packet> p = new LinkedList<>();
                p.add(packet);
                value.put(receivedStr, p);
                streams.put(transmittedStr, value);
            }
        });
        
        streams.entrySet().forEach((item) -> {
            // To generate identifier
            val transmittedNode = new Node(null);
            writer.println(String.format("\t%d [label=<%s>];", transmittedNode.identifier, StringUtil.wrapText(item.getKey(), settings.getGraphSettings().getWrapAfter() * 3, -1).left));
            
            item.getValue().forEach((received, packets) -> {
                val receivedNode = new Node(null);
                writer.println(String.format("\t%d [label=<%s>];", receivedNode.identifier, StringUtil.wrapText(received, settings.getGraphSettings().getWrapAfter() * 3, -1).left));
                
                packets.forEach((packet) -> {
                    writer.println(String.format("\t%d -> %d [label=\"[ac=%d]\"];", tree.root.identifier, transmittedNode.identifier, packet.getAc()));
                    writer.println(String.format("\t%d -> %d [label=\"[ac=%d, time=%d]\"];", transmittedNode.identifier, receivedNode.identifier, packet.getAc(), packet.getResponseTime()));
                });
            });
        });
    }
    
    private OutputTree prepareAnalyzedPackets(Tree tree) {
        val outputTree = new OutputTree(toHexBinaryString(tree.root.getData()), tree.root.identifier, settings);
        tree.streamPackets.forEach((packet) -> {
            val transmitted = getDataFromLeafNode(packet.getTransmittedLeafNode());
            val received = getDataFromLeafNode(packet.getReceivedLeafNode());
            
            val transmittedStr = toHexBinaryString(Arrays.copyOfRange(transmitted, settings.getHeaderLength(), transmitted.length));
            val receivedStr = toHexBinaryString(Arrays.copyOfRange(received, settings.getHeaderLength(), received.length));
            
            val p = new OutputPacket(new OutputMessage(transmittedStr, packet.getTransmittedLeafNode().identifier));
            p.addReceivedMessage(new OutputMessage(receivedStr, packet.getReceivedLeafNode().identifier));
            
            outputTree.addPacket(p);
        });
        
        return outputTree;
    }
    
    private void printAnalyzedPackets(Tree tree, PrintWriter writer) {
        writer.println(prepareAnalyzedPackets(tree).prepareOutput());
    }
    
    private void printAnalyzedPacketsAsText(Tree tree, PrintWriter writer) {
        writer.println(prepareAnalyzedPackets(tree).prepareTextOutput());
    }
    
    private byte[] getDataFromLeafNode(Node node) {
        List<byte[]> data = new ArrayList<>();
        
        int size = 0;
        while (node != null) {
            data.add(node.getData());    
            size += node.getData().length;
            node = node.getParent();
        }
        
        val array = new byte[size];
        size = 0;
        for (int i = data.size() - 1; i >= 0; i--) {
            for (byte b : data.get(i)) {
                array[size++] = b;
            }
        }
        
        return array;
    }
    
    private void printTransmitted(Tree tree, PrintWriter writer) {
        printLabels(tree.root, writer);
        print(tree.root, writer);
    }
    
    private void printReceived(Tree tree, PrintWriter writer) {
        printLabels(tree.receivedRoot, writer);
        print(tree.receivedRoot, writer);
    }
    
    private void printTransmittedFlow(Tree tree, PrintWriter writer) {
        printFlow(tree.root, tree.getPacketsCount(), writer);
    }
    
    private void printReceivedFlow(Tree tree, PrintWriter writer) {
        printFlow(tree.receivedRoot, tree.getPacketsCount(), writer);
    }
    
    private void print(Node node, PrintWriter writer) {
        Stack<Node> stack = new Stack<>();
        stack.add(node);
        
        List<Node> toPrint = new ArrayList<>();
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
                toPrint.add(stack.peek().getParent());
            }
        }
    }
    
    private void printLabels(Node node, PrintWriter writer) {
        Queue<Node> queue = new ArrayDeque<>();
        queue.add(node);
        while(!queue.isEmpty()) {
            node = queue.remove();
            writer.println(String.format("\t%d [label=<%s>];", node.identifier, StringUtil.wrapText(toHexBinaryString(node.getData()), settings.getGraphSettings().getWrapAfter() * 3, -1).left));
            queue.addAll(node.getChildNodes());
        }
    }
    
    private void printFlow(Node node, int packetsCount, PrintWriter writer) {
        List<Node> nodes = new ArrayList<>();
        List<Node> childNodes = new ArrayList<>();
        val labels = new StringBuilder();
        val graph = new StringBuilder();
        nodes.add(node);
        while(!nodes.isEmpty()) {
            Map<String, Integer> map = new HashMap<>();
            for (int i = 0; i < nodes.size(); i++) {
                node = nodes.get(i);
                childNodes.addAll(node.getChildNodes());
                String value = toHexBinaryString(node.getData());
                Integer count = map.get(value);
                if (count != null) {
                    map.put(value, count + node.getCount());
                    continue;
                }

                map.put(value, node.getCount());
            }
            
            val max = Collections.max(map.entrySet(), (Entry<String, Integer> o1, Entry<String, Integer> o2) -> o1.getValue().compareTo(o2.getValue()));
            val hexColor = String.format("#%06X", 0xFFFFFF & java.awt.Color.HSBtoRGB((float)max.getValue() / packetsCount / 3f, 1f, 1f));
            labels.append(String.format("\t%d [label=<%s> style=filled fillcolor=\"%s\"];%s", flowIndex, StringUtil.wrapText(max.getKey(), settings.getGraphSettings().getWrapAfter() * 3, -1).left, hexColor, System.lineSeparator()));
            graph.append(String.format("%d -> ", flowIndex++));
            
            nodes = childNodes;
            childNodes = new ArrayList<>();
        }
        
        writer.println(labels.toString());
        writer.println(String.format("\t%s;", graph.substring(0, graph.length() - 4)));
    }
    
    private void printGraphSettings(PrintWriter writer) {
        val graphSettings = settings.getGraphSettings();
        
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
        
        val str = new StringBuilder(bytes.length * (separator.length() + 2));
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
