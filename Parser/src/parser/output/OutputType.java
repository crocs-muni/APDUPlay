/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.output;

/**
 *
 * @author Andrej
 */
public class OutputType {

    /**
     * Simple nodes output. Represents internal structure as the stream is represent in the memory
     */
    public static final int NODES                   = 0x1;

    /**
     * Simple flow output. Nodes represents by color the frequency of the current byte. Nodes got only 1 byte and its always the most frequent one
     */
    public static final int FLOW                    = 0x2;

    /**
     * Flow represented as packets with response time and their index of occurrence
     */
    public static final int PACKETS                 = 0x4;

    /**
     * Flow represented as packets with analyzed bytes in data stream by their frequency
     */
    public static final int PACKETS_ANALYZED        = 0x8;
    
    /**
     * Flow represented as packets with analyzed bytes in data stream by their frequency in text format
     */
    public static final int PACKETS_ANALYZED_TEXT   = 0x10;

    /**
     * Represents all output types
     */
    public static final int ALL                     = -1;
    
    /**
     * All output types as an array
     */
    public static final int[] TYPES = { NODES, FLOW, PACKETS, PACKETS_ANALYZED, PACKETS_ANALYZED_TEXT };
}
