/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.settings.graph;

/**
 *
 * @author Andrej
 */
public class GraphSettings {
    private String rankDir;
    private String size;
    private String nodeAttributes;
    private String randomByteStreamColor;
    private String similarByteStreamColor;
    
    /**
     * Sets rankdir property of Graphviz settings
     * 
     * @param val rankdir to be set
     */
    public void setRankDir(String val) {
        rankDir = val;
    }
    
    /**
     * Gets rankdir property of Graphviz settings
     * 
     * @return rankdir property of Graphviz settings
     */
    public String getRankDir() {
        return rankDir;
    }
    
    /**
     * Sets size property of Graphviz settings
     * 
     * @param val size to be set
     */
    public void setSize(String val) {
        size = val;
    }
    
    /**
     * Gets size property of Graphviz settings
     * 
     * @return size property of Graphviz settings
     */
    public String getSize() {
        return size;
    }
    
    /**
     * Sets node attributes of Graphviz settings
     * 
     * @param val node attributes to be set
     */
    public void setNodeAttributes(String val) {
        nodeAttributes = val;
    }
    
    /**
     * Gets node attributes of Graphviz settings
     * 
     * @return node attributes of Graphviz settings
     */
    public String getNodeAttributes() {
        return nodeAttributes;
    }
    
    /**
     * Sets color of bytes that have been analyzed as random
     * 
     * @param val color of bytes that have been analyzed as random
     */
    public void setRandomByteStreamColor(String val) {
        randomByteStreamColor = val;
    }
    
    /**
     * Gets color of bytes that have been analyzed as random
     * 
     * @return color of bytes that have been analyzed as random
     */
    public String getRandomByteStreamColor() {
        return randomByteStreamColor;
    }
    
    /**
     * Sets color of bytes that have been analyzed as similar
     * 
     * @param val color of bytes that have been analyzed as similar
     */
    public void setSimilarByteStreamColor(String val) {
        similarByteStreamColor = val;
    }
    
    /**
     * Gets color of bytes that have been analyzed as similar
     * 
     * @return color of bytes that have been analyzed as similar
     */
    public String getSimilarByteStreamColor() {
        return similarByteStreamColor;
    }
}
