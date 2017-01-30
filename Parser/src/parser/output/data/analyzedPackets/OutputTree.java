/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.output.data.analyzedPackets;

import java.util.ArrayList;
import java.util.List;
import parser.data.Node;
import parser.settings.Settings;
import tools.SimilarityTool;

/**
 *
 * @author Andrej
 */
public class OutputTree {
    public final String header;
    public final int identifier;
    private final Settings settings;
    private List<OutputPacket> packets;
    
    /**
     * Creates new instance of ABDUOutputTree
     * 
     * @param header packet header
     * @param identifier node identifier for output
     * @param settings output settings
     */
    public OutputTree(String header, int identifier, Settings settings) {
        this.header = header;
        this.identifier = identifier;
        this.settings = settings;
        packets = new ArrayList<>();
    }
    
    public List<OutputPacket> getPackets() {
        return packets;
    }
    
    public void setPackets(List<OutputPacket> packets) {
        this.packets = packets;
    }
    
    /**
     * Adds output packet into this tree
     * 
     * @param packet packet to be added
     */
    public void addPacket(OutputPacket packet) {
        int index = packets.indexOf(packet);
        if (index == -1) {
            packets.add(packet);
            return;
        }
        
        OutputPacket p = packets.get(index);
        p.getTransmittedMessage().increaseCount(packet.getTransmittedMessage().getCount());
        p.addReceivedMessages(packet.getReceivedMessages());
    }

    /**
     * Prepares output of the current tree
     * 
     * @return prepared output of the current tree
     */
    public String prepareOutput() {
        StringBuilder sb = new StringBuilder();
        sb.append(String.format("\t%d [label=\"%s\"];%s", identifier, header, System.lineSeparator()));
        
        List<OutputMessage> transmittedMessages = new ArrayList<>();
        packets.forEach((packet) -> {
            transmittedMessages.add(packet.getTransmittedMessage());
            prepare(sb, packet.getTransmittedMessage().identifier, packet.getReceivedMessages(), true);
        });
        prepare(sb, identifier, transmittedMessages, false);
        
        return sb.toString();
    }
    
    private void prepare(StringBuilder sb, int parentIdentifier, List<OutputMessage> msgs, boolean generateIdentifier) {
        String[] strings = new String[msgs.size()];
        String[] invertedStrings = new String[msgs.size()];
        for (int i = 0; i < msgs.size(); i++) {
            strings[i] = msgs.get(i).message;
            invertedStrings[i] = new StringBuilder(strings[i]).reverse().toString();
        }
        
        int left = longestCommonPrefix(strings);
        int right = longestCommonPrefix(invertedStrings);
        
        String color = getColorForMidStream(msgs, left, right);
        msgs.forEach((msg) -> {
            int msgLength = msg.message.length();
            int nodeIdentifier = generateIdentifier ? new Node(null).identifier : msg.identifier;
            
            if (left + right + 1 >= msgLength) {
                sb.append(String.format("\t%d [label=\"%s\"];", nodeIdentifier, msg.message));
            } else {
                sb.append(String.format("\t%d [label=<", nodeIdentifier));
                if (left > 0) {
                    sb.append(msg.message.substring(0, left));
                }
                
                sb.append(String.format("<font color=\"%s\">%s</font>", color, msg.message.substring(left, msgLength - right)));
                if (right > 0) {
                    sb.append(msg.message.substring(msgLength - right));
                }
                
                sb.append(">];");
            }
            
            sb.append(String.format("%s\t%d -> %d;%s", System.lineSeparator(), parentIdentifier, nodeIdentifier, System.lineSeparator()));
        });
    }
    
    private int longestCommonPrefix(String[] strs) {
        if(strs.length == 0) {
            return 0;
        }
        
        String minStr = strs[0];
        
        // Get shortest string
        for(int i = 1; i < strs.length; i++){
            if(strs[i].length() < minStr.length())
                minStr = strs[i];
        }
        
        int end = minStr.length();
        for (String str : strs) {
            int j;
            for (j = 0; end != 0 && j < end + 1; j += 3) {
                if (minStr.charAt(j) != str.charAt(j) || minStr.charAt(j + 1) != str.charAt(j + 1)) {
                    break;
                }
            }
            
            if(j < end) {
                end = j;
            }
        }
        
        boolean returnIndex = end >= settings.getMinimalConstantLength() * 3;
        if (!returnIndex && !settings.getCheckMinimalLengthOnShorterStreams()) {
            returnIndex = end >= minStr.length();
        }
        
        return returnIndex ? end - 1 : 0; // ignore last space
    }
    
    private String getColorForMidStream(List<OutputMessage> msgs, int leftIndex, int rightIndex) {
        double similarityRank = 0;
        int count = 0;
        int msgsLength = msgs.size();
        
        for (int i = 0; i < msgsLength; i++) {
            for (int j = 1; j < msgsLength; j++) {
                if (i == j) {
                    continue;
                }
                
                String msg1 = msgs.get(i).message;
                String msg2 = msgs.get(j).message;
                double currentRank = SimilarityTool.compareStrings(msg1.substring(leftIndex, msg1.length() - rightIndex), msg2.substring(leftIndex, msg2.length() - rightIndex));
                int msgCount = msgs.get(i).getCount() * msgs.get(j).getCount();

                similarityRank += currentRank * msgCount;
                count += msgCount;
            }
            
            int msgCount = msgs.get(i).getCount();
            if (msgCount > 1) {
                msgCount = msgCount * (msgCount - 1) / 2;
                similarityRank += msgCount;
                count += msgCount;
            }
        }
        
        return similarityRank / count > .5 ? "darkorchid4" : "red";
    }
}
