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

# File: vtk_line_segment.py
# Author: TDSternberg
# Created: 11/10/03

import anag_utils
from self_control import SelfControl
import vtkpython

class VtkLineSegment( SelfControl ):
    def __init__( self, color, dep_dict ):
        anag_utils.funcTrace()
        SelfControl.__init__( self, dep_dict, metadata =
          [
            {'name':'is_on', 'initval':0, 'get':1},
            {'name':'actor'},
            {'name':'source'},
            {'name':'property'},
            {'name':'endpoints', 'set':1, 'get':1, 'initval':None },
            {'name':'color', 'set':2, 'initval':(1,1,1)}
          ] )
        self.decls = anag_utils.Declarations( "decls", instance=self )

        self.source = vtkpython.vtkLineSource()
        mapper = self.vtk_vtk.newMapper()
        self.actor = self.vtk_vtk.newActor()
        self.property = self.vtk_vtk.newProperty()
        
        self.property.SetLineWidth( 3.0 )
        self.actor.SetProperty( self.property )
        mapper.SetInput( self.source.GetOutput() )
        mapper.ImmediateModeRenderingOn()
        self.actor.SetMapper( mapper )

        self.setColor( color )


    def turnOn( self, endpoints ):
        """
        Show the ray.
        Arg endpoints should be 2*dim-long.
         """
        anag_utils.funcTrace()

        self.is_on = 1

        if len(endpoints) == 4:
            self.source.SetPoint1( (endpoints[0], endpoints[1], 0) )
            self.source.SetPoint2( (endpoints[2], endpoints[3], 0) )
        else:
            self.source.SetPoint1( (endpoints[0], endpoints[1], endpoints[2]) )
            self.source.SetPoint2( (endpoints[3], endpoints[4], endpoints[5]) )

        self.vtk_vtk.addActor( self.actor )
        self.vtk_vtk.render()


    def turnOff( self ):
        """ Hide the ray. """
        anag_utils.funcTrace()
        self.is_on = 0
        self.vtk_vtk.removeActor( self.actor )
        self.vtk_vtk.render()

    def setColor( self, rgb ):
        apply( self.property.SetColor, rgb )


