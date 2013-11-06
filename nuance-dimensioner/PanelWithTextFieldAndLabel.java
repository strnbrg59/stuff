/*
  File:        PanelWithTextFieldAndLabel.java
  Author:      Sternberg
  Date:
  Description: A Panel that has a Label with a TextField under it.
               See also the subclasses PanelWithTextFieldAndTwoLabels
               PanelWithTextFieldLabelAndButton.
*/

import java.awt.*;
import javax.swing.*;

/**
  A Panel that has a Label with a TextField under it.
*/
public class PanelWithTextFieldAndLabel extends JPanel
{
    public PanelWithTextFieldAndLabel( String text, int text_width,
                                       String label_caption )
    {
        m_text_field    = new ScalableJTextField( text, text_width );
        m_label         = new ScalableJLabel( label_caption );

        // Layout.  The purpose of m_south_panel is to enable
        // derived classes to place a button or second label
        // in the same line as the TextField.
        setLayout( new BorderLayout() );
        m_south_panel = new JPanel();
        m_south_panel.setLayout( new FlowLayout(FlowLayout.LEFT) );
        m_south_panel.add( m_text_field );
        add( m_south_panel, "South" );
        if( label_caption != "" )
        {
            add( m_label, "North" );
        }
    }

    /** text of the TextField */
    public String getText()
    {
        return m_text_field.getText();
    }

    /** The TextField itself */
    public JTextField getTextField()
    {
        return m_text_field;
    }

    /** text of the TextField */
    public void setText(String str)
    {
        m_text_field.setText(str);
    }

    /** text of the TextField */
    public void setText(double x)
    {
        StringBuffer sb = new StringBuffer(10);
        sb.append(x);
        String s = new String(sb);
        m_text_field.setText(s);
    }

    /** text of the TextField */
    public void setText(int x)
    {
        StringBuffer sb = new StringBuffer(10);
        sb.append(x);
        String s = new String(sb);
        m_text_field.setText(s);
    }
    
    /** refers to the TextField */
    public void setEditable( boolean b )
    {
        m_text_field.setEditable( b );
    }

    private JTextField m_text_field;
    private JLabel     m_label;
    JPanel             m_south_panel;
}



