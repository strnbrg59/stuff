/*
  File:        PrintDataDialog.java
  Authors:     Lennig, Sternberg
  Date:
  Description: Convenience for defining new grammars.
*/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

/**
   This Dialog pops up when the user presses the "clone" button on the
   main frame.
*/
public class PrintDataDialog extends JDialog
{
    public PrintDataDialog(JFrame parent,
                           String name,
                           boolean is_modal )
    {
        super(parent, name, is_modal);
        setTitle("Summary of the Data");
        registerListeners();
    }

    private void registerListeners()
    {
        addWindowListener( new WindowAdapter(){
            public void windowClosing(WindowEvent event)
            {
                dispose();
            }
        });
    }

    /** Append the new grammar to the database.  But reject the new
     *  grammar if its name is the same as that of some existing grammar.
     */
    void saveButton_MousePressed(MouseEvent event)
    {
        String name = m_grammar_name_panel.getText();

        // Check if name is already in database.  If yes, then reject name,
        // and do something menacing...like flashing red.
        for (int i=0; i < GrammarDatabase.size(); i++)
        {
            if (name.equals( GrammarDatabase.elementAt(i).name))
            {
                JOptionPane.showMessageDialog( 
                  ParentFrame.getParentFrame(this), 
                  "Please select a new name for the grammar", 
                  "clone error", 
                  JOptionPane.ERROR_MESSAGE );
                return;
            }
        }

        // Get load and duration of new grammar.

        Double d = new Double(0.0);
        d=d.valueOf(m_grammar_load_panel.getText());
        double ld=d.doubleValue();
    
        d=d.valueOf(m_duration_panel.getText());
        double dur=d.doubleValue();

        GrammarDatabase.append( new Grammar(name,ld,dur) );
    }

    /** Pressed Cancel or Close */
    void cancelButton_MousePressed(MouseEvent event)
    {
        dispose();
    }

    // Instance variables.
    private PanelWithTextFieldAndLabel     m_grammar_name_panel;
    private PanelWithTextFieldAndTwoLabels m_duration_panel;
    private PanelWithTextFieldAndTwoLabels m_grammar_load_panel;
/*
    private JButton                        m_save_button;
    private JButton                        m_cancel_button;
    private JButton                        m_close_button;
*/
}
