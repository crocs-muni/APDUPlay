/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.output.data.analyzedPackets;

import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Andrej
 */
public class ABDUOutputTree {
    public final String header;
    public final int identifier;
    private List<ABDUOutputPacket> packets;
    
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
    
    public void addPacket(ABDUOutputPacket packet) {
        int index = packets.indexOf(packet);
        if (index == -1) {
            packets.add(packet);
            return;
        }
        
        ABDUOutputPacket p = packets.get(index);
        p.addReceivedMessages(packet.getReceivedMessages());
    }

    /**
     * Prepares output of the current tree
     * 
     * @return prepared output of the current tree
     */
    public String prepareOutput() {
        StringBuilder sb = new StringBuilder();
        sb.append(String.format("\t%d [label=\"%s\"];", identifier, header));
        
        /// TODO
        
        return sb.toString();
    }
}
