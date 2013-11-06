/*
  File:        FrontEnd.java
  Author:      Sternberg
  Date:        April 1999
  Description: The GUI.
*/

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import javax.swing.*;
import javax.swing.border.*;

/** This is the GUI.  It holds a reference to an instance of BackEnd, which
 *  is where the calculations take place.
 */
public class FrontEnd extends JPanel
{
    /** @arg backend An interface.
    */
    public FrontEnd( BackEnd backend )
    {
        m_backend = backend;
        m_traffic_params = new TrafficParams();

        // Initialize all the panels and other components that go into this
        // Frame.
        addComponents();
        updateFromTextFields();
        bhcvButton_MousePressed(null); // obtain mutually consistent state
        updateTextFields();
        
        // Long "Choice" (drop-down list) of grammars.
        refreshGrammarChoice();

        // Register Listener objects with the components that need them.
        registerListeners();
    }
    
    private void refreshGrammarChoice()
    {
        m_grammar_choice.removeAll();
        for (int i=0;i<GrammarDatabase.size();i++)
            m_grammar_choice.addItem(GrammarDatabase.elementAt(i));
    }
    
    /** Set data members to what's in their corresponding TextFields. */
    private void updateFromTextFields()
    {
        m_traffic_params.set_bhcv( Integer.parseInt( m_bhcv_panel.getText() ));
            
        Double d = new Double(0.0);

        m_traffic_params.set_holding_time(
          Integer.parseInt( m_holding_time_panel.getText() ));
        
        d=d.valueOf( m_block_prob_panel.getText() );
        m_traffic_params.set_block_prob( d.doubleValue() );

        m_traffic_params.set_speech_channels( 
          Integer.parseInt(m_speech_channels_panel.getText()));
/*
        d=d.valueOf( m_avg_requests_per_call_panel.getText() );
        m_traffic_params.set_requests_per_call( d.doubleValue() );
*/
        d=d.valueOf( m_latency_threshold_panel.getText() );     
        m_traffic_params.set_latency_threshold( d.doubleValue() );
        
        d=d.valueOf( m_latency_prob_panel.getText() );
        m_traffic_params.set_latency_prob( d.doubleValue() );

        d=d.valueOf( m_ru_per_server_panel.getText() );
        m_traffic_params.set_ru_per_server( d.doubleValue() );

        m_traffic_params.set_twopass( m_twopass_checkbox.isSelected() );
    }
    
    /** Make the TextFields show the current values of the data members
      * which correspond to them.  This method is called at the end of
      * event handlers.
    */
    private void updateTextFields()
    {
        TrafficParams tp = m_traffic_params; // To relieve eye-strain ;-)

        m_bhcv_panel.setText(                  tp.get_bhcv() );
        m_holding_time_panel.setText(          tp.get_holding_time());
        m_block_prob_panel.setText(            tp.get_block_prob() );
        m_carried_calls_panel.setText(         tp.get_carried_calls());
        m_offered_traffic_panel.setText(       tp.get_offered_traffic());
        m_speech_channels_panel.setText(       tp.get_speech_channels());
//      m_avg_requests_per_call_panel.setText( tp.get_requests_per_call() );
        m_latency_threshold_panel.setText(     tp.get_latency_threshold() );
        m_latency_prob_panel.setText(          tp.get_latency_prob() );
        m_ru_required_panel.setText(           tp.get_required_ru() );
        m_ru_required_per_cpu_panel.setText(   tp.get_min_ru_per_cpu() );
        m_ru_per_server_panel.setText(         tp.get_ru_per_server() );
        m_memory_required_panel.setText(Math.round( tp.get_memory()*100 )/100.0);
        m_twopass_checkbox.setSelected(        tp.get_twopass() );
    }

    /** Find busy hour call attempts. */
    private void bhcvButton_MousePressed(MouseEvent event)
    {
        updateFromTextFields();
    
        try
        {
            m_traffic_params.set_bhcv( m_backend.solveForBHCV(
                new TrafficParams(m_traffic_params)));
            m_traffic_params.set_carried_calls( m_backend.solveForCarriedCalls(
                new TrafficParams(m_traffic_params)));
            m_traffic_params.set_offered_traffic( m_backend.solveForOfferedTraffic(
                new TrafficParams(m_traffic_params)));
        
            updateTextFields();
        }
        catch( BackEndException bee )
        {
            JOptionPane.showMessageDialog( 
              ParentFrame.getParentFrame(this),
              bee.toString(),
              "input error", JOptionPane.ERROR_MESSAGE );
        }
    }
    
