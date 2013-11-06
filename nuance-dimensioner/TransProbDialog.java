/*
  File:     TransProbDialog.java
  Author:   Sternberg
  Date:     Feb 2001
  Description: Present a transition probability matrix, to help user
               compute requests-per-grammar.
*/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

/**
   This Dialog pops up when the user presses the "edit" button on the
   main frame.
*/
public class TransProbDialog extends JDialog
{
    public TransProbDialog(JFrame parent,
                           GrammarList grammar_list )
    {
        super(parent, "Does this show?", false);
        setTitle("Requests-per-grammar computation");
        getContentPane().setLayout( new BorderLayout() );

        //  data we'll need in calculations
        int n_grammars = grammar_list.getItemCount();
        m_grammar_list = grammar_list;
        if( m_previous_grammar_count !=  m_grammar_list.getItemCount() )
        {
            m_prob = new ScalableJTextField[n_grammars][n_grammars];
            m_request = new ScalableJTextField[ n_grammars ];
        }

        // Middle panel -- requests, grammar names, and prob matrix
        JPanel data_panel = new BorderedJPanel( "" );
        data_panel.setLayout( new BorderLayout() );
        JPanel requests_panel = new JPanel( new GridLayout( n_grammars, 1));
        JPanel grammar_name_panel = new JPanel( new GridLayout( n_grammars, 1 ));
        JPanel prob_matrix_panel = new JPanel( new GridLayout( n_grammars, n_grammars ));
        data_panel.add( requests_panel, "West" );
        data_panel.add( grammar_name_panel, "Center" );
        data_panel.add( prob_matrix_panel, "East" );

        JPanel buttons_panel = new JPanel();

        getContentPane().add( data_panel, "Center" );
        getContentPane().add( buttons_panel, "South" );

        // Initialize all the cells.
        for( int i=0;i<n_grammars;i++ )
        {
            if( m_previous_grammar_count != m_grammar_list.getItemCount() )
            {
                m_request[i] = new ScalableJTextField( "0.00", 3 );
                m_request[i].setEditable( false ); // Users can edit in the grammar model
                                              // panel.  Here, it'll confuse them about
                                              // the proper use of this dialog.
            }
            requests_panel.add( m_request[i] );

            JLabel gram_name = new ScalableJLabel( grammar_list.grammarAt(i).name );
            grammar_name_panel.add( gram_name );

            for( int j=0;j<n_grammars;j++ )
            {
                if( m_previous_grammar_count != m_grammar_list.getItemCount() )
                {
                    m_prob[i][j] = new ScalableJTextField("0.00", 3);
                }
                prob_matrix_panel.add( m_prob[i][j] );
            }
        }

        // Buttons: compute, apply, and close.
        m_compute_button = new ScalableJButton( "Compute" );
        m_apply_button = new ScalableJButton( "Apply" );
        m_close_button = new ScalableJButton( "Close" );
        buttons_panel.add( m_compute_button );
        buttons_panel.add( m_apply_button );
        buttons_panel.add( m_close_button );

        if( m_first_time == true )
        {
            m_hangup_panel = new PanelWithTextFieldAndTwoLabels( "0.00", 3, "",
                            "Probability of immediate hangup" );
        }
        getContentPane().add( m_hangup_panel, "North" );

        pack();

        registerListeners();            
        setVisible(true);
        m_previous_grammar_count = m_grammar_list.getItemCount();
        m_first_time = false;
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

        m_compute_button.addMouseListener( 
          new ComputeButtonMouseAdapter( m_request, m_prob, 
                                         m_hangup_panel.getTextField() )
        );

        m_apply_button.addMouseListener( 
          new ApplyButtonMouseAdapter( m_grammar_list, m_request, m_prob, 
                                       m_hangup_panel.getTextField(), this )
        );

        m_close_button.addMouseListener( new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                closeButton_MousePressed(event);
            }
        });


    }

    void closeButton_MousePressed(MouseEvent event)
    {
        setVisible(false);
        dispose();
    }

    //
    // Data
    //
    private ScalableJButton m_compute_button;
    private ScalableJButton m_apply_button;
    private ScalableJButton m_close_button;
    private static PanelWithTextFieldAndTwoLabels m_hangup_panel;
    private static JTextField m_prob[][];  // static, so they'll keep their
    private static JTextField m_request[]; // numbers across invocations.
    private GrammarList m_grammar_list;
    private static int m_previous_grammar_count = 0;
    private static boolean m_first_time = true;
}

