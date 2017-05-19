/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.output.data;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import lombok.Getter;
import lombok.Setter;
import lombok.val;
import parser.data.Node;
import parser.settings.Settings;
import tools.Pair;
import tools.SimilarityTool;
import tools.StringUtil;

/**
 *
 * @author Andrej
 */
public class OutputTree {
    public final String header;
    public final int identifier;
    private final Settings settings;
    
    /**
    * Represents transmitted packets with all their respective responsese
    * 
    * @param packets transmitted packets with all their respective responsese
    * @return transmitted packets with all their respective responsese
    */
    @Getter @Setter private List<OutputPacket> packets;
    
    /**
     * Creates new instance of OutputTree
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
        
        val outputPackets = packets.get(index);
        outputPackets.getTransmittedMessage().increaseCount(packet.getTransmittedMessage().getCount());
        outputPackets.addReceivedMessages(packet.getReceivedMessages());
    }

    /**
     * Prepares output of the current tree
     * 
     * @return prepared output of the current tree
     */
    public String prepareOutput() {
        val sb = new StringBuilder();
        sb.append(String.format("\t%d [label=\"%s\"];%s", identifier, header, System.lineSeparator()));
        
        List<OutputMessage> transmittedMessages = new ArrayList<>();
        packets.forEach((packet) -> {
            transmittedMessages.add(packet.getTransmittedMessage());
            prepare(sb, packet.getTransmittedMessage().identifier, packet.getReceivedMessages(), true);
        });
        prepare(sb, identifier, transmittedMessages, false);
        
        return sb.toString();
    }
    
    /**
     * Prepares output of the current tree in text form
     * 
     * @return prepared output of the current tree in text form
     */
    public String prepareTextOutput() {
        val sb = new StringBuilder();
        sb.append(header);
        sb.append(":");
        
        List<OutputMessage> transmittedMessages = new ArrayList<>();
        List<OutputMessage> receivedMessages = new ArrayList<>();
        
        packets.forEach((packet) -> {
            transmittedMessages.add(packet.getTransmittedMessage());
            receivedMessages.addAll(packet.getReceivedMessages());
        });
        
        val prefixes = longestCommonPrefixes(transmittedMessages);
        val receivedPrefixes = longestCommonPrefixes(receivedMessages);
        
        val firstTransmitted = transmittedMessages.get(0).message;
        val firstReceived = receivedMessages.get(0).message;
        sb.append("{");
        sb.append(settings.getTextOutputSettings().getBeforeDataStream());
        val transmittedMid = analyzeMessages(transmittedMessages, prefixes.left, prefixes.right);
        if (transmittedMid.length() == 0) {
            sb.append(splitAndJoinString(firstTransmitted));
        } else {
            sb.append(splitAndJoinString(String.format("%s %s %s",
                    firstTransmitted.substring(0, prefixes.left).trim(), 
                    transmittedMid.trim(),
                    firstTransmitted.substring(firstTransmitted.length() - prefixes.right).trim())));
        }
        
        sb.append(settings.getTextOutputSettings().getAfterDataStream());
        sb.append("}{");
        sb.append(settings.getTextOutputSettings().getBeforeDataStream());
        
        val receivedMid = analyzeMessages(receivedMessages, receivedPrefixes.left, receivedPrefixes.right);
        if (receivedMid.length() == 0) {
            sb.append(splitAndJoinString(firstReceived));
        } else {
            sb.append(splitAndJoinString(String.format("%s %s %s",
                    firstReceived.substring(0, receivedPrefixes.left).trim(), 
                    receivedMid.trim(),
                    firstReceived.substring(firstReceived.length() - receivedPrefixes.right).trim())));
        }
        sb.append(settings.getTextOutputSettings().getAfterDataStream());
        sb.append("}");
        return sb.toString();
    }
    
    private String splitAndJoinString(String str) {
        // split on any whitespace so we do not rely on just one space
        String[] values = str.trim().split("\\s+");
        if (settings.getTextOutputSettings().isDataByteIndexIncluded()) {
            for (int i = 0; i < values.length; i++) {
                values[i] = String.format("%04d: %s", i, values[i]);
            }
        }
        return String.join(settings.getTextOutputSettings().getBytesSeparator(), values);
    }
    
