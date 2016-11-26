/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser;

import parser.settings.ABDUSettings;
import java.io.File;
import java.io.PrintWriter;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;
import javafx.util.Pair;
import javax.xml.bind.DatatypeConverter;

/**
 *
 * @author Andrej
 */
public class ABDUParser {
    private final ABDULogger logger;
    private final ABDUSettings settings;
    private final Map<Short, ABDUTree> packets;
    private short lastProcessedPacket;
    
    /**
     * Creates new instance of parser
     * @param settings
     * @param logger
     */
    public ABDUParser(ABDUSettings settings, ABDULogger logger) {
        this.settings = settings;
        this.logger = logger;
        this.packets = new HashMap<>();
        lastProcessedPacket = -1;
    }
    
    public void parseFile(String path) {
        try (Scanner scanner =  new Scanner(new File(path))){
            while(scanner.hasNextLine()) {
                Pair pair = processLine(scanner.nextLine());
                handleABDULine(pair);
            }
        } catch(Exception ex) {
            logger.error(ex.getMessage());
        }
    }
    
    public void printData() {
        ABDUWriter abduWriter = new ABDUWriter(settings, logger);
        abduWriter.write(packets.values());
    }
    
    private Pair processLine(String line) {
        String[] parts = line.split(":");
        if (parts.length != 2) {
            return null;
        }
        
        return new Pair(parts[0], parts[1]);
    }
    
    private void handleABDULine(Pair pair) {
        if (pair == null) {
            return;
        }
        
        switch(pair.getKey().toString()) {
            case "transmitted":
                handleTransmittedData(DatatypeConverter.parseHexBinary(normalizeHexString(pair.getValue().toString())));
                break;
            case "received":
                handleReceivedData(DatatypeConverter.parseHexBinary(normalizeHexString(pair.getValue().toString())));
            default:
                break;
        }
    }
    
    private String normalizeHexString(String str) {
        return str.trim().replaceAll(" ", "");
    }
    
    private void handleTransmittedData(byte[] data) {
        byte[] packetHeader = Arrays.copyOfRange(data, 0, 2);
        byte[] packetData = Arrays.copyOfRange(data, 2, data.length);
        ByteBuffer wrapped = ByteBuffer.wrap(packetHeader);
        short header = wrapped.getShort();
        
        ABDUTree existingTree = packets.get(header);
        if (existingTree == null) {
            ABDUTree tree = new ABDUTree(packetHeader, packetData);
            packets.put(tree.header, tree);
        } else {
            existingTree.merge(packetData);
        }
        
        lastProcessedPacket = header;
    }
    
    private void handleReceivedData(byte[] data) {
        ABDUTree tree = packets.get(lastProcessedPacket);
        if (tree == null) {
            logger.error("Packet \"" + DatatypeConverter.printHexBinary(data) + "\" not found as parsed tree");
            return;
        }
        
        tree.addReceivedData(data);
    }
}
