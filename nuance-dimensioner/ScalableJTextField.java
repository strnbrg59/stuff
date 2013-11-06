/*
  File:        ScalableJTextField.java
  Author:      Sternberg
  Date:        January 23, 2001
  Description: We control the size of the font from a single place
               in the code.
*/

import java.awt.*;
import javax.swing.*;

public class ScalableJTextField
extends JTextField
{
    public ScalableJTextField( String text, int text_width )
    {
        super( text, text_width );
        Font f = getFont().deriveFont( GlobalFontSetter.getSize() );
        setFont( f );
    }
}
