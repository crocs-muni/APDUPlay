/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package apduparse;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

/**
 *
 * @author Andrej
 */
public class APDUSettingsLoader {
    
    private String jsonSettings;
    
    /**
     * Loads settings from file
     * 
     * @param fileName file to load
     * @throws IOException
     */
    public void load(String fileName) throws IOException {
        try (BufferedReader br = new BufferedReader(new FileReader(fileName))) {
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = br.readLine()) != null) {
               line = line.trim();
               if (line.isEmpty() || line.startsWith("#")) {
                   continue;
               }
               
               sb.append(line);
            }
            
            jsonSettings = sb.toString();
        }
    }
    
    /**
     * Gets settings in json string format
     * 
     * @return settings in json string format
     */
    public String getJsonSettings() {
        return jsonSettings;
    }
}
