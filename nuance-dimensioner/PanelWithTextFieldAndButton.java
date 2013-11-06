/*
  File:        PanelWithTextFieldAndButton.java
  Author:      Sternberg
  Date:        January 23, 2001
  Description: Panel with a button over the textfield.
*/

import java.awt.*;
import javax.swing.*;

/**
Button appears over the textfield.
*/
public class PanelWithTextFieldAndButton 
extends JPanel
{
    public PanelWithTextFieldAndButton( String text, int text_width,
                                        String button_text )
    {
        m_text_field = new ScalableJTextField( text, text_width );
        m_button = new ScalableJButton( button_text );

        setLayout( new BorderLayout() );
        add( m_button, "North" );
        add( m_text_field, "South" );
    }

    /** Needed for setting button event handler. */
    public  JButton getButton()
    {
        return m_button;
    }

    /** text of the TextField */
    public String getText()
    {
        return m_text_field.getText();
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
    private JButton    m_button;
}