    /** Find probability of blocking. */
    private void blockProbButton_MousePressed(MouseEvent event)
    {
        updateFromTextFields();

        try
        {
            m_traffic_params.set_block_prob( m_backend.solveForBlockProb(
                new TrafficParams(m_traffic_params)));
            m_traffic_params.set_carried_calls( m_backend.solveForCarriedCalls(
                new TrafficParams(m_traffic_params)));
            m_traffic_params.set_offered_traffic( 
              m_backend.solveForOfferedTraffic(
                new TrafficParams(m_traffic_params)));
        }
        catch( BackEndException bee )
        {
            JOptionPane.showMessageDialog( 
                ParentFrame.getParentFrame(this),
                bee.toString(),
                "input error", JOptionPane.ERROR_MESSAGE );
        }

        updateTextFields();
    }

    /** Find speech channels. */
    private void speechChannelsButton_MousePressed(MouseEvent event)
    {
        updateFromTextFields();
        
        try
        {
            m_traffic_params.set_speech_channels( 
              m_backend.solveForSpeechChannels(
                new TrafficParams(m_traffic_params)));
            m_traffic_params.set_carried_calls( m_backend.solveForCarriedCalls(
                new TrafficParams(m_traffic_params)));
            m_traffic_params.set_offered_traffic( 
              m_backend.solveForOfferedTraffic(
                new TrafficParams(m_traffic_params)));
        }
        catch( BackEndException bee )
        {
            JOptionPane.showMessageDialog( 
                ParentFrame.getParentFrame(this),
                bee.toString(),
                "input error", JOptionPane.ERROR_MESSAGE );
        }
        
        updateTextFields();
    }

    /** Add selected grammar to call model (and display it in the List). */
    private void addGrammarButton_MousePressed(MouseEvent event)
    {
        updateFromTextFields();
        m_grammar_list.addElement( m_grammar_choice.getSelectedGrammar(),
//                                 m_traffic_params.get_requests_per_call(),
                                   0,
//                                 m_verify_checkbox.isSelected());
                                   false );
    }

    /** Delete call element from call model. */
    private void removeGrammarButton_MousePressed(MouseEvent event)
    {
        int selected = m_grammar_list.getSelectedIndex();
        if( selected == -1 )
        {
            JOptionPane.showMessageDialog( 
              ParentFrame.getParentFrame(this),
              "Please click on the grammar you wish to delete",
              "no-grammar-selected error", 
              JOptionPane.ERROR_MESSAGE );
        }
        else
        {
            m_grammar_list.delItem( selected );

            m_edit_grammar_button.setEnabled( false );
            m_clone_grammar_button.setEnabled( false );
            m_delete_grammar_button.setEnabled( false );
        }
    }

    /** Clear all the items from the call model. */
    private void clearGrammarModelButton_MousePressed(MouseEvent event)
    {
        m_grammar_list.clearAll();

        m_edit_grammar_button.setEnabled( false );
        m_clone_grammar_button.setEnabled( false );
        m_delete_grammar_button.setEnabled( false );
    }

    /** Compute recognition power required. */
    private void findRecognitionPowerButton_MousePressed( MouseEvent event)
    {
        updateFromTextFields();

        try
        {
            m_traffic_params.set_required_ru(m_backend.solveForRecognitionPower(
                new TrafficParams(m_traffic_params),
                m_grammar_list));

            m_traffic_params.set_min_ru_per_cpu( 
                m_backend.solveForMinRUPerCPU(m_grammar_list));
        }
        catch( BackEndException bee )
        {
            JOptionPane.showMessageDialog( 
              ParentFrame.getParentFrame(this),  
              bee.toString(),
              "input error", JOptionPane.ERROR_MESSAGE );
        }

        updateTextFields();
    }

    /** Compute memory requirements */
    private void findMemoryRequiredButton_MousePressed( MouseEvent event)
    {
        updateFromTextFields();

        try
        {
            m_traffic_params.set_required_ru(m_backend.solveForRecognitionPower(
                new TrafficParams(m_traffic_params),
                m_grammar_list));

            m_traffic_params.set_min_ru_per_cpu( 
                m_backend.solveForMinRUPerCPU(m_grammar_list));

            m_traffic_params.set_memory(
                m_backend.solveForMemory( m_traffic_params, m_grammar_list ));
        }
        catch( BackEndException bee )
        {
            JOptionPane.showMessageDialog( 
              ParentFrame.getParentFrame(this),  
              bee.toString(),
              "input error", JOptionPane.ERROR_MESSAGE );
        }

        updateTextFields();
    }
    

