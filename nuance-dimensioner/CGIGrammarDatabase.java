/*
  File:        CGIGrammarDatabase.java
  Author:      Sternberg
  Date:
  Description: Subclass of GrammarDatabase: loads extra grammars that the user
               has defined in previous sessions (i.e. on previous dynamic
               web pages).
*/

import java.util.*;

public class CGIGrammarDatabase extends GrammarDatabase
{
    /** If the user specified a new grammar last time, then its name, lu and
     *  duration are in text fields (now in the hashtable) named newGrammarName,
     *  newGrammarLU and newGrammarDur.  We add that grammar to our database,
     *  provided its name is not a duplicate.
     *
     *  We may have other user-defined grammars, defined in preceding sessions.
     *  These are held in a hidden field.  We append to that hidden field the
     *  specifications of last session's new grammar (if any).
     *
     *  @return Number of extra grammars.
    */
    public static int loadExtraGrammars( Hashtable cgi_query_hash )
      throws BackEndException
    {
        // Was a new grammar defined in the previous session?
        String name    = (String)cgi_query_hash.get("newGrammarName");
        String lu_str  = (String)cgi_query_hash.get("newGrammarLU");
        String dur_str = (String)cgi_query_hash.get("newGrammarDur");
        if( (((String)cgi_query_hash.get("submit")).equals( "New grammar" ))
        &&  (name != null) && (lu_str != null) && (dur_str != null) )
        try
        {        
            // Check if name is already in database.  If yes, then reject name,
            // and do something menacing.  But this won't catch duplicates of
            // other new grammars, because we don't load them into the database
            // until later in this method.
            for (int i=0; i < CGIGrammarDatabase.size(); i++)
            {
                if (name.equals( CGIGrammarDatabase.elementAt(i).name))
                {
                    throw new BackEndException("Duplicate grammar: " + name );
                }
            }

            Double d = new Double(0.0);
            d = d.valueOf( lu_str );
            double lu = d.doubleValue();
            d = d.valueOf( dur_str );
            double duration = d.doubleValue();

            String newExtraGrammar = name                              + 
                                     GrammarList.s_intra_grammar_delim +
                                     lu                                +
                                     GrammarList.s_intra_grammar_delim +
                                     duration                          +
                                     GrammarList.s_inter_grammar_delim;
            String preexistingExtraGrammars =
              (String)cgi_query_hash.get("extraGrammars");
            String allExtraGrammars;
            if( preexistingExtraGrammars == null )
            {
                allExtraGrammars = newExtraGrammar;
            }
            else
            {
                allExtraGrammars = preexistingExtraGrammars          +
                                   newExtraGrammar;
            }

            cgi_query_hash.remove( "extraGrammars" );
            cgi_query_hash.put( "extraGrammars", allExtraGrammars );
        }
        catch( Exception e ) { e.printStackTrace(); }

        int result=0; // number of extra grammars
        String extraGrammars = (String)cgi_query_hash.get("extraGrammars");
        if( extraGrammars != null )
        {
            StringTokenizer st = new StringTokenizer(
              extraGrammars, GrammarList.s_inter_grammar_delim );
            while( st.hasMoreTokens() )
            {
                Grammar g = new Grammar( (String)st.nextToken() );
                m_rep.addElement( g );
                result++;
            }
        }
        return result;
    }

    /** Ideally, the web page with the grammar's documentation would come up
     *  on the user's screen.  But I don't know how to do that.  So instead,
     *  I print an <A HREF...> thing for him to click on.
     *
     */
/*
    // This feature isn't supported anymore.  The last time it was supported
    // was at CVS tag last_url_jumping_version.
    public static void gotoGrammarWebPage( Grammar gram )
    {
        CGIForm.println( "<HTML><CENTER><FONT SIZE=12>" );        

        String href = "<A HREF=\"" + gram.url + 
          "\"> click here for documentation on " + gram.name + "</A>";
        CGIForm.println( href );
        CGIForm.println( "</FONT></CENTER></HTML>" );
    }
*/
}
