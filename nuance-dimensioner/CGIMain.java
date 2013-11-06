/*
  File:        CGIMain.java
  Author:      Sternberg
  Date:
  Description: 
    Basis for CGI program that produces the HTML page that the user thinks of
    as the "Dimensioner GUI".
*/

public class CGIMain
{
    public static void main( String[] args )
    {
        CGIForm form = new CGIForm();

        if( args.length > 0 ) // ==0 only first time
        {
            // Extract the info that the POST method passed via stdin.
            try
            {
                int content_length = Integer.parseInt( args[0] );
                byte[] content_bytes = new byte[ content_length ];
                System.in.read( content_bytes );
                String content_str = new String( content_bytes );
                form.processQuery( content_str );
            }
            catch( Exception e )
            {
                System.err.println( e.toString() );
                System.exit(1);
            }
        }

        form.printPage();
    }
}
