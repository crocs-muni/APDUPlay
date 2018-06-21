/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.settings.graph;

import lombok.Getter;
import lombok.Setter;

/**
 *
 * @author Andrej
 */
public class GraphSettings {
    
    /**
     * Represents rankdir property of Graphviz settings
     * 
     * @param rankDir rankdir to be set
     * @return rankdir property of Graphviz settings
     */
    @Getter @Setter private String rankDir;
    
    /**
     * Represents size property of Graphviz settings
     * 
     * @param size size to be set
     * @return size property of Graphviz settings
     */
    @Getter @Setter private String size;
    
    /**
     * Represents node attributes of Graphviz settings
     * 
     * @param nodeAttributes node attributes to be set
     * @return node attributes of Graphviz settings
     */
    @Getter @Setter private String nodeAttributes;
    
    /**
     * Represents color of bytes that have been analyzed as random
     * 
     * @param randomByteStreamColor color of bytes that have been analyzed as random
     * @return color of bytes that have been analyzed as random
     */
    @Getter @Setter private String randomByteStreamColor;
    
    /**
     * Represents color of bytes that have been analyzed as similar
     * 
     * @param similarByteStreamColor color of bytes that have been analyzed as similar
     * @return color of bytes that have been analyzed as similar
     */
    @Getter @Setter private String similarByteStreamColor;
    
    /**
     * Represents the number of bytes after which the output should be wrapped
     * 
     * @param wrapAfter number of bytes after which the output get wrapped
     * @return number of bytes after which the output get wrapped
     */
    @Getter @Setter private int wrapAfter;
}
