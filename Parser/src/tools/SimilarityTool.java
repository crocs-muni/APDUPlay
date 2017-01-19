/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package tools;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 *
 * @author Andrej
 */
public class SimilarityTool {

    /**
     * Compares the two strings based on letter pair matches
     * 
     * @param str1
     * @param str2
     * @return percentage match from 0.0 to 1.0 where 1.0 is 100%
     */
    public static double compareStrings(String str1, String str2)
    {
        List<String> pairs1 = wordLetterPairs(str1.trim());
        List<String> pairs2 = wordLetterPairs(str2.trim());

        int intersection = 0;
        int union = pairs1.size() + pairs2.size();

        for (int i = 0; i < pairs1.size(); i++)
        {
            for (int j = 0; j < pairs2.size(); j++)
            {
                if (pairs1.get(i) == null ? pairs2.get(j) == null : pairs1.get(i).equals(pairs2.get(j)))
                {
                    intersection++;
                    pairs2.remove(j); // prevents "GGGG" to match "GG" with 100% success

                    break;
                }
            }
        }

        return (2.0 * intersection) / union;
    }
    
    private static List<String> wordLetterPairs(String str)
    {
        // All stream strings are already pairs so we just split them
        return new ArrayList(Arrays.asList(str.split(" ")));
    }
}
