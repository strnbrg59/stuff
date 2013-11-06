/*
  File:        Dimensioner.java
  Author:      Sternberg
  Date:        April 1999
  Description: Entry point into Dimensioner program.  
*/

import java.awt.*;
import java.awt.event.*;
import java.math.*;
import java.net.*;
import javax.swing.*;

/** Entry point into dimensioner Java program.
*/
public class Dimensioner
{
    /** For running standalone. */
    public static void main( String[] args )
    {
        BackEnd back_end = new AnalyticalBackEnd();
/* This stuff is for the public-key-encryption licensing scheme, now abandoned.
        PKC pkc = new PKC();
        String licensee = pkc.decryptFromFile( "license.dat",
              new BigInteger("d12afb16c43ea524b190dbc792cedcb07e2954a2f0c0fd5a277c7d7e186ab95592d46cae3bb1648f421e2478c3c15cf9a0f73a983191f27a347b67aa1bf6ed39",16),
              new BigInteger("010001",16));
*/

        FrontEnd frontend = new FrontEnd( back_end );
        CloseableFrame cf = new CloseableFrame( frontend );
//      cf.setTitle( "Nuance Dimensioner, licensed to " + licensee );
        cf.setTitle( "Nuance Dimensioner, version 4.20" );

        LicenseDialog license_dialog = new LicenseDialog( cf );

        cf.pack();
        cf.setVisible(true);
    }
}

/** A Frame that responds to the window manager's close-window control. 
 *  The ctor takes a FrontEnd because we want to do things
 *  from the menubar that require access to what's displayed in the
 *  frontend JPanel.
*/
class CloseableFrame extends JFrame implements WindowListener 
{
    CloseableFrame( FrontEnd frontend ) 
    { 
        super();

        JMenuBar menubar = createMenu( frontend );
        setJMenuBar( menubar );

        getContentPane().add( frontend );
        this.addWindowListener(this); 
    }
    CloseableFrame(String title) 
    {
        super(title);
        this.addWindowListener(this);
    }

    // methods of the WindowListener object.
    public void windowClosing(WindowEvent e) 
    {
        this.dispose(); 
        System.exit(0);
    }
    public void windowOpened(WindowEvent e) {}
    public void windowClosed(WindowEvent e) {}
    public void windowIconified(WindowEvent e){}
    public void windowDeiconified(WindowEvent e) {}
    public void windowActivated(WindowEvent e) {}
    public void windowDeactivated(WindowEvent e) {}

    //
    // Menu stuff
    //
    /** Gets called automatically by the JFrame ctor. */
    protected void frameInit()
    {
        super.frameInit();
    }

    /** Create the menu bar.
     *  The FrontEnd arg is so we can call methods on it to implement
     *  various menubar options like save and open; these require knowledge
     *  of what's displayed in the frontend.
     */
    protected JMenuBar createMenu( FrontEnd frontend )
    {
        final FrontEnd m_frontend = frontend;
        JMenuBar menubar = new JMenuBar();

        //
        // File menu
        //

        JMenu file_menu = new JMenu( "File" );

        JMenuItem save_item = new JMenuItem( "Save as..." );
        save_item.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                m_frontend.menubarSaveAs();
            }
        });

        JMenuItem load_item = new JMenuItem( "Load" );
        load_item.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                m_frontend.menubarLoad();
            }
        });

        final JMenuItem undo_item = new JMenuItem( "Undo" );
        undo_item.setEnabled( false );

        undo_item.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                m_frontend.menubarUndo();

                if( m_frontend.undoStackEmpty() )
                {
                    undo_item.setEnabled( false );
                }
            }
        });

        final JMenuItem checkpoint_item = new JMenuItem( "Set checkpoint" );
        checkpoint_item.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                m_frontend.menubarSetCheckpoint();
                undo_item.setEnabled( true );
            }
        });

        JMenuItem exit_item = new JMenuItem( "Exit" );
        exit_item.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                System.exit(0);
            }
        });
        file_menu.add( save_item );
        file_menu.add( load_item );
        file_menu.addSeparator();
        file_menu.add( checkpoint_item ); 
        file_menu.add( undo_item ); 
        file_menu.addSeparator();
        file_menu.add( exit_item );

        //
        // Options menu
        //

        JMenu options_menu = new JMenu( "Options" );
        JMenuItem preferences_item = new JMenuItem( "Preferences" );
        /*
        preferences_item.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                PreferencesDialog dialog = new PreferencesDialog();
            }
        });
        */
        options_menu.add( preferences_item );

        //
        // Info menu (don't add until you can say something more interesting in it).
        //
        JMenu info_menu = new JMenu( "Info" );
        JMenuItem about_item = new JMenuItem( "About" );
        about_item.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                JOptionPane.showMessageDialog( 
                  new JFrame(), "Nuance Dimensioner", 
                  "about", 
                  JOptionPane.INFORMATION_MESSAGE );
                return;
            }
        });
        info_menu.add( about_item );

        menubar.add( file_menu );    
//      menubar.add( options_menu );
//      menubar.add( info_menu );

        return menubar;
    }
}        
