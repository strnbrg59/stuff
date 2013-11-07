#
#   _______              __
#  / ___/ /  ___  __ _  / /  ___
# / /__/ _ \/ _ \/  ' \/ _ \/ _ \
# \___/_//_/\___/_/_/_/_.__/\___/ 
#
# This software is copyright (C) by the Lawrence Berkeley
# National Laboratory.  Permission is granted to reproduce
# this software for non-commercial purposes provided that
# this notice is left intact.
# 
# It is acknowledged that the U.S. Government has rights to
# this software under Contract DE-AC03-765F00098 between
# the U.S. Department of Energy and the University of
# California.
#
# This software is provided as a professional and academic
# contribution for joint exchange.  Thus it is experimental,
# is provided ``as is'', with no warranties of any kind
# whatsoever, no support, no promise of updates, or printed
# documentation.  By using this software, you acknowledge
# that the Lawrence Berkeley National Laboratory and
# Regents of the University of California shall have no
# liability with respect to the infringement of other
# copyrights by any part of this software.
#

import Tkinter
import tkSimpleDialog
import tkFileDialog
import tkMessageBox
import Pmw
import sys
import os
import string

import anag_utils
import anag_megawidgets
import visualizable_dataset
import self_control # For module function resetMultiInstanceQualifier()
from self_control import SelfControl


