/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.settings;

/**
 *
 * @author Andrej
 */
public class ABDUSettings {
    private String outputDirectory;
    private boolean separatePackets;
    private boolean simpleNodes;
    private int outputTypeMask;
    
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
    
    public void setsimpleNodes(boolean val) {
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
}
