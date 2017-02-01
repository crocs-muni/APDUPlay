/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.settings;

import parser.output.OutputType;
import parser.settings.graph.GraphSettings;

/**
 *
 * @author Andrej
 */
public class Settings {
    private String outputDirectory;
    private String bytesSeparator;
    private boolean separatePackets;
    private boolean simpleNodes;
    private boolean checkMinimalLengthOnShorterStreams;
    private int outputTypeMask;
    private int headerLength;
    private int minimalConstantLength;
    private final GraphSettings graphSettings = new GraphSettings();

    public void setDefaults() {
        outputDirectory = "output";
        separatePackets = false;
        simpleNodes = false;
        outputTypeMask = OutputType.ALL;
        headerLength = 4;
        minimalConstantLength = 0;
        bytesSeparator = " ";
        
        graphSettings.setRankDir("LR");
        graphSettings.setNodeAttributes("color=lightblue2, style=filled");
        graphSettings.setRandomByteStreamColor("red");
        graphSettings.setSimilarByteStreamColor("darkorchid4");
    }
    
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
    
    public GraphSettings getGraphSettings() {
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