    /** Clone a grammar, and let user modify its name and properties. 
     */
    private void cloneButton_MousePressed(MouseEvent event)
    {
        // Find the GrammarDatabase element that matches the selected Grammar.

        int selected = m_grammar_list.getSelectedIndex();
        if( selected == -1 )
        {
            JOptionPane.showMessageDialog( 
              ParentFrame.getParentFrame(this),  
              "Please click on the grammar you wish to clone",
              "no-grammar-selected error", 
              JOptionPane.ERROR_MESSAGE );
            return;
        }

        int iGrammar = m_grammar_list.getSelectedIndex();
        Grammar currGrammar = m_grammar_list.grammarAt(iGrammar);
        double load = currGrammar.load;
        double dur  = currGrammar.duration;
        String name = currGrammar.name;
        
        // Put up the Dialog that enables the user to define a new grammar
        // using the cloned one as a starting point.
        {
            CloneGrammarDialog dialog = new CloneGrammarDialog( 
                ParentFrame.getParentFrame(this),  
                name, load, dur );
        }
        refreshGrammarChoice();
    }

    /* Pop up a transition probability matrix. */
    private void transprobButton_MousePressed(MouseEvent event)
    {
        if( m_grammar_list.getItemCount() == 0 )
        {
            JOptionPane.showMessageDialog( 
              ParentFrame.getParentFrame(this),  
              "Grammar model is empty, nothing to compute",
              "empty grammar model error", 
              JOptionPane.ERROR_MESSAGE );
            return;
        }

        TransProbDialog dialog = 
          new TransProbDialog( ParentFrame.getParentFrame(this), m_grammar_list );
    }

    /** Edit a grammar, letting user modify its name and properties. 
     */
    private void editButton_MousePressed(MouseEvent event)
    {
        // Find the GrammarDatabase element that matches the selected Grammar.

        int selected = m_grammar_list.getSelectedIndex();
        if( selected == -1 )
        {
            JOptionPane.showMessageDialog( 
              ParentFrame.getParentFrame(this),  
              "Please click on the grammar you wish to edit",
              "no-grammar-selected error", 
              JOptionPane.ERROR_MESSAGE );
            return;
        }

        // Put up the Dialog that enables the user to modify this entry of the
        // grammar model.
        {
            GrammarAndModifiers gm = m_grammar_list.grammarAndModifiersAt( selected );
            EditGrammarDialog dialog = new EditGrammarDialog( 
                ParentFrame.getParentFrame(this), gm, m_grammar_list, selected );
            m_grammar_list.select( selected );
        }
    }

    /** Print a summary of the text fields to a file.  Triggered by the "Save as"
     *  menu option.
    */
    private void printReport()
    {
        // Get the name of the file you'll save the report to.
        // FIXME: replace all awt calls with swing calls.
        FileDialog fileDialog = new FileDialog(
            ParentFrame.getParentFrame(this), 
            "Save state", FileDialog.SAVE);
        fileDialog.setVisible(true);
        String outfilename = fileDialog.getFile();
        if( (outfilename == null) || (outfilename.equals("")) ) return;
        // FIXME: we should handle invalid input more elegantly.  
        // But first, I have to learn what Event gets generated when the user 
        // presses the various buttons on the FileDialog.

        try
        {
            // Open the file.
            PrintWriter writer = new PrintWriter(
              new FileOutputStream( outfilename, false ), true);
            writer.println( this.toString() );
        }
        catch( IOException ioe )
        {
            System.err.println( ioe );
            ioe.printStackTrace();
        }
    }

    static final String DIM_SUMMARY = "======== Dimensioner Summary =========";
    static final String DASHES      = "--------------------------------------";
    static final String CALL_MODEL_HEADER =
                                      "Call Model (req|verif|name|LU|duration)";

    public String toString()
    {
        String result = DIM_SUMMARY + "\n"
                      + m_traffic_params + "\n"
                      + DASHES + "\n"
                      + CALL_MODEL_HEADER + "\n"
                      + m_grammar_list + "\n"
                      + DASHES;
        return result;
    }

