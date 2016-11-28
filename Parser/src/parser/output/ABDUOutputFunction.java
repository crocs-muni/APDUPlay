/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.output;

import java.io.PrintWriter;
import java.util.function.BiConsumer;
import parser.ABDUTree;

/**
 *
 * @author Andrej
 */
public class ABDUOutputFunction {
    
    private final BiConsumer<ABDUTree, PrintWriter> transmittedFunction;
    private final BiConsumer<ABDUTree, PrintWriter> receivedFunction;
    
    public ABDUOutputFunction(BiConsumer<ABDUTree, PrintWriter> transmittedFunction, BiConsumer<ABDUTree, PrintWriter> receivedFunction) {
        this.transmittedFunction = transmittedFunction;
        this.receivedFunction = receivedFunction;
    }
    
    public void invokeTransmitted(ABDUTree tree, PrintWriter writer) {
        if (this.transmittedFunction != null) {
            this.transmittedFunction.accept(tree, writer);
        }
    }
    
    public void invokeReceived(ABDUTree tree, PrintWriter writer) {
        if (this.receivedFunction != null) {
            this.receivedFunction.accept(tree, writer);
        }
    }
    
    public boolean hasTransmittedFunction() {
        return this.transmittedFunction != null;
    }
    
    public boolean hasReceivedFunction() {
        return this.receivedFunction != null;
    }
}