    private String analyzeMessages(List<OutputMessage> msgs, int leftIndex, int rightIndex) {
        StringBuilder returnStr = new StringBuilder("");
        if (leftIndex > 0) {
            leftIndex++;
        }
        
        if (rightIndex > 0) {
            rightIndex++;
        }
        
        while(true) {
            Set<String> bytes = new HashSet<>();
            int msgsCount = 0;
            boolean addEmptyByte = false;
            for(val msg : msgs) {
                if(msg.message.length() > leftIndex + rightIndex) {
                    val curByte = msg.message.substring(leftIndex, leftIndex + 2);
                    bytes.add(curByte);
                } else if (!addEmptyByte) {
                    addEmptyByte = true;
                }
                
                msgsCount += msg.getCount();
            }
            
            leftIndex += 3;
            if (bytes.isEmpty()) {
                break;
            }
            
            if (returnStr.length() > 0) {
                returnStr.append(" ");
            }
            
            if (addEmptyByte && settings.getTextOutputSettings().isEmptyByteIncluded()) {
                bytes.add(settings.getTextOutputSettings().getEmptyByteValue());
            }
            
            if (bytes.size() == 1) {
                returnStr.append(bytes.iterator().next());
                continue;
            }
            
            if (bytes.size() > 20 || bytes.size() / msgsCount > 0.33) {
                returnStr.append(settings.getTextOutputSettings().getRandomByteValue());
                continue;
            }
            
            returnStr.append(String.join(settings.getTextOutputSettings().getByteEnumerationSeparator(), bytes));
        }
        
        return returnStr.toString();
    }
    
    private Pair<Integer, Integer> longestCommonPrefixes(List<OutputMessage> msgs) {
        val strings = new String[msgs.size()];
        val invertedStrings = new String[msgs.size()];
        for (int i = 0; i < msgs.size(); i++) {
            strings[i] = msgs.get(i).message;
            invertedStrings[i] = new StringBuilder(strings[i]).reverse().toString();
        }
        
        int left = longestCommonPrefix(strings);
        int right = longestCommonPrefix(invertedStrings);
        
        return new Pair(left, right);
    }
    
    private void prepare(StringBuilder sb, int parentIdentifier, List<OutputMessage> msgs, boolean generateIdentifier) {
        val prefixes = longestCommonPrefixes(msgs);
        val color = getColorForMidStream(msgs, prefixes.left, prefixes.right);
        
        msgs.forEach((msg) -> {
            int msgLength = msg.message.length();
            int nodeIdentifier = generateIdentifier ? new Node(null).identifier : msg.identifier;
            
            if (prefixes.left + prefixes.right + 1 >= msgLength) {
                sb.append(String.format("\t%d [label=<%s>];", nodeIdentifier, StringUtil.wrapText(msg.message, settings.getGraphSettings().getWrapAfter() * 3, -1).left));
            } else {
                sb.append(String.format("\t%d [label=<", nodeIdentifier));
                Pair<String, Integer> wrappedText = StringUtil.wrapText(msg.message.substring(0, prefixes.left), settings.getGraphSettings().getWrapAfter() * 3, -1);
                sb.append(wrappedText.left);
                
                wrappedText = StringUtil.wrapText(msg.message.substring(prefixes.left, msgLength - prefixes.right), settings.getGraphSettings().getWrapAfter() * 3, wrappedText.right);
                sb.append(String.format("<font color=\"%s\">%s</font>", color, wrappedText.left));
                sb.append(StringUtil.wrapText(msg.message.substring(msgLength - prefixes.right), settings.getGraphSettings().getWrapAfter() * 3, wrappedText.right).left);
                
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
        if (!returnIndex && !settings.isCheckMinimalLengthOnShorterStreams()) {
            returnIndex = end >= minStr.length();
        }
        
        if (!returnIndex) {
            return 0;
        }
        
        if (end >= minStr.length()) {
            return minStr.length();
        }
        
        return end > 1 ? end - 1 : 0; // ignore last space
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
        
        return similarityRank / count > .5 ? settings.getGraphSettings().getSimilarByteStreamColor() : settings.getGraphSettings().getRandomByteStreamColor();
    }
}
