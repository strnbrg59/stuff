/*
  File:        ScalableJCheckbox.java
  Author:      Sternberg
  Date:        January 23, 2001
  Description: We control the size of the font from a single place
               in the code.
*/

import java.awt.*;
import javax.swing.*;

public class ScalableJCheckbox
extends JCheckBox
{
    public ScalableJCheckbox( String text, boolean state )
    {
        super( text, state );
        Font f0 = getFont();
        Font f1;
        if( f0 != null )
        {
            f1 = f0.deriveFont( GlobalFontSetter.getSize() );
            setFont( f1 );
        }
    }

    public ScalableJCheckbox( String text )
    {
        this( text, false );
    }
}
