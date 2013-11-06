/*
  File:        ScalableJLabel.java
  Author:      Sternberg
  Date:        January 23, 2001
  Description: We control the size of the font from a single place
               in the code.
*/

import java.awt.*;
import javax.swing.*;

public class ScalableJLabel
extends JLabel
{
    public ScalableJLabel( String text )
    {
        super( text );
        Font f = getFont().deriveFont( GlobalFontSetter.getSize() );
        setFont( f );
    }
}
