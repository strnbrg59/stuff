/*
  File:        ScalableJButton.java
  Author:      Sternberg
  Date:        January 23, 2001
  Description: JButton that we control the size of from a single place
               in the code.
*/

import java.awt.*;
import javax.swing.*;

public class ScalableJButton
extends JButton
{
    public ScalableJButton( String text )
    {
        super( text );
        Font f = getFont().deriveFont( GlobalFontSetter.getSize() );
        setFont( f );
        setMargin( new Insets(0,0,0,0) );
    }
}
