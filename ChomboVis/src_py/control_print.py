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

# File: control_print.py
# Author: TDSternberg
# Created: 10/15/01

"""
GUI layer for controlling production of hardcopy output.
"""

import Tkinter
import tkFileDialog
import tkMessageBox

from self_control import SelfControl
import anag_megawidgets
import anag_utils

class ControlPrint( anag_megawidgets.SelfDescribingDialog, SelfControl ):
    def __init__( self, dep_dict, master=None, **kw ):
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )
        SelfControl.__init__( self, dep_dict, metadata=
          [ {'name':'filename_entry'},
            {'name':'magnification_scale'},   # scale
            {'name':'filetype_menu'}
          ] )
        self.configure( title = 'Save image to file',
                        buttons = ('OK', 'Close'),
                        command = self.buttonHandler )
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        self.dialog_inventory.registerDialog( self )

        #
        # Filename
        #
        filename_frame = Tkinter.Frame( self.interior() )
        Tkinter.Label( filename_frame, text="File name:" ).pack( side='left' )
        self.filename_entry = Tkinter.Entry( filename_frame )
        self.filename_entry.pack( side='left' )
        filename_frame.pack( anchor='w' )

        #
        # Magnification
        #
        mag_frame = Tkinter.Frame( self.interior() )
        Tkinter.Label( mag_frame, text = 'magnification factor:' ).pack(
            side='left' )
        self.magnification_scale = anag_megawidgets.Scale(
            mag_frame,
            orient = 'horizontal', resolution = 1,
            from_ = 1, to = 10 )
            # Set "to" to a bigger number if need more than 100x magnification.
        self.magnification_scale.pack(side='left')

        #
        # File formats
        #
        format_frame = Tkinter.Frame( self.interior() )
        Tkinter.Label( format_frame, text='format:' ).pack( side='left' )
        format_list = []
        for item in self.vtk_print.getFileFormats():
            format_list.append( '.' + item[0] + ' (' + item[2] + ')' )
        self.filetype_menu = anag_megawidgets.OptionMenu(
            format_frame,
            initialitem = 0,  # format_list[0],
            command = self._filetypeHandler,
            items = format_list )
        self.filetype_menu.pack( side='left' )
        format_frame.pack( anchor='w' )
        self.filetype_menu.invoke(0)

        if self.cmd_line.getOffScreen() != 1:
            mag_frame.pack()


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
    def _refresh( self ):
        anag_utils.funcTrace()

    def cleanup( self ):
        anag_utils.funcTrace()
        self.dialog_inventory.unregisterDialog( self )
        self.withdrawGUI()

    def getShortDescription(self):
        return "Hard copy"
    def getLongDescription(self):
        return "Save current image to file"

    def _filetypeHandler( self, label ):
        anag_utils.funcTrace()
        extension = label[1:4]
        if extension == 'eps':
            self.magnification_scale.setSterile( 1 )
            self.magnification_scale.configure( state = 'disabled' )
        else:
            self.magnification_scale.configure( state = 'normal' )


    def buttonHandler( self, butt_name ):
        anag_utils.funcTrace()

        if butt_name == 'Close':
            self.withdrawGUI()
            return

        extension = self.filetype_menu.getcurselection().split()[0][1:]

        filename = self.filename_entry.get()
        if filename == '':
            tkMessageBox.showerror( title='no file name', message =
                'You must supply a file name' )
            return
        filename_extension_pos = filename.rfind('.')
        if filename_extension_pos == -1:
            filename = filename + '.' + extension
        else:
            filename = filename[:filename_extension_pos] + '.' + extension
        
        try:
            mag_factor = self.magnification_scale.get()
            self.vtk_print.hardCopy( filename, mag_factor, extension )
        except:
            tkMessageBox.showerror( title='failed', message =
              'Failed.  File permission, illegal file name, or other problem.')
        self.withdrawGUI()
