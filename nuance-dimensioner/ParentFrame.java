/*
  File:        ParentFrame.java
  Author:      Sternberg
  Date:
  Description: Static method for finding a Frame's parent.  Useful for
               constructing new Dialogs.
*/

import java.awt.*;
import javax.swing.*;

public class ParentFrame
{
    public static JFrame defaultJFrame = new JFrame();

    /**------ This version is from Matt's code */
    public static JFrame getParentFrame( Container child )
    {
        Container theFrame = child;
        do
        {
            theFrame = theFrame.getParent();
        } while ((theFrame != null) && !(theFrame instanceof JFrame));
        if( theFrame == null )
        {
//          theFrame = new JFrame();
            theFrame = defaultJFrame;
        }

        return (JFrame)theFrame;
    }


/*--- This version is from Ed Anuff's _Java Sourcebook_.
    public static JFrame getParentFrame( Container child )
    {
        Component currParent = child;
        JFrame theFrame = null;
        
        while( currParent != null )
        {
            if( currParent instanceof JFrame )
            {
                theFrame = (JFrame)currParent;
                break;
            }
            currParent = currParent.getParent();
        }

        if( theFrame == null )
        {
            theFrame = new JFrame();
        }

        return theFrame;
    }
*/
}


