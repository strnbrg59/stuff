/*
  File:        CGIGrammarList.java
  Author:      Sternberg
  Date:
  Description: Subclass of GrammarList, the text area widget in our
               applet.  Specialized for IO with a CGI form.
*/

import java.util.*;

public class CGIGrammarList extends GrammarList
{
    /** Find which item has been selected in the CGI form.
    */
    void setSelected( Hashtable cgi_query_hash )
    {
        String selected = (String)cgi_query_hash.get("Grammar list");
        if( selected != null )
        {
            int i=0;
            while( ! getItem(i).equals( selected ) )
            {
                i++;
            }
            select( i );
        }
    }

    /** Print html text, to stdout, that a browser would interpret as a
     *  CGI scrolled selection list.  Print also to a hidden field, because
     *  a CGI scrolled selection list considers its state to consist only of
     *  the selected item (if any).
    */
    void toHtml()
    {
        CGIForm.println( "<SELECT NAME=\"Grammar list\" SIZE=5>" );

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

        dumpToHiddenField();
    }

    /** This is how we remember the grammar model, across forms.  Otherwise,
     *  the contents of the grammar model would be lost, as CGI considers the
     *  state of a list widget to consist solely of the selected item (if any).
     *
     *  See "inverse" function loadFromHiddenField().
    */
    private void dumpToHiddenField()
    {
        CGIForm.print( "<INPUT TYPE=\"hidden\" " +
          "NAME=\"Grammar Model Hidden Field\" VALUE=\"" );

        for( int i=0;i<getItemCount();i++ )
        {
            if( i>0 ) CGIForm.print( s_inter_grammar_delim );
            CGIForm.print( getItem(i) );
        }

        CGIForm.println( "\"><BR>" );
    }

    /** Build the m_grammar_list object from the grammar model stored in
     *  the hidden field.
    */
    public void loadFromHiddenField( Hashtable cgi_query_hash )
    {
        String hidden = 
          (String)cgi_query_hash.get("Grammar Model Hidden Field");
        if( hidden == null )  // The case if the grammar model is empty.
        {
            return;
        }

        StringTokenizer st = 
          new StringTokenizer( hidden, s_inter_grammar_delim );
        while( st.hasMoreTokens() )
        {
            String tok = st.nextToken();

            Double d = new Double(0.0);

            // Grab the name of the grammar: it's the second subtoken.
            StringTokenizer st1 = 
              new StringTokenizer( tok, GrammarList.s_intra_grammar_delim );
            double num_requests = (d.valueOf( st1.nextToken())).doubleValue();
            Grammar gram = GrammarDatabase.elementAt(st1.nextToken());
            boolean verify = false;
            if( st1.hasMoreTokens() )
            {
                verify = true;
            }
            addElement( gram, num_requests, verify );
        }
    }

    public CGIGrammarList( int num_lines )
    {
        super( num_lines );
    }
}