class ApplyButtonMouseAdapter extends MouseAdapter
{
    ApplyButtonMouseAdapter( GrammarList grammar_list,
                             JTextField[] request,
                             JTextField[][] prob,
                             JTextField hangup_prob,
                             TransProbDialog dialog )
    {
        super();
        m_grammar_list = grammar_list;
        m_request = request;
        m_prob = prob;
        m_hangup_prob = hangup_prob;
        m_dialog = dialog;
    }

    /** Save current contents of the cells to their (static)
     *  TextString components.
     *  Copy the requests numbers to their counterparts in the
     *  GrammarList.
    */
    public void mousePressed(MouseEvent event) {
        if( m_request.length != m_grammar_list.getItemCount() )
        {   // Sign that the set of grammars has changed: bail out fast!
            m_dialog.setVisible(false);
            m_dialog.dispose();
            return;
        }            

        for( int i=0;i<m_request.length;i++ )
        {
            Double d = new Double(0.0);
            m_grammar_list.grammarAndModifiersAt(i).m_requests =
              d.valueOf(m_request[i].getText()).doubleValue();

            m_grammar_list.replaceItem(
              m_grammar_list.grammarAndModifiersAt(i), i );
        }

        m_grammar_list.setTransProbMatrix( m_prob );
    }

    GrammarList m_grammar_list;
    JTextField[] m_request;
    JTextField[][] m_prob;
    JTextField m_hangup_prob;
    TransProbDialog m_dialog;
}

class ComputeButtonMouseAdapter extends MouseAdapter
{
    ComputeButtonMouseAdapter( JTextField[] request,
                               JTextField[][] trans_prob,
                               JTextField hangup_prob )
    {
        super();
        m_request = request;
        m_trans_prob = trans_prob;
        m_hangup_prob = hangup_prob;
    }

    /** Save current contents of the cells to their (static)
     *  TextString components.
     *  Copy the requests numbers to their counterparts in the
     *  GrammarList.
    */
    public void mousePressed(MouseEvent event) {
        int n = m_request.length;
        double[][] p = new double[n][n];
        Double d = new Double(0.0);
        double d_hangup_prob = d.valueOf( m_hangup_prob.getText() ).doubleValue();

        for( int i=0;i<n;i++ ) 
        {
            double sum = 0;
            for( int j=0;j<n;j++ )
            {
                try
                {
                    p[i][j] = d.valueOf(m_trans_prob[i][j].getText()).doubleValue();
                }
                catch( NumberFormatException nfe )
                {
                    JOptionPane.showMessageDialog( 
                      ParentFrame.getParentFrame(m_request[0]),
                      "Item in cell (" + i + ", " + j + ") is not a number.",
                      "Number format error", JOptionPane.ERROR_MESSAGE );
                }
                if( p[i][j] < 0.0 )
                {
                    JOptionPane.showMessageDialog( 
                      ParentFrame.getParentFrame(m_request[0]),
                      "probability in cell (" + i + ", " + j + ") is negative.",
                      "Invalid probability", JOptionPane.ERROR_MESSAGE );
                }
                sum += p[i][j];
            }
            if( sum >= 1.0 )
            {
                JOptionPane.showMessageDialog( 
                  ParentFrame.getParentFrame(m_request[0]),
                  "Items in row " + i + " exceed 1.0.",
                  "Inconsistent probabilities", JOptionPane.ERROR_MESSAGE );
            }
        }

        

        CallModel cm = new CallModel();
        double[] c = cm.computeExpectedRequests( p );

        for( int i=0;i<n;i++ )
        {
            double x = Math.round( c[i] * (1-d_hangup_prob) * 100 )/100.0;
            m_request[i].setText( Double.toString(x) );
        }
    }

    JTextField[] m_request;
    JTextField[][] m_trans_prob;
    JTextField m_hangup_prob;
}
