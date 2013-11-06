/*
  File:        GrammarAndModifiers.java
  Author:      Sternberg
  Date:        February 2001
  Description: Contains a Grammar, its associated requests-per-call,
               and the verify boolean.
*/

import java.util.*;

public class GrammarAndModifiers
{
    public GrammarAndModifiers( Grammar grammar, double requests, boolean verify )
    {
        this.m_grammar  = new Grammar(grammar);
        this.m_requests = requests;
        this.m_verify   = verify;
    }

    /** Parses the format produced by toString().
     *  Needed for loading from a file. 
    */
    public GrammarAndModifiers( String str ) throws BackEndException
    {
        StringTokenizer st = 
          new StringTokenizer( str, GrammarList.s_intra_grammar_delim );

        if( st.countTokens() != 5)
        {
            throw new BackEndException(
              "Wrong number of tokens in grammar-model data.  " +
              "Check input file.");
        }

        m_requests = (new Double(st.nextToken())).doubleValue();
        m_verify = (new Boolean(st.nextToken())).booleanValue();

        // Use the Grammar ctor on the rest of the string.
        StringBuffer theRest = new StringBuffer();
        theRest.append( st.nextToken() );
        while( st.hasMoreTokens() )
        {
            theRest.append( GrammarList.s_intra_grammar_delim );
            theRest.append( st.nextToken() );
        }
        m_grammar = new Grammar( new String(theRest) );
    }        

    public String toString()
    {
        StringBuffer sb = new StringBuffer( Double.toString(m_requests) );
        sb.append( GrammarList.s_intra_grammar_delim );
        sb.append( (new Boolean(m_verify)).toString() );
        sb.append( GrammarList.s_intra_grammar_delim );
        sb.append( m_grammar.toString() );
        return new String( sb );
    }

    /** Like toString(), but print only the number of requests, the grammar
     *  name and the verify flag -- for use in the List display.
    */
    public String getRequestsAndName()
    {
        StringBuffer sb = new StringBuffer( Double.toString(m_requests) );
        sb.append( GrammarList.s_intra_grammar_delim );
        sb.append( m_grammar.name );
        if( m_verify == true )  // put a 'v' in there if verification is true.
        {
            sb.append( GrammarList.s_intra_grammar_delim );
            sb.append( "v" );
        }

        return new String( sb );
    }
        
    Grammar m_grammar;
    double  m_requests;
    boolean m_verify;
}
