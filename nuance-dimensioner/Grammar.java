/*
  File:        Grammar.java
  Author:      Lennig, then Sternberg
  Date:
  Description: Grammar name and processing costs.
*/

import java.util.*;

/** Grammar name and processing costs. */
public class Grammar
{
    public String name;
    public double load;
    public double duration;

    private Grammar() { }

    public Grammar( String name, double load, double duration )
    {
        this.name     = name;
        this.load     = load;
        this.duration = duration;
    }
    
    /** Parse out the name, load and duration. */
    public Grammar( String str ) throws BackEndException
    {
        StringTokenizer st = 
          new StringTokenizer( str, GrammarList.s_intra_grammar_delim );
        if( st.countTokens() != 3 )
        {
            throw new BackEndException(
                "Wrong number of tokens in Grammar ctor(String).  Found"
                + st.countTokens() + ", should be 3." );
        }

        name     = st.nextToken();
        load     = (new Double(st.nextToken())).doubleValue();
        duration = (new Double(st.nextToken())).doubleValue();
    }

    public Grammar( Grammar g )
    {
        name = new String(g.name);
        load = g.load;
        duration = g.duration;
    }

    public String toString()
    {
        StringBuffer sb = new StringBuffer();
        sb.append( name + GrammarList.s_intra_grammar_delim );
        sb.append( load + GrammarList.s_intra_grammar_delim );
        sb.append( duration );

        return new String(sb);
    }

    /** For use by Vector.indexOf().  The argument must be of type Object
     *  (we typecast to Grammar later); if the argument were a Grammar,
     *  the system wouldn't consider this method as a reimplementation of
     *  Object.equals(), and it would never get called.
    */
    public boolean equals( Object other )
    {
        Grammar g = (Grammar)other;

        if( name.equals( g.name )
        &&  load == g.load
        &&  duration == g.duration )
        {
            return true;
        }
        else return false;
    }

}
