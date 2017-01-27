/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.settings;

import parser.settings.graph.ABDUGraphSettings;

/**
 *
 * @author Andrej
 */
public class ABDUSettings {
    private String outputDirectory;
    private String bytesSeparator;
    private boolean separatePackets;
    private boolean simpleNodes;
    private boolean checkMinimalLengthOnShorterStreams;
    private int outputTypeMask;
    private int headerLength;
    private int minimalConstantLength;
    private final ABDUGraphSettings graphSettings = new ABDUGraphSettings();
    
    public void setOutputDirectory(String outputDir) {
        outputDirectory = outputDir;
    }
    
    public String getOutputDirectory() {
        return outputDirectory;
    }
    
    public void setSeparatePackets(boolean val) {
        separatePackets = val;
    }
    
    public boolean separatePackets() {
        return separatePackets;
    }
    
    public void setSimpleNodes(boolean val) {
        simpleNodes = val;
    }
    
    public boolean simpleNodes() {
        return simpleNodes;
    }
    
    public void setOutputTypeMask(int mask) {
        outputTypeMask = mask;
    }
    
    public int getOutputTypeMask() {
        return outputTypeMask;
    }
    
    public void setHeaderLength(int length) {
        headerLength = length;
    }
    
    public int getHeaderLength() {
        return headerLength;
    }
    
    public void setBytesSeparator(String val) {
        bytesSeparator = val;
    }
    
    public String getBytesSeparator() {
        return bytesSeparator;
    }
    
    public ABDUGraphSettings getGraphSettings() {
        return graphSettings;
    }
    
    public int getMinimalConstantLength() {
        return minimalConstantLength;
    }
    
    public void setMinimalConstantLength(int length) {
        minimalConstantLength = length;
    }
    
    public boolean getCheckMinimalLengthOnShorterStreams() {
        return checkMinimalLengthOnShorterStreams;
    }
    
    public void setCheckMinimalLengthOnShorterStreams(boolean check) {
        checkMinimalLengthOnShorterStreams = check;
    }
}
