package jqc.server;

import java.io.*;
import java.util.Properties;

class Passwords {

    static boolean verifyTrader( String trader, String pwd )
    {
        Properties props = new Properties();

        try {
            props.load(new BufferedInputStream(
                new FileInputStream( G.passwordsFile )));
        }
        catch (FileNotFoundException e) {
            System.err.println(e);
			e.printStackTrace();
            System.exit(1);
        }            
        catch (IOException e) {
            System.err.println(e);
			e.printStackTrace();
            System.exit(1);
        }            

        if( ( props.getProperty( trader ) != null )
		&&  (pwd.compareTo( props.getProperty( trader ) ) == 0 ) )
            return true;
        else
            return false;
    } // verifyTrader()
    //------------------------

} // class Passwords