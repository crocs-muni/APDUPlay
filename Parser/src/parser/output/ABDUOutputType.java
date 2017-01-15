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
public class ABDUOutputType {
    public static final int NODES               = 0x1;
    public static final int FLOW                = 0x2;
    public static final int PACKETS             = 0x4;
    public static final int PACKETS_ANALYZED    = 0x8;
    public static final int ALL                 = -1;
    
    /**
     * All output types as an array
     */
    public static final int[] TYPES = { NODES, FLOW, PACKETS, PACKETS_ANALYZED };
}
