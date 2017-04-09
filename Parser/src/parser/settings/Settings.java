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
    private String dateTimePattern;
    private final GraphSettings graphSettings = new GraphSettings();

    /**
     * Sets default values for all properties
     */
    public void setDefaults() {
        outputDirectory = "output";
        separatePackets = false;
        simpleNodes = false;
        outputTypeMask = OutputType.ALL;
        headerLength = 4;
        minimalConstantLength = 0;
        bytesSeparator = " ";
        dateTimePattern = "yyyy-MM-dd hh:mm:ss";
        
        graphSettings.setRankDir("LR");
        graphSettings.setNodeAttributes("color=lightblue2, style=filled");
        graphSettings.setRandomByteStreamColor("red");
        graphSettings.setSimilarByteStreamColor("darkorchid4");
    }
    
    /**
     * Sets the output directory
     * 
     * @param outputDir output directory
     */
    public void setOutputDirectory(String outputDir) {
        outputDirectory = outputDir;
    }
    
    /**
     * Gets the output directory
     * 
     * @return output directory
     */
    public String getOutputDirectory() {
        return outputDirectory;
    }
    
    /**
     * Sets if every packet should be in separated file
     * 
     * @param val boolean value
     */
    public void setSeparatePackets(boolean val) {
        separatePackets = val;
    }
    
    /**
     * Gets if every packet should be in separated file
     * 
     * @return Boolean value that determines if packets should be separated
     */
    public boolean separatePackets() {
        return separatePackets;
    }
    
    /**
     * Sets if every node in tree should contain only 1 byte
     * 
     * @param val boolean value
     */
    public void setSimpleNodes(boolean val) {
        simpleNodes = val;
    }
    
    /**
     * Gets if every node in tree should contain only 1 byte
     * 
     * @return Boolean value that determines if nodes should contain only 1 byte
     */
    public boolean simpleNodes() {
        return simpleNodes;
    }
    
    /**
     * Sets output type mask
     * 
     * @param mask output type mask
     */
    public void setOutputTypeMask(int mask) {
        outputTypeMask = mask;
    }
    
    /**
     * Gets output type mask
     * 
     * @return output type mask
     */
    public int getOutputTypeMask() {
        return outputTypeMask;
    }
    
    /**
     * Sets header length
     * 
     * @param length header length
     */
    public void setHeaderLength(int length) {
        headerLength = length;
    }
    
    /**
     * Gets header length
     * 
     * @return header length
     */
    public int getHeaderLength() {
        return headerLength;
    }
    
    /**
     * Sets byte separator of byte stream of one node in outputs 
     * 
     * @param val byte separator
     */
    public void setBytesSeparator(String val) {
        bytesSeparator = val;
    }
    
    /**
     * Gets byte separator of byte stream of one node in outputs 
     * 
     * @return byte separator
     */
    public String getBytesSeparator() {
        return bytesSeparator;
    }
    
    /**
     * Gets Graphviz settings
     * 
     * @return Graphviz settings
     */
    public GraphSettings getGraphSettings() {
        return graphSettings;
    }
    
    /**
     * Gets minimal length of constant bytes in data stream to be analyzed as a constant
     * 
     * @return minimal length of constant bytes in data stream to be analyzed as a constant
     */
    public int getMinimalConstantLength() {
        return minimalConstantLength;
    }
    
    /**
     * Sets minimal length of constant bytes in data stream to be analyzed as a constant
     * 
     * @param length minimal length of constant bytes in data stream to be analyzed as a constant
     */
    public void setMinimalConstantLength(int length) {
        minimalConstantLength = length;
    }
    
    /**
     * Gets if minimalConstantLength should be applied if length of data stream is shorter than length specified
     * 
     * @return Boolean value that determines if minimalConstantLength should be applied if length of data stream is shorter than length specified
     */
    public boolean getCheckMinimalLengthOnShorterStreams() {
        return checkMinimalLengthOnShorterStreams;
    }
    
    /**
     * Sets if minimalConstantLength should be applied if length of data stream is shorter than length specified
     * 
     * @param check boolean value
     */
    public void setCheckMinimalLengthOnShorterStreams(boolean check) {
        checkMinimalLengthOnShorterStreams = check;
    }
    
    /**
     * Sets date time pattern to be used
     * 
     * @param pattern date time pattern to be used
     */
    public void setDateTimePattern(String pattern) {
        this.dateTimePattern = pattern;
    }
    
    /**
     * Gets date time pattern to be used
     * 
     * @return date time pattern to be used
     */
    public String getDateTimePattern() {
        return dateTimePattern;
    }
}
