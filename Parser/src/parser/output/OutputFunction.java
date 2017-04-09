/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.output;

import java.io.PrintWriter;
import java.util.function.BiConsumer;
import parser.data.Tree;

/**
 *
 * @author Andrej
 */
public class OutputFunction {
    private final BiConsumer<Tree, PrintWriter> transmittedFunction;
    private final BiConsumer<Tree, PrintWriter> receivedFunction;
    
    /**
     * Determines if output should be wrapped as graphviz output
     */
    public final boolean wrapGraphvizOutput;
    
    /**
     * Creates new instance of OutputFunction
     * 
     * @param transmittedFunction   transmitted data output function
     * @param receivedFunction      receicved data output function
     */
    public OutputFunction(BiConsumer<Tree, PrintWriter> transmittedFunction, BiConsumer<Tree, PrintWriter> receivedFunction) {
        this(transmittedFunction, receivedFunction, true);
    }
    
    /**
     * Creates new instance of OutputFunction
     * 
     * @param transmittedFunction   transmitted data output function
     * @param receivedFunction      receicved data output function
     * @param wrapGraphvizOutput    determines if output should be wrapped as graphviz output
     */
    public OutputFunction(BiConsumer<Tree, PrintWriter> transmittedFunction, BiConsumer<Tree, PrintWriter> receivedFunction, boolean wrapGraphvizOutput) {
        this.transmittedFunction = transmittedFunction;
        this.receivedFunction = receivedFunction;
        this.wrapGraphvizOutput = wrapGraphvizOutput;
    }
    
    /**
     * Invokes transmitted data output function
     * 
     * @param tree      tree which output function will use
     * @param writer    writer which output function will use
     */
    public void invokeTransmitted(Tree tree, PrintWriter writer) {
        if (this.transmittedFunction != null) {
            this.transmittedFunction.accept(tree, writer);
        }
    }
    
    /**
     * Invokes received data output function
     * 
     * @param tree      tree which output function will use
     * @param writer    writer which output function will use
     */
    public void invokeReceived(Tree tree, PrintWriter writer) {
        if (this.receivedFunction != null) {
            this.receivedFunction.accept(tree, writer);
        }
    }
    
    /**
     * Informs about existence of transmitted data output function
     * 
     * @return true if transmitted data output function exists, false otherwise
     */
    public boolean hasTransmittedFunction() {
        return this.transmittedFunction != null;
    }
    
    /**
     * Informs about existence of received output function
     * 
     * @return true if received data output function exists, false otherwise
     */
    public boolean hasReceivedFunction() {
        return this.receivedFunction != null;
    }
}
