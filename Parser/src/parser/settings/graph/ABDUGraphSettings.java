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
public class ABDUGraphSettings {
    private String rankDir;
    private String size;
    private String nodeAttributes;
    
    public void setRankDir(String val) {
        rankDir = val;
    }
    
    public String getRankDir() {
        return rankDir;
    }
    
    public void setSize(String val) {
        size = val.replace('.', ',');
    }
    
    public String getSize() {
        return size;
    }
    
    public void setNodeAttributes(String val) {
        nodeAttributes = val;
    }
    
    public String getNodeAttributes() {
        return nodeAttributes;
    }
}
