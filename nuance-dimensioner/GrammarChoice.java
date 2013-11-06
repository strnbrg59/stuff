/*
  File:        GrammarChoice.java
  Author:      Sternberg
  Date:
  Description: Specialization of Choice, for use in the grammar Choice widget.
*/

import java.util.*;

/** Specialization of Choice, for use in the grammar Choice widget.
 *  In a standard awt.Choice object, the elements are Strings.  But
 *  we want to work in terms of Grammars (and pass these around to 
 *  the GrammarList widget, and to some of BackEnd's methods).  The
 *  String representation should only matter when it comes time to 
 *  display elements of this class in the widget; the String
 *  representation is thus an implementation detail of this class, and
 *  not part of its public interface.
*/
public class GrammarChoice extends java.awt.Choice
{
    /** Add the grammar to this object's internal vector, and add its
     *  string representation to the display in the widget.
    */
    public void addItem( Grammar grammar )
    {
        m_rep.addElement( grammar );
        super.addItem( grammar.name );
    }

    public Grammar getSelectedGrammar()
    {
        return new Grammar((Grammar)m_rep.elementAt( super.getSelectedIndex()));
    }        

    public void removeAll()
    {
        m_rep.removeAllElements();
        super.removeAll();
    }    

    /** Same as in super. */
    public GrammarChoice()
    {
        super();
        m_rep = new Vector();
    }

    /** A vector of grammars.  We augment it in add(), and remove from
     *  it in delItem().
    */
    private Vector m_rep;
}
