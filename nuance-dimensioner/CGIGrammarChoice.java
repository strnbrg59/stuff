/*
  File:        CGIGrammarChoice.java
  Author:      Sternberg
  Date:
  Description: Subclass of GrammarChoice, the drop-down choice widget in our
               applet.  Specialized for IO with a CGI form.
*/

import java.util.*;

public class CGIGrammarChoice extends GrammarChoice
{
    /** Find which item has been selected in the CGI form, and call
     *  Choice.select(String).
    */
    void setSelected( Hashtable cgiQueryHash )
    {
        //CGIForm.traceln( "hash = " + cgiQueryHash );

        String selected = (String)cgiQueryHash.get("Grammar database");
        if( selected != null )
        {
            select( selected );
        }
    }

    /** Print html text, to stdout, that a browser would interpret as a
     *  CGI scrolled selection list.
    */
    void toHtml()
    {
        CGIForm.println( "<SELECT NAME=\"Grammar database\" SIZE=1>" );

        for( int i=0;i<getItemCount();i++ )
        {
            CGIForm.print( "<OPTION VALUE=\"" + getItem(i) + "\"" );
            if( i == getSelectedIndex() )
            {
                CGIForm.print( " SELECTED" );
            }
            CGIForm.println( "> " + getItem(i) );
        }

        CGIForm.println( "</SELECT><P>" );
    }
}
