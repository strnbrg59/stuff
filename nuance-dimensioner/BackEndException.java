/*
  File:        BackEndException.java
  Author:      Sternberg
  Date:
  Description: Catch-all exception class.  FrontEnd catches these and pops up
    an error dialog.  It's a way to get those nice error dialogs (instead of
    printing an error message to stdout or exiting), while localizing all 
    graphics stuff in the FrontEnd.
*/

public class BackEndException extends Exception
{
    public BackEndException( String message )
    {
        super( message );
    }
    
}