    /** Deserialize.
     *  Note format used by toString().
     */
    private void fromString( String serialized )
    {
        StringTokenizer st = new StringTokenizer( serialized, "\n" );
        
        // Discard "========== Dimensioner Summary ========="
        st.nextToken();

        // Pick up traffic params part.
        String params_str = "";
        String one_line = st.nextToken();
        while( ! one_line.equals( DASHES ) )
        {
            params_str += "\n" + one_line;
            one_line = st.nextToken();
        }
        try
        {
            m_traffic_params.fromString( params_str );  // deserializes it
        }
        catch( BackEndException bee )
        {
            JOptionPane.showMessageDialog( 
              ParentFrame.getParentFrame(this),
              bee.toString(),
              "Format error in input file", JOptionPane.ERROR_MESSAGE );
        }
        updateTextFields();  // to reflect what's now in m_traffic_params.

        // Discard "Call Model (req|verif|name|LU|duration)"
        st.nextToken();

        // Pick up call model part.
        one_line = st.nextToken();
        m_grammar_list.clearAll();
        while( ! one_line.equals( DASHES ) )
        {
            try
            {
                m_grammar_list.addElement( one_line );
            }
            catch( BackEndException bee )
            {
                JOptionPane.showMessageDialog( 
                  ParentFrame.getParentFrame(this), bee.toString(), "data error",
                  JOptionPane.ERROR_MESSAGE );
            }
            one_line = st.nextToken();
        }
    }

    /** Facility for undoing operations.  Each undo returns us to the last
     *  place a checkpoint was set.
    */
    private static Stack s_undo_stack = new Stack();
    
    boolean undoStackEmpty()
    {
        if( s_undo_stack.size() == 0 )
        {
            return true;
        } else
        {
            return false;
        }
    }        

    private void setCheckpoint() 
    { 
        FrontEnd.s_undo_stack.push( toString() ); 
        JOptionPane.showMessageDialog( 
          ParentFrame.getParentFrame(this),
          "Checkpoint has been set: next undo returns to here.",
          "checkpoint set", JOptionPane.INFORMATION_MESSAGE );
    }
    private void undo()
    {
        if( s_undo_stack.size() > 0 )
        {
            String state = (String)s_undo_stack.pop();
            fromString( state );

            m_edit_grammar_button.setEnabled( false );
            m_clone_grammar_button.setEnabled( false );
            m_delete_grammar_button.setEnabled( false );
        } else
        {
            JOptionPane.showMessageDialog( 
              ParentFrame.getParentFrame(this),
              "Undo stack is empty",
              "empty undo stack", JOptionPane.INFORMATION_MESSAGE );
        }
    }

    /** These two are callbacks in the Dimensioner class. */
    void menubarSetCheckpoint()
    {
        setCheckpoint();
    }
    void menubarUndo()
    {
        undo();
    }

