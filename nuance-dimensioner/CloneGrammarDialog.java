/*
  File: CloneGrammarDialog.java
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
public class CloneGrammarDialog extends JDialog
{
    public CloneGrammarDialog(JFrame parent,
                              String name,
                              double load,
                              double dur)
    {
        super(parent, name, false);

        setTitle("Clone a Grammar");

        m_grammar_name_panel = new PanelWithTextFieldAndLabel(
          name, 30,
          "Enter new grammar name" );

        m_grammar_load_panel = new PanelWithTextFieldAndTwoLabels(
          String.valueOf(load), 4,
          "Grammatical load of new grammar",
          "LU" );

        m_duration_panel = new PanelWithTextFieldAndTwoLabels(
          String.valueOf(dur), 4,
          "Average utterance duration",
          "s" );

        m_save_button = new ScalableJButton("Save");
        m_cancel_button = new ScalableJButton("Cancel");
        m_close_button = new ScalableJButton("Close"); // same effect as cancel.

        getContentPane().setLayout( new BorderLayout() );
        {
            // Place grammar-name textfield
            JPanel north = new JPanel();
            north.setLayout( new FlowLayout(FlowLayout.LEFT) );
            north.add( m_grammar_name_panel );

            // Place the rest.
            JPanel south = new JPanel();
            south.setLayout( new BorderLayout() );

            JPanel southwest = new JPanel();
            southwest.setLayout( new BorderLayout() );
            southwest.add( m_grammar_load_panel, "North" );
            southwest.add( m_duration_panel, "South" );
            
            JPanel southeast = new JPanel();
            southeast.setLayout( new GridLayout(4,1,0,0) );
            southeast.add( new JPanel() );
            southeast.add( m_save_button );
            southeast.add( m_close_button );
            southeast.add( m_cancel_button );
            
            south.add( southwest, "West" );
            south.add( southeast, "East" );

            getContentPane().add( north, "North" );
            getContentPane().add( south, "South" );

            pack();
            
            // Warning: Don't call setVisible() or show() here, as the
            // program will not continue beyond that point.
          }

        registerListeners();
        setVisible(true);
    }

    private void registerListeners()
    {
        addWindowListener( new WindowAdapter(){
            public void windowClosing(WindowEvent event)
            {
                setVisible(false);
                dispose();
            }
        });

        m_save_button.addMouseListener( new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                saveButton_MousePressed(event);
            }
        });

        m_close_button.addMouseListener( new MouseAdapter(){
            public void mouseClicked(MouseEvent event) {
                cancelButton_MouseClicked(event);
            }
        });

        m_cancel_button.addMouseListener( new MouseAdapter(){
            public void mouseClicked(MouseEvent event) {
                cancelButton_MouseClicked(event);
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

    /** Clicked Cancel or Close */
    void cancelButton_MouseClicked(MouseEvent event)
    {
        setVisible(false);
        dispose();
    }

    // Instance variables.
    private PanelWithTextFieldAndLabel     m_grammar_name_panel;
    private PanelWithTextFieldAndTwoLabels m_duration_panel;
    private PanelWithTextFieldAndTwoLabels m_grammar_load_panel;
    private JButton                        m_save_button;
    private JButton                        m_cancel_button;
    private JButton                        m_close_button;
}

