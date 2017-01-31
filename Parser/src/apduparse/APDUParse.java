/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package apduparse;

import com.google.gson.Gson;
import java.io.IOException;
import parser.Logger;
import parser.Parser;
import parser.settings.Settings;

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
        
        Logger logger = new Logger();
        APDUSettingsLoader settingsLoader = new APDUSettingsLoader();
        Settings settings = null;
        try {
            settingsLoader.load("APDUParse.conf");
        } catch (IOException ex) {
            logger.error(ex.getMessage());
            logger.warning("Error ocured while parsing config file. Using default values.");
            
            settings = new Settings();
            settings.setDefaults();
        }
        
        if (settings == null) {
            Gson gson = new Gson();
            settings = gson.fromJson(settingsLoader.getJsonSettings(), Settings.class);
        }
        
        Parser parser = new Parser(settings, logger);
        parser.parseFile(args[0]);
        parser.printData();
    }
}
