/*
  File:     EditGrammarDialog.java
  Author:   Sternberg
  Date:     Feb 2001
  Description: Edit an element of the grammar model, in place.
*/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

/**
   This Dialog pops up when the user presses the "edit" button on the
   main frame.
*/
public class EditGrammarDialog extends JDialog
{
    public EditGrammarDialog(JFrame parent,
                             GrammarAndModifiers gm,
                             GrammarList grammar_list,
                             int selected_item)
    {
        super(parent, "Does this show?", false);
        m_grammar_list = grammar_list;
        m_selected_item = selected_item;

        // We're going to use gm_reference to modify the actual gm argument.
        // gm_reference is final, so it can be visible in those inner classes.
        gm_reference = gm;

        setTitle("Edit a Grammar");

        m_grammar_name_panel = new PanelWithTextFieldAndLabel(
          gm.m_grammar.name, 30,
          "Grammar name" );

        m_grammar_load_panel = new PanelWithTextFieldAndTwoLabels(
          String.valueOf(gm.m_grammar.load), 4,
          "Grammatical load",
          "LU" );

        m_duration_panel = new PanelWithTextFieldAndTwoLabels(
          String.valueOf(gm.m_grammar.duration), 4,
          "Average utterance duration in seconds",
          "" );

        m_requests_panel = new PanelWithTextFieldAndLabel(
          String.valueOf(gm.m_requests), 6,
          "Expected number of requests per call" );

        m_verification = new ScalableJCheckbox( "verification", gm.m_verify );

        m_apply_button = new ScalableJButton("Apply");
        m_close_button = new ScalableJButton("Close");

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
            southwest.setLayout( new GridLayout(0,1) );
            southwest.add( m_grammar_load_panel );
            southwest.add( m_duration_panel );
            southwest.add( m_requests_panel );
            southwest.add( m_verification );
            
            JPanel southeast = new JPanel();
            southeast.setLayout( new GridLayout(4,1,0,0) );
            southeast.add( new JPanel() );
            southeast.add( m_apply_button );
            southeast.add( m_close_button );
            
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

        m_apply_button.addMouseListener( new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                applyButton_MousePressed(event);
            }
        });

        m_close_button.addMouseListener( new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                closeButton_MousePressed(event);
            }
        });
    }

    /** Modify the GrammarAndModifiers right in the GrammarList.
     *  We don't care if the name is unique.
     */
    void applyButton_MousePressed(MouseEvent event)
    {
        String name = m_grammar_name_panel.getText();

        Double d = new Double(0.0);
        d=d.valueOf(m_grammar_load_panel.getText());
        double ld=d.doubleValue();
    
        d=d.valueOf(m_duration_panel.getText());
        double dur=d.doubleValue();

        gm_reference.m_grammar.name = name;
        gm_reference.m_grammar.load = ld;
        gm_reference.m_grammar.duration = dur;
        d = Double.valueOf(m_requests_panel.getText());
        gm_reference.m_requests = d.doubleValue();
        gm_reference.m_verify = m_verification.isSelected();

        // Refresh the display of this item of the grammar model, so as to
        // reflect changes to it made in the edit dialog.
        m_grammar_list.replaceItem( gm_reference, m_selected_item );
        m_grammar_list.select( m_selected_item );
    }

    /** Pressed Close */
    void closeButton_MousePressed(MouseEvent event)
    {
        setVisible(false);
        dispose();
    }

    // Instance variables.
    private PanelWithTextFieldAndLabel     m_grammar_name_panel;
    private PanelWithTextFieldAndTwoLabels m_duration_panel;
    private PanelWithTextFieldAndTwoLabels m_grammar_load_panel;
    private PanelWithTextFieldAndLabel     m_requests_panel;
    private ScalableJCheckbox              m_verification;

    private JButton                        m_apply_button;
    private JButton                        m_close_button;

    private GrammarList                    m_grammar_list;
    private int                            m_selected_item; // of m_grammar_list

    final private GrammarAndModifiers gm_reference;

}


