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

# File: annotation_api.py
# Author: TDSternberg
# Created: 5/4/2004

import anag_utils
from self_control import SelfControl

class AnnotationApi( SelfControl ):
#Cut to here
    """
    Place any number of captions on the render window.  Control their size,
    font, color, position and other properties.  The functions that control
    those properties -- setSize(), setFont(), etc -- apply to the caption most
    recently created (i.e. from a call to addNewCaption()).  Thus, there's no
    going back to modify an old caption, once you have called addNewCaption()
    again.
    """

#Cut from here
    def __init__( self, dep_dict ):
        anag_utils.funcTrace()
        instance_vars = []
        SelfControl.__init__( self, dep_dict, instance_vars)
        self.decls = anag_utils.Declarations( 'decls', instance=self )
        decls = anag_utils.Declarations( 'decls','self','instance_vars',
            'dep_dict' )

        decls.memberFunctionAudit(self)

    #
    # Functions required by base class SelfControl.
    #
    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
    def _refresh( self ):
        anag_utils.funcTrace()
    def cleanup( self ):
        anag_utils.funcTrace()

#Cut to here
    def showGUI( self ):
        anag_utils.apiTrace()
        self.control_annotation.showGUI()

    def withdrawGUI( self ):
        anag_utils.apiTrace()
        self.control_annotation.withdrawGUI()


    def addNewCaption( self ):
        """
        Create a new caption (i.e. annotation).
        Use setText(), setSize(), etc to modify its properties.
        """
        self.vtk_annotation.newCaption()


    def setText( self, text_string ):
        """
        Set the text in the most recent caption created with addNewCaption().
        Use '\n' to insert a newline.
        """
        anag_utils.apiTrace()
        self.vtk_annotation.setText( text_string )


    def setSize( self, points ):
        """
        Set the size, in points, of the most recent caption created with
        addNewCaption().
        """
        anag_utils.apiTrace()
        self.vtk_annotation.setSize( points )


    def setOpacity( self, opacity ):
        """
        On a scale of 0 to 100, set the opacity of the most recent caption
        created with addNewCaption().
        """
        anag_utils.apiTrace()
        assert( opacity >= 0  and opacity <= 100 )
        self.vtk_annotation.setOpacity( opacity )


    def setFont( self, font_name ):
        """
        Set the font family the most recent caption created with
        addNewCaption().  Legal values 'arial', 'courier' and 'times'.
        """
        anag_utils.apiTrace()
        assert( font_name=='arial'  or  font_name=='courier'
            or  font_name=='times' )
        self.vtk_annotation.setFont( font_name )


    def setBold( self, zero_one ):
        """
        Arg 0 turns off bold font face, 1 turns it on.
        """
        anag_utils.apiTrace()
        assert( zero_one==0  or  zero_one==1 )
        self.vtk_annotation.setBold( zero_one )
    def setItalic( self, zero_one ):
        """
        Arg 0 turns off italic font face, 1 turns it on.
        """
        anag_utils.apiTrace()
        assert( zero_one==0  or  zero_one==1 )
        self.vtk_annotation.setItalic( zero_one )


    def setColor( self, red, green, blue ):
        """
        Indicate the color in rgb space.  Legal values for each argument lie in
        [0,1].
        """
        anag_utils.apiTrace()
        assert( red >= 0.0 and red <= 1.0
           and  green >= 0.0 and green <= 1.0
           and  blue >= 0.0 and blue <= 1.0 )
        self.vtk_annotation.setColor( (red,green,blue) )


    def setPosition( self, x, y, z=0 ):
        """
        Set the position of the most recent caption created with
        addNewCaption().
        In static2d mode, legal values for (x,y) are points on the unit
        square; coordinates are interpreted as a fraction across, and down,
        the VTK window.
        In dynamic2d and dynamic3d mode, legal values for (x,y,z) are
        anything; coordinates are world coordinates.
        The caption is bottom-left justified.
        """
        anag_utils.apiTrace()
        self.vtk_annotation.setXPosition( x )
        self.vtk_annotation.setYPosition( y )
        self.vtk_annotation.setZPosition( z )


    def setOrientation( self, x, y, z=0 ):
        """
        Set the rotational orientation of the most recent caption created with
        addNewCaption().  Coordinate system is world, and the rotations are
        applied in the order z, then x, then y.
        The arguments are interpreted as degrees.
        This only applies to "dynamic3d" captions.
        """
        anag_utils.apiTrace()
        self.vtk_annotation.setZRotation( z )
        self.vtk_annotation.setXRotation( x )
        self.vtk_annotation.setYRotation( y )

    
    def setMode( self, mode ):
        """
        Choices are:
            'static2d':
            'dynamic2d':
            'dynamic3d':
        """
        anag_utils.apiTrace()
        self.vtk_annotation.setMode( mode )
