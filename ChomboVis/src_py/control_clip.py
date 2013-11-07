##   _______              __
##  / ___/ /  ___  __ _  / /  ___
## / /__/ _ \/ _ \/  ' \/ _ \/ _ \
## \___/_//_/\___/_/_/_/_.__/\___/ 
##
## This software is copyright (C) by the Lawrence Berkeley
## National Laboratory.  Permission is granted to reproduce
## this software for non-commercial purposes provided that
## this notice is left intact.
## 
## It is acknowledged that the U.S. Government has rights to
## this software under Contract DE-AC03-765F00098 between
## the U.S. Department of Energy and the University of
## California.
##
## This software is provided as a professional and academic
## contribution for joint exchange.  Thus it is experimental,
## is provided ``as is'', with no warranties of any kind
## whatsoever, no support, no promise of updates, or printed
## documentation.  By using this software, you acknowledge
## that the Lawrence Berkeley National Laboratory and
## Regents of the University of California shall have no
## liability with respect to the infringement of other
## copyrights by any part of this software.
##

# File: control_clip.py
# Author: TDSternberg
# Created: 9/27/02

import anag_utils
import anag_megawidgets
from self_control import SelfControl

import Tkinter
import Pmw

class ControlClip( anag_megawidgets.SelfDescribingDialog, SelfControl ):
    """ 
    Control over clipping plane.
    """

    def getShortDescription(self):
        return "Clipping"
    def getLongDescription(self):
        return "See into embedded boundaries and isosurfaces"

    def __init__( self, dep_dict, master=None, **kw ):
        anag_utils.funcTrace()
        anag_megawidgets.SelfDescribingDialog.__init__( self, master, **kw )
        SelfControl.__init__( self, dep_dict,
          [
            { 'name':'inside_out_toggle' },
            { 'name':'plane_is_visible' },
            { 'name':'direction_radiobuttons' },
            { 'name':'position_scale' },
            { 'name':'angle_scales', 'initval':{} },
            { 'name':'show_dialog', 'save':6, 'initval':0}
          ])
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls', 'master', 'kw',
            'dep_dict' )
        self.configure( title = 'Clipping' )
        self.dialog_inventory.registerDialog( self )
        decls.memberFunctionAudit( self )


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

        #
        # Inside-out
        #
        self.inside_out_toggle = anag_megawidgets.Checkbutton(
            self.interior(),
            text = 'inside-out toggle' )
        self.inside_out_toggle.configure(
            command = lambda butt = self.inside_out_toggle, self=self:
                self.vtk_clip.setInsideOutIndicator( butt.get() ))
        self.inside_out_toggle.pack( side = 'top', anchor='w' )

        if( self.vtk_data.is2DMode() ):
            return

        #
        # Toggle clipping plane's visibility
        #
        self.plane_is_visible = anag_megawidgets.Checkbutton(
            self.interior(),
            text = 'make plane visible' )
        self.plane_is_visible.configure(
            command = lambda butt = self.plane_is_visible, self=self:
                self.vtk_clip.setVisiblePlaneIsVisible( butt.get() ))
        self.plane_is_visible.pack( side = 'top', anchor='w' )


        #
        # Direction (x|y|z) radiobuttons.
        #
        self.direction_radiobuttons = anag_megawidgets.RadioSelect(
            self.interior(),
            buttontype = 'radiobutton',
            orient = 'horizontal',
            labelpos = 'nw',
            label_text = 'Direction:',
            hull_borderwidth = 2, hull_relief = 'groove'
            )
        map( self.direction_radiobuttons.add, ('x','y','z'))
        self.direction_radiobuttons.configure(
            command = self._directionCallback )
        self.direction_radiobuttons.pack()

        #
        # Position (along indicated direction) scale.
        #
        self.position_scale = anag_megawidgets.EntryScale(
            self.interior(),
            button_text = 'Position',
            dep_dict = {'saved_states':self.saved_states },
            no_validation=1,
            scale_callback = lambda x, self=self:
                self.vtk_clip.defineClipPlane( position=x )
            )
        self.position_scale.pack(anchor='e')


        #
        # Rotation angle scale.
        #
        for axis in 'altitude', 'azimuth':
            self.angle_scales[axis] = anag_megawidgets.EntryScale(
                self.interior(),
                button_text = axis,
                dep_dict = {'saved_states':self.saved_states },
                no_validation=1,
                from_=0, to=360,
                )
            self.angle_scales[axis].pack(anchor='e')
        self.angle_scales['altitude'].configure(
            scale_callback = lambda x, self=self:
               self.vtk_clip.defineClipPlane( angle0=x ))
        self.angle_scales['azimuth'].configure(
            scale_callback = lambda x, self=self:
               self.vtk_clip.defineClipPlane( angle1=x ))
    

    def _refresh( self ):
        """ Overrides SelfControl._refresh().  See comments there. """
        anag_utils.funcTrace()
        assert( self.saved_states.getNumHDF5sLoaded() > 0 )

        #
        # The dialog itself -- visible or hidden.
        #
        anag_megawidgets.SelfDescribingDialog.restoreDialogVisibility( self )
        if( self.vtk_data.is2DMode() ):
            return

        self.plane_is_visible.set( self.vtk_clip.getVisiblePlaneIsVisible() )
        self.direction_radiobuttons.setcurselection(
            self.vtk_clip.getDirection() )
        self.position_scale.setScaleSterile( self.vtk_clip.getPosition() )
        frum = self.vtk_data.getDomainMin()[self.vtk_clip.getDirection()]
        to   =self.vtk_data.getDomainMax()[self.vtk_clip.getDirection()]
        self.position_scale.configure(
            from_= frum, to = to, scale_normal_resolution=(to-frum)/100.0 )
        self.angle_scales['altitude'].setScaleSterile(self.vtk_clip.getAngle0())
        self.angle_scales['azimuth'].setScaleSterile(self.vtk_clip.getAngle1())


    def show( self ):
        anag_utils.funcTrace()
        self.showGUI()
    def withdraw( self ):
        anag_utils.funcTrace()
        self.withdrawGUI()


    def cleanup( self ):
        anag_utils.funcTrace()
        self.dialog_inventory.unregisterDialog( self )
        self.withdraw()


    def _directionCallback( self, xyz ):
        anag_utils.funcTrace()
        assert( xyz=='x' or xyz=='y' or xyz=='z' )
        self.position_scale.configure(
            from_=self.vtk_data.getDomainMin()[xyz],
            to   =self.vtk_data.getDomainMax()[xyz] )
        self.vtk_clip.defineClipPlane( direction=xyz )













