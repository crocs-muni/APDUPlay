/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.settings;

import lombok.Getter;
import lombok.Setter;
import parser.output.OutputType;
import parser.settings.graph.GraphSettings;
import parser.settings.output.TextOutputSettings;

/**
 *
 * @author Andrej
 */
public class Settings {
    
    /**
     * Output directory
     * 
     * @param outputDirectory output directory
     * @return output directory
     */
    @Getter @Setter private String outputDirectory;
    
    /**
     * Represents byte separator of byte stream of one node in outputs 
     * 
     * @param bytesSeparator byte separator
     * @return byte separator
     */
    @Getter @Setter private String bytesSeparator;
    
    /**
     * Determines if every packet should be in separated file
     * 
     * @param separatePackets boolean value
     * @return Boolean value that determines if packets should be separated
     */
    @Getter @Setter private boolean separatePackets;
    
    /**
     * Determines if every node in tree should contain only 1 byte
     * 
     * @param simpleNodes boolean value
     * @return Boolean value that determines if nodes should contain only 1 byte
     */
    @Getter @Setter private boolean simpleNodes;
    
    /**
     * Determines if minimalConstantLength should be applied if length of data stream is shorter than length specified
     * 
     * @param checkMinimalLengthOnShorterStreams boolean value
     * @return Boolean value that determines if minimalConstantLength should be applied if length of data stream is shorter than length specified
     */
    @Getter @Setter private boolean checkMinimalLengthOnShorterStreams;
    
    /**
     * Output type mask
     * 
     * @param outputTypeMask output type mask
     * @return output type mask
     */
    @Getter @Setter private int outputTypeMask;
    
    /**
     * Header length
     * 
     * @param headerLength header length
     * @return header length
     */
    @Getter @Setter private int headerLength;
    
    /**
     * Represents minimal length of constant bytes in data stream to be analyzed as a constant
     * 
     * @param minimalConstantLength length of constant bytes in data stream to be analyzed as a constant
     * @return minimal length of constant bytes in data stream to be analyzed as a constant
     */
    @Getter @Setter private int minimalConstantLength;
    
    /**
     * Represents Date time pattern to be used
     * 
     * @param dateTimePattern date time pattern to be used
     * @return date time pattern to be used
     */
    @Getter @Setter private String dateTimePattern;
    
    /**
     * Gets Graphviz settings
     * 
     * @return Graphviz settings
     */
    @Getter private final GraphSettings graphSettings = new GraphSettings();
    
    /**
     * Gets Graphviz settings
     * 
     * @return Graphviz settings
     */
    @Getter private final TextOutputSettings textOutputSettings = new TextOutputSettings();

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
        dateTimePattern = "yyyy-MM-dd HH:mm:ss";
        
        graphSettings.setRankDir("LR");
        graphSettings.setNodeAttributes("color=lightblue2, style=filled");
        graphSettings.setRandomByteStreamColor("red");
        graphSettings.setSimilarByteStreamColor("darkorchid4");
        
        textOutputSettings.setBytesSeparator(";");
        textOutputSettings.setByteEnumerationSeparator(",");
        textOutputSettings.setRandomByteValue("x");
        textOutputSettings.setEmptyByteValue("-");
        textOutputSettings.setEmptyByteIncluded(true);
    }
}
