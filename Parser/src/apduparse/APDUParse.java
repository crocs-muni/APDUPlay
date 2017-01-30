/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package apduparse;

import parser.Logger;
import parser.Parser;
import parser.output.OutputType;
import parser.settings.Settings;
import parser.settings.graph.GraphSettings;

/**
 *
 * @author Andrej
 */
public class APDUParse {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        if (args.length < 1) {
            throw new IllegalArgumentException("Path to file to parse is required as an argument");
        }
        
        Settings settings = new Settings();
        settings.setOutputDirectory("output");
        settings.setSeparatePackets(false);
        settings.setSimpleNodes(false);
        settings.setOutputTypeMask(OutputType.ALL);
        settings.setHeaderLength(4);
        settings.setMinimalConstantLength(3);
        settings.setBytesSeparator(" ");
        
        GraphSettings graphSettings = settings.getGraphSettings();
        graphSettings.setRankDir("LR");
        graphSettings.setNodeAttributes("color=lightblue2, style=filled");
        
        Parser parser = new Parser(settings, new Logger());
        parser.parseFile(args[0]);
        parser.printData();
    }
}
