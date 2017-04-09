/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package apduparse;

import com.google.gson.Gson;
import com.google.gson.JsonSyntaxException;
import java.io.IOException;
import lombok.val;
import parser.logging.Logger;
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
        
        val logger = new Logger("Output/log.txt");
        val settingsLoader = new APDUSettingsLoader();
        Settings settings;
        try {
            settingsLoader.load("APDUParse.conf");
            val gson = new Gson();
            settings = gson.fromJson(settingsLoader.getJsonSettings(), Settings.class);
            logger.setDateTimePattern(settings.getDateTimePattern());
            
        } catch (JsonSyntaxException | IOException ex) {
            logger.error(ex.getMessage());
            logger.warning("Error ocured while parsing config file. Default values will be used.");
            
            settings = new Settings();
            settings.setDefaults();
        }
        
        val parser = new Parser(settings, logger);
        parser.parseFile(args[0]);
        parser.printData();
    }
}