    /** Called only from FrontEnd constructor. */
    private void registerListeners()
    {
        m_bhcv_panel.getButton().addMouseListener( new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                bhcvButton_MousePressed(event);
            }
        });

        m_block_prob_panel.getButton().addMouseListener( new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                blockProbButton_MousePressed(event);
            }
        });

        m_speech_channels_panel.getButton().addMouseListener(new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                speechChannelsButton_MousePressed(event);
            }
        });

        m_add_grammar_button.addMouseListener( new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                addGrammarButton_MousePressed(event);
            }
        });

        m_delete_grammar_button.addMouseListener( new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                removeGrammarButton_MousePressed(event);
            }
        });

        m_clear_grammar_model_button.addMouseListener( new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                clearGrammarModelButton_MousePressed(event);
            }
        });

        m_find_recognition_power_button.addMouseListener( new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                findRecognitionPowerButton_MousePressed(event);
            }
        });


        m_find_memory_required_button.addMouseListener( new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                findMemoryRequiredButton_MousePressed(event);
            }
        });

        m_clone_grammar_button.addMouseListener( new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                cloneButton_MousePressed(event);
            }
        });

        m_edit_grammar_button.addMouseListener( new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                editButton_MousePressed(event);
            }
        });

        m_transprob_button.addMouseListener( new MouseAdapter(){
            public void mousePressed(MouseEvent event) {
                transprobButton_MousePressed(event);
            }
        });
    }

    /** Called only from FrontEnd constructor. 
        We add the top-level components (provisioning_panel, call_model_panel,
        etc.) at the very end of this method.  That's where we also set
        the FrontEnd Frame's layout.
     */
    private void addComponents()
    {
        // ----------------------------------------------------
        // provisioning_panel
        //
        BorderedJPanel provisioning_panel = 
          new BorderedJPanel("Speech Channel Provisioning");
        provisioning_panel.setLayout(new GridLayout(3,2,30,10));

        m_bhcv_panel =
            new PanelWithTextFieldAndButton( "3600", 7, 
                                             "Busy hour call attempts" );
        m_holding_time_panel =
            new PanelWithTextFieldAndTwoLabels( "120", 7,
                                                "Call holding time",
                                                "sec" );
        m_block_prob_panel =
            new PanelWithTextFieldAndButton( "0.02", 7,
                                             "Probability of blocking" );
        m_carried_calls_panel =
            new PanelWithTextFieldAndLabel( "0", 7,
                                            "Carried calls" );
        m_carried_calls_panel.setEditable( false );

        m_offered_traffic_panel =
            new PanelWithTextFieldAndTwoLabels( "0.0", 7,
                                                "Offered traffic",
                                                "Erlang" );
        m_offered_traffic_panel.setEditable( false );

        m_speech_channels_panel =
            new PanelWithTextFieldAndButton( "133", 7,
                                             "Speech channels" );

        provisioning_panel.add(m_bhcv_panel);
        provisioning_panel.add(m_carried_calls_panel);
        provisioning_panel.add(m_holding_time_panel);
        provisioning_panel.add(m_offered_traffic_panel);
        provisioning_panel.add(m_block_prob_panel);
        provisioning_panel.add(m_speech_channels_panel);
        // ----------------------------------------------------

        // ----------------------------------------------------
        // call_model_panel
        // West subpanel: grammar menu and text area.  
        // East subpanel: requests/call, add grammar, edit and clone.
        //
        BorderedJPanel call_model_panel = 
            new BorderedJPanel("Grammar Usage per Call");
        call_model_panel.setLayout( new BorderLayout(5,5) );

        {   // scope for sub-panels of call_model_panel.

            // West
            JPanel west_panel = new JPanel();
            west_panel.setLayout( new BorderLayout() );

            JPanel northwest_panel = new JPanel();
            northwest_panel.setLayout( new BorderLayout() );
            ScalableJLabel select_grammar_label = new ScalableJLabel("Select grammar");
            m_grammar_choice = new GrammarChoice();
            northwest_panel.add( select_grammar_label, "North" );
            northwest_panel.add( m_grammar_choice, "South" );

            JPanel southwest_panel = new JPanel();
            southwest_panel.setLayout( new BorderLayout() );
            ScalableJLabel select_item_label = 
                new ScalableJLabel("Grammar Model (click to select item)");
            m_grammar_list = new GrammarList(12);
            m_grammar_list.addItemListener( new ItemListener() {
                public void itemStateChanged( ItemEvent e )
                {
                    if( e.getStateChange() == e.SELECTED )
                    {
                        m_edit_grammar_button.setEnabled( true );
                        m_clone_grammar_button.setEnabled( true );
                        m_delete_grammar_button.setEnabled( true );
                    } else
                    if( e.getStateChange() == e.DESELECTED )
                    {
                        m_edit_grammar_button.setEnabled( true );
                        m_clone_grammar_button.setEnabled( true );
                        m_delete_grammar_button.setEnabled( true );
                    }
                }
            });
            //m_grammar_list.addElement( 
            //  GrammarDatabase.elementAt(12), 1.0, false );
            
            southwest_panel.add( select_item_label, "North" );
            southwest_panel.add( m_grammar_list, "South" );

            west_panel.add( northwest_panel, "North" );
            west_panel.add( southwest_panel, "South" );

            // East -- the buttons
            JPanel east_panel = new JPanel();
            east_panel.setLayout( new BorderLayout() );

/*
            m_avg_requests_per_call_panel = 
                new PanelWithTextFieldAndLabel( "0.0", 7,
                                                "Avg requests/call");
*/

            m_add_grammar_button = new ScalableJButton( "Add Grammar" );

            // Here are the grammarlist control buttons:
            m_edit_grammar_button.setEnabled( false );
            m_clone_grammar_button.setEnabled( false );
            m_delete_grammar_button.setEnabled( false );
            m_clear_grammar_model_button = new ScalableJButton( "Clear" );
    
            {
                JPanel nnePanel = new JPanel();
                nnePanel.setLayout( new BorderLayout());
/*
                nnePanel.add( m_avg_requests_per_call_panel, "North" );
                nnePanel.add( m_verify_checkbox, "Center" );
*/
                nnePanel.add( m_add_grammar_button, "South" );
                JPanel nePanel = new JPanel();
                nePanel.setLayout( new BorderLayout() );
                nePanel.add( nnePanel );
                east_panel.add( nePanel, "North" );

                east_panel.add( grammarListControlButtons(), "South" );
            }

            call_model_panel.add( west_panel, "West" );
            call_model_panel.add( east_panel, "East" );
        }
        // ----------------------------------------------------

        // ----------------------------------------------------
        // ru_panel
        //
        BorderedJPanel ru_panel = 
          new BorderedJPanel("Recognition Power Requirements");
        ru_panel.setLayout( new GridLayout(0,1) );

        JPanel rl_panel = new JPanel();
        rl_panel.setLayout( new BorderLayout() );

        m_latency_threshold_panel = new PanelWithTextFieldAndTwoLabels(
            "2.0", 8,
            "Latency threshold, T      ",
            "sec" );
        m_latency_prob_panel = new PanelWithTextFieldAndTwoLabels( 
            "95.0", 8, 
            "Prob(Latency < T)", 
            "%" );

        rl_panel.add( m_latency_threshold_panel, "West" );
        rl_panel.add( m_latency_prob_panel );

        m_ru_required_panel = new PanelWithTextFieldAndTwoLabels(
          "0", 6,
          "",
          "Total RU" );
        m_ru_required_panel.setEditable( false );
        
        m_ru_required_per_cpu_panel = new PanelWithTextFieldAndTwoLabels(
          "0.0", 6,
          "",
          "Min RU per CPU" );
        m_ru_required_per_cpu_panel.setEditable( false );

        m_find_recognition_power_button = 
            new ScalableJButton( "  Find RU requirements  " );
        JPanel ru_button_panel = new JPanel();  // To make the button smaller
        ru_button_panel.add( m_find_recognition_power_button );

        ru_panel.add( rl_panel );
        ru_panel.add( m_ru_required_panel );
        ru_panel.add( m_ru_required_per_cpu_panel );
        ru_panel.add( ru_button_panel );
        // ----------------------------------------------------

        // ----------------------------------------------------
        // Memory usage panel
        //
        BorderedJPanel memory_panel = new BorderedJPanel("Memory requirements");
        memory_panel.setLayout( new GridLayout(0,1) );

        m_twopass_checkbox = 
            new ScalableJCheckbox("two-pass model");

        m_ru_per_server_panel = new PanelWithTextFieldAndTwoLabels(
          "20.0", 6,
          "",
          "RU per recserver" );
        m_ru_per_server_panel.setEditable( true );

        m_memory_required_panel = new PanelWithTextFieldAndTwoLabels(
          "0.0", 6,
          "",
          "Megabytes per recserver" );
        m_memory_required_panel.setEditable( false );

        m_find_memory_required_button = 
          new ScalableJButton( "  Find memory requirements  " );
        JPanel mem_button_panel = new JPanel(); // to make the button smaller
        mem_button_panel.add( m_find_memory_required_button );
        
        memory_panel.add( m_twopass_checkbox );
        memory_panel.add( m_ru_per_server_panel );
        memory_panel.add( m_memory_required_panel );
        memory_panel.add( mem_button_panel );        

        // ----------------------------------------------------
        // Add the top-level components
        //
        setLayout( new GridBagLayout() );
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.fill = GridBagConstraints.NONE;
        gbc.insets = new Insets(10,10,10,10);
        gbc.weightx = gbc.weighty = 1.0;

        gbc.gridx=0; gbc.gridy=0; gbc.gridwidth=4; gbc.gridheight=3;
        add(provisioning_panel, gbc);

        gbc.gridx=0; gbc.gridy=gbc.RELATIVE; gbc.gridwidth=5; gbc.gridheight=4;
        add(call_model_panel, gbc);

        gbc.gridx=5; gbc.gridy=gbc.RELATIVE; gbc.gridwidth=4; gbc.gridheight=4;
        add(ru_panel, gbc);

        gbc.gridx=5; gbc.gridy=gbc.RELATIVE; gbc.gridwidth=3; gbc.gridheight=2;
        add(memory_panel, gbc);
    }

    /** Produce the clone-, add-, delete-, clear-grammar  buttons.
    */
    JPanel grammarListControlButtons()
    {
        JPanel result = new JPanel();
        result.setLayout( new GridLayout(8,1,5,5));
        // We have 6, not 8, buttons, but 8 places them more nicely if we start
        // with a few empty panels.
        result.add( new JPanel() ); 
        result.add( new JPanel() ); 

        result.add( m_transprob_button );
        result.add( m_edit_grammar_button );
        result.add( m_clone_grammar_button );
        result.add( m_delete_grammar_button );
        result.add( m_clear_grammar_model_button );

        return result;
    }

    /** Implements menubar's "load" option. */
    void menubarLoad()
    {
        FileDialog fileDialog = new FileDialog( 
          ParentFrame.getParentFrame(this),  
          "load state", FileDialog.LOAD);
        fileDialog.show();
        String infilename = fileDialog.getFile();
        StringBuffer sb = new StringBuffer();

        if( infilename != null )
        {
            try
            {
                BufferedReader reader = new BufferedReader(
                  new FileReader( infilename ));

                String all_lines = null;
                String one_line = null;
                while ( null != (one_line = reader.readLine()) )
                {
                    all_lines += one_line + "\n";
                }

                fromString( all_lines );

                m_edit_grammar_button.setEnabled( false );
                m_clone_grammar_button.setEnabled( false );
                m_delete_grammar_button.setEnabled( false );
            }
            catch (FileNotFoundException e )
            {
                JOptionPane.showMessageDialog( 
                  ParentFrame.getParentFrame(this),
                  "Invalid file name, or file not readable",
                  "input error", JOptionPane.ERROR_MESSAGE );
            }
            catch (IOException ioe )
            {
                JOptionPane.showMessageDialog( 
                  ParentFrame.getParentFrame(this),                   
                  "IO Exception.  Bad data?  Check your file.",
                  "io error", JOptionPane.ERROR_MESSAGE );
            }                
        }
    }

    /** Implements menubar's "save as" option. */
    void menubarSaveAs()
    {
        printReport();
    }


    // -------------------------------------------------------
    // data members
    //
    private BackEnd                            m_backend;

    private PanelWithTextFieldAndButton        m_bhcv_panel;
    private PanelWithTextFieldAndTwoLabels     m_holding_time_panel;
    private PanelWithTextFieldAndButton        m_block_prob_panel;
    private PanelWithTextFieldAndLabel         m_carried_calls_panel;
    private PanelWithTextFieldAndTwoLabels     m_offered_traffic_panel;
    private PanelWithTextFieldAndButton        m_speech_channels_panel;

    private GrammarChoice                      m_grammar_choice;
    protected GrammarList                      m_grammar_list;
      // Not private because it's used in FrontEndFor* derived classes.
