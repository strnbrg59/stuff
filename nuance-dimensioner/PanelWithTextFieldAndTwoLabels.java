/*
  File:        PanelWithTextFieldAndTwoLabels.java
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
public class PanelWithTextFieldAndTwoLabels extends PanelWithTextFieldAndLabel
{
    public PanelWithTextFieldAndTwoLabels( String text, int text_width,
                                           String label_caption1, 
                                           String label_caption2 )
    {
        super(text, text_width, label_caption1);

        m_label2 = new ScalableJLabel( label_caption2 );
        m_south_panel.add( m_label2 );
    }

    private ScalableJLabel m_label2; // other label is in superclass.
}
