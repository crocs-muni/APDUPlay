/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser;

import parser.data.Tree;
import parser.output.Writer;
import parser.settings.Settings;
import java.io.File;
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
public class Parser {
    private int ac;
    private final Logger logger;
    private final Settings settings;
    private final Map<String, Tree> packets;
    private String lastProcessedPacket;
    
    /**
     * Creates new instance of parser
     * 
     * @param settings
     * @param logger
     */
    public Parser(Settings settings, Logger logger) {
        this.settings = settings;
        this.logger = logger;
        this.packets = new HashMap<>();
        ac = 1;
    }
    
    /**
     * Parses specified file
     * 
     * @param path absolute or relative path to file
     */
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
    
    /**
     * Writes stored data
     */
    public void printData() {
        Writer abduWriter = new Writer(settings, logger);
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
                break;
            case "responseTime":
                handleResponseTime(pair.getValue().toString());
                break;
            default:
                break;
        }
    }
    
    private String normalizeHexString(String str) {
        return str.trim().replaceAll(" ", "");
    }
    
    private void handleTransmittedData(byte[] data) {
        byte[] packetHeader = Arrays.copyOfRange(data, 0, settings.getHeaderLength());
        byte[] packetData = Arrays.copyOfRange(data, settings.getHeaderLength(), data.length);
        String header = DatatypeConverter.printHexBinary(packetHeader);
        
        Tree existingTree = packets.get(header);
        if (existingTree == null) {
            Tree tree = new Tree(packetHeader, packetData);
            packets.put(tree.header, tree);
        } else {
            existingTree.merge(packetData);
        }
        
        lastProcessedPacket = header;
    }
    
    private void handleReceivedData(byte[] data) {
        Tree tree = packets.get(lastProcessedPacket);
        if (tree == null) {
            logger.error("Packet \"" + DatatypeConverter.printHexBinary(data) + "\" not found as parsed tree");
            return;
        }
        
        tree.addReceivedData(data);
    }
    
    private void handleResponseTime(String time) {
        time = time.replaceAll("[^0-9]","");
        Tree tree = packets.get(lastProcessedPacket);
        
        // Sets also ac
        tree.setAdditionalInfoForLastPacket(Integer.parseInt(time), ac++);
    }
}
