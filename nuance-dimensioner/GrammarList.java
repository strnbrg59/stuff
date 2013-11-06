/*
  File:        GrammarList.java
  Author:      Sternberg
  Date:
  Description: Specialization of List, for use in the grammar List widget.
*/

import java.awt.*;
import java.util.*;
import javax.swing.*;

/** Specialization of List, for use in the grammar List widget.
 *  Elements are not Strings, but rather instances of the 
 *  GrammarAndModifiers class (defined in this file) which is a bundle 
 *  consisting of a Grammar, a number-of-requests, and a verify flag.
*/
public class GrammarList extends java.awt.List
{
    /** Add the grammar to this object's internal vector, and add its
     *  string representation to the display in the widget.
     *  We don't call this add() because then it's considered to override
     *  List.add(), which doesn't throw BackEndException, and then that's
     *  a compile error.
    */
    public void addElement( Grammar grammar, double requests, boolean verify )
    {
      add(
        new GrammarAndModifiers(grammar,requests,verify).getRequestsAndName());
      m_rep.addElement( new GrammarAndModifiers(grammar,requests,verify) );
    }

    /** str contains one GrammarAndModifiers object.  Parse and append
     *  it to this object.  Useful for loading from a file.
     *  For why we don't call this method add(), see comments for
     *  addElement(Grammar,double).
    */
    public void addElement( String str ) throws BackEndException
    {
        GrammarAndModifiers newElem = new GrammarAndModifiers(str);
        this.addElement( 
          newElem.m_grammar, newElem.m_requests, newElem.m_verify );
    }

    /** Return index-th GrammarAndModifiers */
    public GrammarAndModifiers grammarAndModifiersAt( int index )
    {
        return (GrammarAndModifiers)m_rep.elementAt(index);
    }

    /** Return index-th Grammar */
    public Grammar grammarAt( int index )
    {
        return new Grammar( 
            ((GrammarAndModifiers)m_rep.elementAt(index)).m_grammar
        );
    }

    /** Return requests-per-call associated with index-th Grammar */
    public double requestsAt( int index )
    {
        return ((GrammarAndModifiers)m_rep.elementAt(index)).m_requests;
    }

    /** Return state of verify boolean field. */
    public boolean verifyAt( int index )
    {
        return ((GrammarAndModifiers)m_rep.elementAt(index)).m_verify;
    }

    /** Remove a grammar from this object's internal vector, and remove
     *  its string representation from the display in the widget.
    */
    public void delItem( int index )
    {
        m_rep.removeElementAt( index );
        super.delItem( index );
    }

    public void replaceItem( GrammarAndModifiers item, int index )
    {
        super.replaceItem( item.getRequestsAndName(), index );
        // I think super.replaceItem() is calling delItem(), and we've
        // defined our own delItem() which deletes an element of m_rep.
        // So we'll have to reinsert the element into m_rep:
        m_rep.insertElementAt( item, index );
    }

    /** Delete all the elements. */
    public void clearAll()
    {
        int n = m_rep.size();
        for( int i=0;i<n;i++ )
        {
            this.delItem(0);
        }
    }

    /** Write all the elements of m_rep, delimiting them with '\n'; */
    public String toString()
    {
        StringBuffer sb = new StringBuffer();
        for( int i=0;i<m_rep.size();i++ )
        {
            sb.append( m_rep.elementAt(i) + "\n" );
        }

        if( m_trans_prob_matrix != null )
        {
            sb.append( FrontEnd.DASHES + "\n" );
            sb.append( "Transition probability matrix:\n" );
            for( int i=0;i<m_trans_prob_matrix.length;i++ )
            {
                for( int j=0;j<m_trans_prob_matrix.length;j++ )
                {
                    sb.append( (m_trans_prob_matrix[i][j]).getText() );
                    sb.append( " " );
                }
                sb.append( '\n' );
            }
        }

        return new String(sb);
    }

    /** For deserialization */
    void fromString( String str )
    {
// IMPLEMENT ME: 
    }


    /** For serialization */
    void setTransProbMatrix( JTextField[][] p )
    {
        m_trans_prob_matrix = p;
    }

    /** Same as in super. */
    public GrammarList( int num_lines )
    {
        super( num_lines );
        m_rep = new Vector();
    }

    /** A vector of elements of type GrammarAndModifiers.  
     *  We augment it in add(), and remove from it in delItem().
    */
    private Vector m_rep;
    static String s_intra_grammar_delim = "|";
    static String s_inter_grammar_delim = "!";
    private JTextField m_trans_prob_matrix[][] = null;
    private JTextField hangup_prob = null;
}