class Menubar( SelfControl ):
    """
    Everything that manipulates the vis state, i.e. everything that the user
    might hope to be able to accomplish from the API, is implemented in terms
    of the API alone.  This is so we can journal the actions in a file that
    can later be used as a legal ChomboVis Python script.

    Low-level stuff -- infrastructure, bookkeeping, GUI -- may be implemented
    with things not in the API.
    """

    def __init__(self, dep_dict):
        #
        # Variable "declarations"
        #
        instance_data = [
            { 'name':'menu_bar'},
            { 'name':'message_bar'},
            { 'name':'chologos'},
            { 'name':'autorender'},
            { 'name':'debuglevel'},
            { 'name':'balloon'},
            { 'name':'vis_controls'},
            { 'name':'slave_mode'},
            { 'name':'master_mode'},
            { 'name':'debug_level_dialog'},
        ]
        SelfControl.__init__( self,dep_dict, instance_data)
        self.decls = anag_utils.Declarations( "decls", instance=self )
        decls = anag_utils.Declarations( "decls",
            "dep_dict", 
            "parent",
            "s", "n"
        )

        self.chologos = []

        parent = self.vtk_vtk.getMenuFrame()
        self.balloon = Pmw.Balloon( parent )
        self.menu_bar = Pmw.MenuBar( parent,
                hull_relief = 'raised',
                hull_borderwidth = 1,
                balloon = self.balloon)

        # Message area, to display help on menu options.
        self.message_bar = Pmw.MessageBar(parent)
        self.message_bar.pack(fill = 'x')
        self.balloon.configure(statuscommand = self.message_bar.helpmessage)

        self.menu_bar.pack(fill = 'x')

        #
        # File menu
        # 
        self.menu_bar.addmenu(menuName='File',
                              tearoff=1,
                              balloonHelp='Save, load, undo')
        self.menu_bar.addmenuitem('File', 'command',
                'Load an hdf5 data file',
                command = lambda self=self: self._loadNewHdf5(),
                label = 'Load hdf5')
        self.menu_bar.addmenuitem('File', 'command',
                'Load an hdf5 data file, retaining current state',
                command = lambda self=self: self._loadNewHdf5(preserve_state=1),
                label = 'Load hdf5, preserving state')
        self.menu_bar.addmenuitem('File', 'command',
                'Save data, including generated components, to an hdf5 file. '+
                'If in reslice mode, saves the slice.',
                command = lambda self=self: self._saveToHdf5(),
                label = 'Save to hdf5')

        self.menu_bar.addmenuitem('File', 'separator')

        """
        self.menu_bar.addmenuitem('File', 'command',
            'Save a Python script that reconstructs current state',
            command = self._saveStateAsPythonScript,
            label = 'Save journal script' )
        """
        self.menu_bar.addmenuitem('File', 'command',
            'Save current state of ChomboVis',
            command = lambda self=self: self._saveState(),
            label = 'Save state' )
        self.menu_bar.addmenuitem('File', 'command',
            'Restore a previously stored state of ChomboVis',
            command = lambda self=self: self._restoreState(),
            label = 'Restore state' )

        self.menu_bar.addmenuitem('File', 'separator')

        self.menu_bar.addmenuitem('File', 'command', 
            'Save the image to file',
            command = lambda self=self: self.control_print.showGUI(),
            label = 'Save image')
        self.menu_bar.addmenuitem('File', 'separator')
        self.menu_bar.addmenuitem('File', 'command', 'Exit the application',
                command = self._exit, label = 'Exit' )

        # Disable options irrelevant until we have an hdf5 loaded.
        for opt in ( 'Load hdf5, preserving state', 'Save to hdf5',
                     'Save state', 'Restore state', 'Save image'):
            self.menu_bar.component('File-menu').entryconfigure(
                opt, state='disabled')

        #
        # Visualization menu
        #
        self.menu_bar.addmenu(
            menuName='Visualization',
            tearoff=1,
            balloonHelp='Look at the data in different ways' )
        self.vis_controls = [
                           self.control_data,
                           self.control_cmap,
                           self.control_clip,
                           self.control_grid,
                           self.control_slice,
                           self.control_iso,
                           self.control_stream,
                           self.control_vol,
                           self.control_eb,
                           self.control_particles
                       ]
        self.menu_bar.component('Visualization-button').configure(
            state='disabled')


        #
        # Tools menu
        #
        self.menu_bar.addmenu(
            menuName='Tools',
            tearoff=1,
            balloonHelp='data browsers, camera parameters,...' )
        self.menu_bar.addmenuitem(
            menuName='Tools',
            itemType='command',
            statusHelp='Databrowser launcher',
            label = 'Data browsers',
            command = lambda self=self: self.control_fab_tables.showGUI()
            )

        def datasummaryDialog( txt, dialog_inventory ):
            d = Pmw.TextDialog( title='Data summary')
            dialog_inventory.registerDialog( d )
            d.insert( 'end', txt )
        self.menu_bar.addmenuitem(
            menuName='Tools',
            itemType='command',
            statusHelp='Data summary',
            label = 'Data summary',
            command = lambda self=self, f=datasummaryDialog: 
                f( self.reader_api.getDataSummary(), self.dialog_inventory ))
        self.menu_bar.addmenuitem(
            menuName='Tools',
            itemType='command',
            statusHelp='Camera parameters',
            label = 'Camera parameters',
            command = self.control_cameras.showGUI
            )
        self.menu_bar.addmenuitem(
            menuName='Tools',
            itemType='command',
            statusHelp='Annotation',
            label = 'Annotation',
            command = self.control_annotation.showGUI
            )
        self.menu_bar.component('Tools-button').configure(state='disabled')


        #
        # Options menu
        #
        self.menu_bar.addmenu('Options', 'Set user preferences', tearoff=1)

        # Autorender checkbutton.
        self.autorender = Tkinter.IntVar()
        self.menu_bar.addmenuitem( 
            menuName='Options',
            itemType='checkbutton',
            statusHelp='When off, a click in the main window forces a rerender',
            label = 'Autorender',
            command = lambda self=self:
                self.vtk_vtk.setAutorender(self.autorender.get()),
            variable = self.autorender)

        # Debug level
        self.menu_bar.addmenuitem(
            menuName='Options',
            itemType='command',
            statusHelp='Set the debug levels. 0=nothing, 1=fatal, 2=error, 3=warning, 4=info, 5=trace',
            label = 'Set debug level',
            command = self._debugLevelDialog )

        self.menu_bar.component('Options-button').configure(state='disabled')

        #
        # Close, reopen and flash all dialogs.
        #
        self.menu_bar.addmenuitem('Options', 'separator')
        self.menu_bar.addmenuitem(
            menuName='Options',
            itemType='command',
            statusHelp='Close all dialogs belonging to this process',
            label = 'Close dialogs',
            command = lambda self=self:
                self.dialog_inventory.closeAllDialogs() )
        self.menu_bar.addmenuitem(
            menuName='Options',
            itemType='command',
            statusHelp='Reopen all previously-open dialogs',
            label = 'Reopen dialogs',
            command = lambda self=self:
                self.dialog_inventory.reopenDialogs() )
        self.menu_bar.addmenuitem(
            menuName='Options',
            itemType='command',
            statusHelp='Briefly close, then reopen, all dialogs',
            label = 'Flash dialogs',
            command = lambda self=self:
                self.dialog_inventory.flashDialogs() )
        self.menu_bar.addmenuitem('Options', 'separator')


        #
        # Master/slave
        #
        self.slave_mode = Tkinter.IntVar()
        self.master_mode = Tkinter.IntVar()
        self.menu_bar.addmenuitem(
            menuName='Options',
            itemType='checkbutton',
            statusHelp='Become controlled by another ChomboVis',
            label = 'slave',
            variable = self.slave_mode,
            command = lambda self=self:
                self._beSlave( self.slave_mode.get() ))
        self.menu_bar.addmenuitem(
            menuName='Options',
            itemType='checkbutton',
            statusHelp='Take control of another ChomboVis',
            label = 'master',
            variable = self.master_mode,
            command = lambda self=self:
                self._beMaster( self.master_mode.get() ))
        self.menu_bar.addmenuitem('Options', 'separator')


        #
        # Help menu
        #
        self.menu_bar.addmenu(
            menuName='Help', balloonHelp='help', tearoff=1, side='right')
        self.menu_bar.addmenuitem(
            menuName='Help',
            itemType='command',
            statusHelp='Version of ChomboVis you are running',
            label = 'Release number',
            command = lambda self=self: self._about())
        self.menu_bar.addmenuitem(
            menuName='Help',
            itemType='command',
            statusHelp='Find out about chombovis',
            label = 'Documentation',
            command = lambda self=self: self._startWwwBrowser())



    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
        self.message_bar.message('state', 'viewing ' +
            self.cmd_line.getInfile() )

        # Say "Isosurfaces" or "Isocontours" as appropriate to dimensionality.
        for mode in self.vis_controls:
            self.menu_bar.addmenuitem(
                menuName='Visualization',
                itemType='command',
                command = lambda mode=mode: mode.showGUI(),
                label = mode.getShortDescription(),
                statusHelp=mode.getLongDescription()
                )

        # Enable menus (but not irrelevant ones).
        self.menu_bar.component('Visualization-button').configure(
            state='normal')
        self.menu_bar.component('File-menu').entryconfigure(
            'Save image', state='normal')
        self.menu_bar.component('Options-button').configure( state='normal')
        self.menu_bar.component('Tools-button').configure( state='normal')

        # Enable data-menu options.
        for opt in ( 'Load hdf5, preserving state', 'Save to hdf5',
                     'Save state', 'Restore state', 'Save image'):
            self.menu_bar.component('File-menu').entryconfigure(
                opt, state='normal')


        # Disable visualization-menu options irrelevant until we've loaded a
        # data component.
        for vis_mode in ( self.control_slice,
                          self.control_iso, self.control_stream,
                          self.control_vol, self.control_eb ):
            self.menu_bar.component('Visualization-menu').entryconfigure(
                vis_mode.getShortDescription(), state='disabled')
        self.menu_bar.component('Visualization-menu').entryconfigure(
            self.control_clip.getShortDescription(), state='normal' )
        if self.vtk_data.getNumParticles() == 0:
            self.menu_bar.component('Visualization-menu').entryconfigure(
                self.control_particles.getShortDescription(), state='disabled')


    def _initForFirstLoadedComponent( self, dummy1, dummy2 ):
        """
        Do what needs to be done upon loading a component from a situation in
        which no component had been loaded.
        """
        anag_utils.funcTrace()
        self._refresh()


    def _refresh( self, dummy1=None, dummy2=None ):
        assert( self.saved_states.getNumHDF5sLoaded() > 0 )

        self.registerCallback(
            self.vtk_data.getNotifierLoadedFirstComponentNotifier(),
            self._initForFirstLoadedComponent )
        if not self.vtk_data.getCurComponent():
            return   # Will come back here upon loading first component.

        self.autorender.set( self.vtk_vtk.getAutorender() )
        self.registerCallback(
            self.network_api.getNotifierMasterEnabled(),
            lambda on_off, dummy, self=self:
                self._setMasterButton(on_off))
        self.registerCallback(
            self.network_api.getNotifierSlaveEnabled(),
            lambda on_off, dummy, self=self:
                self._setSlaveButton(on_off))

        for vis_mode in self.control_slice, self.control_iso:
            self.menu_bar.component('Visualization-menu').entryconfigure(
                vis_mode.getShortDescription(), state='normal')

        if not self.vtk_data.isResliceMode():
            # Not until we learn to do Cauchy maps...            
            self.menu_bar.component('Visualization-menu').entryconfigure(
                self.control_stream.getShortDescription(), state='normal' )

        if not self.vtk_data.is2DMode():
            self.menu_bar.component('Visualization-menu').entryconfigure(
                self.control_vol.getShortDescription(), state='normal' )

        if( (not self.vtk_data.isResliceMode())
        or  (not self.control_eb.isOldStyle()) ):
            self.menu_bar.component('Visualization-menu').entryconfigure(
                self.control_eb.getShortDescription(), state='normal' )

        if self.vtk_data.getNumParticles() > 0:
            self.menu_bar.component('Visualization-menu').entryconfigure(
                self.control_particles.getShortDescription(), state='normal' )


    def cleanup( self ):
        anag_utils.funcTrace()

        for name in 'File', 'Visualization', 'Tools', 'Options', 'Help':
            self.menu_bar.deletemenu( name )
 
        if self.cmd_line.getOffScreen() != 1:
            chilun = self.vtk_vtk.getMenuFrame().children
            for k in chilun.keys():
                try:
                    chilun[k].forget()
                except:
                    #anag_utils.warning( 'Cannot call forget() on', chilun[k] )
                    pass


    def _saveStateAsPythonScript( self ):
        """
        Copy journal file (which we've been maintaining all along in /tmp)
        to a filename of the user's choice.
        """
        anag_utils.funcTrace()
        outfilename = tkFileDialog.asksaveasfilename(
                filetypes = [('Python scripts', "*.py")])
        if outfilename == '':
            return

        suffix_pos = outfilename.rfind('.')
        if (suffix_pos == -1) or (outfilename[suffix_pos:] != '.py'):
            outfilename = outfilename + '.py'

        try:
            anag_utils.journalCopyTo( outfilename )
        except:  # Whatever
            anag_utils.excepthook()
            response = tkMessageBox.askquestion(
                message = "Bad file name.  Try again?" )
            if response == 'yes':
                self._saveStateAsPythonScript()


    def _saveState(self):
        """
        Save the current state of pyChomboVis, to a file.
        """
        anag_utils.funcTrace()
        outfilename = tkFileDialog.asksaveasfilename(
                filetypes = [('ChomboVis state files', "*.state")])
        if outfilename == '':
            return

        suffix_pos = outfilename.rfind('.')
        if (suffix_pos == -1) or (outfilename[suffix_pos:] != '.state'):
            outfilename = outfilename + '.state'

        try:
            self.saved_states.saveState( outfilename )
        except:  # Whatever
            response = tkMessageBox.askquestion(
                message = "Bad file name.  Try again?" )
            if response == 'yes':
                self._saveState()


    def _restoreState(self):
        """
        Restore a previously-saved state of ChomboVis, from a file.
        """
        anag_utils.funcTrace()
        filename = tkFileDialog.askopenfilename( 
                filetypes = [('ChomboVis state files', "*.state")])
        if filename == '': # It's '' if user pressed dialog's "Cancel" button.
            return

        try:
            self.saved_states.loadState( filename )
        except:
            if anag_utils.getDebugLevel() > 3:
                anag_utils.excepthook()
            response = tkMessageBox.askquestion( 
                message = "Bad, nonexistent or inapplicable state file.\n"
                          "Try again?" )
            if response == 'yes':
                self._restoreState()


    def _showBusyIndicator( self, on_off, extra_info ):
        """
        Callback for Notifier that vtk_data sets to let us know we're doing
        a time-consuming vtk update.
        """
        if on_off == 1:
#           self.message_bar.helpmessage( 'Busy with vtk update...' )
            #FIXME: The message doesn't show up until the vtk update is already
            #over.  (But the "Busy..." does print right away.)
            pass
        else:
            pass
#           self.message_bar.resetmessages('busy')


    def _debugLevelDialog( self ):

        def customValidator( self, n ):
            """
            This is where we actually set the new debug level.  This is the
            best way I know to cause the debug level to change without needing
            to press an extra button, e.g. an 'OK' button.

            So the main business here is to set the debug level, but while we're
            at it, we also do entry validation.
            """
            anag_utils.funcTrace()
            try:
                intval = int(n)
            except:
                return Pmw.ERROR
            if not 0 <= intval <= 5:
                return Pmw.ERROR

            self.misc_api.setDebugLevel( int(self.debug_level_dialog.get()) )

            return Pmw.OK


        def close( self, disposition ):
            """ Handler for 'Close' button on the dialog. """
            anag_utils.funcTrace()
            self.debug_level_dialog.withdraw()


        self.debug_level_dialog = Pmw.CounterDialog(
                label_text = 'Set debug level.\n' +
                             '0 = nothing, 1 = fatal, 2 = error,\n' +
                             '3 = warning, 4 = info, 5 = trace',
                counter_labelpos = 'n',
                entryfield_value = anag_utils.getDebugLevel(),
                buttons = ('Close',),
                defaultbutton = 'Close',
                title = 'Debug level',
                command = lambda d, self=self, f=close : f(self, d) )
        self.debug_level_dialog.configure(
                entryfield_validate = lambda n, self=self, f=customValidator:
                    f( self, n ))
        self.dialog_inventory.registerDialog( self.debug_level_dialog )

        # This doesn't work: Can set the width to a big number (e.g. 200), but
        # not a small one; apparently there's a minimum size somewhere, or maybe
        # there's an 'expand' or 'fill' option in there.
        self.debug_level_dialog.component('entryfield').component(
            'entry').configure(width=2)


    def _about( self ):
        """ Show current release number """
        anag_utils.funcTrace()
        tkMessageBox.showinfo( message='ChomboVis release ' +
            self.vtk_data.getReleaseNumber() )


    def _startWwwBrowser(self):
        """ Start web browser pointed to online documentation. """
        anag_utils.funcTrace()
        preferred_browser = os.getenv('WWW_BROWSER')
        if not preferred_browser:
            tkMessageBox.showwarning( message=
                'Please define an environment variable, WWW_BROWSER, '
                'and set it to the path to your preferred web browser. '
                'For example, setenv WWW_BROWSER /usr/local/bin/mozilla.'
                'You can provide extra command-line arguments, too, e.g. '
                'setenv WWW_BROWSER "lynx -cfg $HOME/lynx.cfg"' )
        else:
            pid = os.fork()
            if pid == 0:
                split_browser = preferred_browser.split()
                print "execvp(", split_browser[0],\
                           tuple(
                              split_browser
                            + [os.getenv('CHOMBOVIS_HOME') +
                               '/doc/ChomboVis/UsersGuide.html',] )
                os.execvp( split_browser[0],
                           tuple(
                              split_browser
                            + [os.getenv('CHOMBOVIS_HOME') +
                               '/doc/ChomboVis/UsersGuide.html',] ) )
                sys.exit(1) # Should never get here


    def _beMaster( self, on_off ):
        """
        Go into master mode if on_off==1.
        """
        anag_utils.funcTrace()
        if on_off == 1:
            response = tkMessageBox.askokcancel(
                    message = "This process will hang now until you put another"
                              " ChomboVis process into slave mode.  Proceed?" )
            if response == 0:
                self._setMasterButton( 0 )
                return                 # Early return
            else:
                self._setMasterButton( 1 )
        else:
            self._setMasterButton( 0 )

        self.network_api.beMaster( on_off )


    def _setMasterButton( self, on_off ):
        """
        Deals with just the GUI part -- doesn't call to network_api.
        """
        anag_utils.funcTrace()
        self.master_mode.set( on_off )
        if on_off == 1:
            self.menu_bar.component('Options-menu').entryconfigure(
                'slave', state='disabled' )
        else:
            self.menu_bar.component('Options-menu').entryconfigure(
                'slave', state='normal' )


    def _beSlave( self, on_off ):
        """
        Go into slave mode if on_off==1.
        """
        anag_utils.funcTrace()

        if on_off == 1:
            response = tkMessageBox.askokcancel(
                    message = "This process will hang now until you put another"
                              " ChomboVis process into master mode.  Proceed?" )
            if response == 0:
                self._setSlaveButton( 0 )
                return                 # Early return
            else:
                self._setSlaveButton( 1 )
        else:
            self._setSlaveButton( 0 )

        self.network_api.beSlave( on_off )


    def _setSlaveButton( self, on_off ):
        """
        Deals with just the GUI part -- doesn't call to network_api.
        """
        anag_utils.funcTrace()
        self.slave_mode.set( on_off )
        if on_off == 1:
            self.menu_bar.component('Options-menu').entryconfigure(
                'master', state='disabled' )
        else:
            self.menu_bar.component('Options-menu').entryconfigure(
                'master', state='normal' )


    def _saveToHdf5( self ):
        """
        Saves the currently visualizable data (including generated components)
        to an HDF5 file.  If currently in reslice mode, saves a slice of the
        data, at the current slicing axis and slicing position.
        """
        anag_utils.funcTrace()

        outfile = tkFileDialog.asksaveasfilename(
            filetypes=(("HDF5 files",".hdf5"),))
        if not outfile:
            return                                   # Early return

        axis = None
        axis_position = None
        if self.vtk_data.isResliceMode():
            axis = self.vtk_data.getResliceDirection()
            axis_position = self.vtk_data.getReslicePosition()[axis]

        self.reader_api.saveToHDF5( outfile, axis, axis_position )


    def _loadNewHdf5(self, preserve_state=None ):
        """
        If arg preserve_state!=None, then try to load the new file in the same
        state as the current file.  Will fail if old state mentions a level or
        box or component etc that doesn't exist in the new file.
        """
        anag_utils.funcTrace()
        current_infile = self.vtk_data.getInfileName()
        new_infile = tkFileDialog.askopenfilename(
            filetypes=(("HDF5 files",".hdf5"), ("All files", "*")))
        if new_infile:
            try:
                pid = os.getpid()
                (temp_statefile_name, dummy) =\
                    anag_utils.g_temp_file_mgr.makeTempFile(
                        'chombovis', 'state', create=0 )
                self.saved_states.saveState( temp_statefile_name )
         
                self.reader_api.loadHDF5( new_infile, preserve_state )
            except:
                anag_utils.excepthook()
                response = tkMessageBox.showerror(
                    message = "Incompatible files.  Previous state has been "
                      + "stored in " + temp_statefile_name + ".  "
                      + "The best thing to do now is to load an hdf5 -- any "
                      + "hdf5 -- but until you do, ChomboVis will be in a "
                      + "corrupted state." ) 
                #Tried this but it doesn't work; too much has been corrupted.
                #if response == 'ok':
                #    self._loadNewHdf5( current_infile )
        # Don't try to do anything past this point in the function, as the
        # current instance of menubar will have been destroyed in the process
        # of loading a new hdf5.


    def _exit( self ):
        """
        Exit ChomboVis and clean up stuff.  (Not clear why Journal.__del__
        doesn't get called from sys.exit(0) without this...)
        """
        anag_utils.funcTrace()
        anag_utils.journalCleanup()
        sys.exit(0)


#
# Unit test (doesn't work; need to init lots of other classes).
#    
if __name__ == '__main__':
    root = Tkinter.Tk()
    m = Menubar(root)
    root.mainloop()
