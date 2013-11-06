/*
  File:        PanelWithTextFieldLabelAndButton.java
  Author:      Sternberg
  Date:
  Description: Subclass of PanelWithTextFieldAndLabel.  Second Label appears
               to the right of the TextField.
*/

import java.awt.*;
import javax.swing.*;

/**
  Subclass of PanelWithTextFieldAndLabel.  Second Label appears
  to the right of the TextField.
*/
public class PanelWithTextFieldLabelAndButton 
extends PanelWithTextFieldAndLabel
{
    public PanelWithTextFieldLabelAndButton( String text, int text_width,
                                             String label_caption, 
                                             String button_text )
    {
        super( text, text_width, label_caption );

        m_button = new ScalableJButton( button_text );
        m_south_panel.add( m_button );
    }

    /** Needed for setting button event handler. */
    public  JButton getButton()
    {
        return m_button;
    }
    private ScalableJButton m_button;
}
