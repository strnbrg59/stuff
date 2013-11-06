/*
  File:        BorderedJPanel.java
  Author:      Sternberg
  Date:        September 2000
  Description: A JPanel with a labeled border
*/

import java.awt.*;
import javax.swing.*;
import javax.swing.border.*;

/** See javax/swing/JPanel for how we implement the border in awt. */
public class BorderedJPanel extends JPanel
{
    public BorderedJPanel( String title )
    {
        TitledBorder tb = BorderFactory.createTitledBorder( title );
        Font f = tb.getTitleFont().deriveFont( GlobalFontSetter.getSize() );
        tb.setTitleFont( f );
        setBorder( tb );
    }

    public BorderedJPanel()
    {
        setBorder(BorderFactory.createLineBorder(Color.black));
    }
}