/*
    private PanelWithTextFieldAndLabel         m_avg_requests_per_call_panel;
    private   JCheckBox                        m_verify_checkbox;
*/
    protected JButton                          m_add_grammar_button;
    final protected JButton m_transprob_button =
      new ScalableJButton( "Requests per grammar" );
    final protected JButton m_edit_grammar_button = new ScalableJButton( "Edit Grammar" );
    final protected JButton m_clone_grammar_button = new ScalableJButton( "Clone Grammar" );
    final protected JButton m_delete_grammar_button = 
      new ScalableJButton( "Delete Grammar" );
    protected JButton                          m_clear_grammar_model_button;

    private PanelWithTextFieldAndTwoLabels     m_latency_threshold_panel;
    private PanelWithTextFieldAndTwoLabels     m_latency_prob_panel;

    private PanelWithTextFieldAndTwoLabels     m_ru_required_panel;
    private PanelWithTextFieldAndTwoLabels     m_ru_required_per_cpu_panel;
    private PanelWithTextFieldAndTwoLabels     m_memory_required_panel;
    private PanelWithTextFieldAndTwoLabels     m_ru_per_server_panel;
    private JButton                            m_find_recognition_power_button;
    private JButton                            m_find_memory_required_button;

    private JCheckBox                          m_twopass_checkbox;
    private JButton                            m_saveinfo_button;

    private TrafficParams                      m_traffic_params; 
    // -------------------------------------------------------    
}


class Dbg
{
    static public void println(String str)
    {
        System.out.println(str);
    }
}

