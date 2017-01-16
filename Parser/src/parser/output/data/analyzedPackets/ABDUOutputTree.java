/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.output.data.analyzedPackets;

import java.util.ArrayList;
import java.util.List;
import parser.data.ABDUNode;

/**
 *
 * @author Andrej
 */
public class ABDUOutputTree {
    public final String header;
    public final int identifier;
    private List<ABDUOutputPacket> packets;
    
    /**
     * Creates new instance of ABDUOutputTree
     * 
     * @param header packet header
     * @param identifier node identifier for output
     */
    public ABDUOutputTree(String header, int identifier) {
        this.header = header;
        this.identifier = identifier;
        packets = new ArrayList<>();
    }
    
    public List<ABDUOutputPacket> getPackets() {
        return packets;
    }
    
    public void setPackets(List<ABDUOutputPacket> packets) {
        this.packets = packets;
    }
    
    /**
     * Adds output packet into this tree
     * 
     * @param packet packet to be added
     */
    public void addPacket(ABDUOutputPacket packet) {
        int index = packets.indexOf(packet);
        if (index == -1) {
            packets.add(packet);
            return;
        }
        
        ABDUOutputPacket p = packets.get(index);
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
        
        List<ABDUOutputMessage> transmittedMessages = new ArrayList<>();
        packets.forEach((packet) -> {
            transmittedMessages.add(packet.getTransmittedMessage());
            prepare(sb, packet.getTransmittedMessage().identifier, packet.getReceivedMessages(), true);
        });
        prepare(sb, identifier, transmittedMessages, false);
        
        return sb.toString();
    }
    
    private void prepare(StringBuilder sb, int parentIdentifier, List<ABDUOutputMessage> msgs, boolean generateIdentifier) {
        String[] strings = new String[msgs.size()];
        String[] invertedStrings = new String[msgs.size()];
        for (int i = 0; i < msgs.size(); i++) {
            strings[i] = msgs.get(i).message;
            invertedStrings[i] = new StringBuilder(strings[i]).reverse().toString();
        }
        
        int left = longestCommonPrefix(strings);
        int right = longestCommonPrefix(invertedStrings);
        
        msgs.forEach((msg) -> {
            int msgLength = msg.message.length();
            int nodeIdentifier = generateIdentifier ? new ABDUNode(null).identifier : msg.identifier;
            
            if (left + right + 1 >= msgLength) {
                sb.append(String.format("\t%d [label=<<font color=\"green\">%s</font>>];", nodeIdentifier, msg.message));
            } else {
                sb.append(String.format("\t%d [label=<", nodeIdentifier));
                if (left > 0) {
                    sb.append(String.format("<font color=\"green\">%s</font>", msg.message.substring(0, left)));
                }
                
                sb.append(String.format("<font color=\"red\">%s</font>", msg.message.substring(left, msgLength - right)));
                if (right > 0) {
                    sb.append(String.format("<font color=\"green\">%s</font>", msg.message.substring(msgLength - right)));
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
        
        return end > 0 ? end - 1 : 0; // ignore last space
    }
}
