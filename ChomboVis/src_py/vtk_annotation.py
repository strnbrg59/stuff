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

# File: vtk_annotation.py
# Author: TDSternberg
# Created: 4/26/04

import anag_utils
import vtk
from self_control import SelfControl

class VtkAnnotation( SelfControl ):
    
    def __init__(self, dep_dict):
        anag_utils.funcTrace()
        instance_data = [
            {'name':'caption_list', 'get':1, 'save':1, 'initval':[]},
            {'name':'curr_caption_index', 'get':1, 'set':1, 'save':1},
            {'name':'text_pipelines', 'initval':[]}
        ]
        SelfControl.__init__( self,dep_dict, instance_data)

        self.decls = anag_utils.Declarations( "decls", instance=self )
        decls = anag_utils.Declarations( "decls", "instance_data", "dep_dict" )
        self.defineAccessors()


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

    def _refresh( self ):
        anag_utils.funcTrace()
        curr_caption_index_saved = self.curr_caption_index
        self.curr_caption_index = 0
        for c in self.caption_list:
            tp = TextPipeline()
            self.text_pipelines.append( tp )
            self.updateVis()
            self.curr_caption_index += 1
        self.curr_caption_index = curr_caption_index_saved


    def cleanup( self ):
        anag_utils.funcTrace()
        for tp in self.text_pipelines:
            self.vtk_vtk.removeActor( tp.actor )


    def makeDefaultCaption( self ):
        """
        Set some reasonable defaults.
        """
        anag_utils.funcTrace()
        return {
            'text':'type your caption here',
            'size':32,
            'opacity':100,
            'font':g_fonts.arial,
            'bold':0,
            'italic':0,
            'x_position':0.2,
            'y_position':0.5,
            'z_position':0.0,
            'x_rotation':0.0,
            'y_rotation':0.0,
            'z_rotation':0.0,
            'color':(0.9,0.9,0.0),
            'mode':g_modes.static2d
            }


    def newCaption( self ):
        """
        Append a new caption to self.caption_list and self.text_pipelines, and
        set the curr_caption_index to point to it.
        """
        anag_utils.funcTrace()
        self.caption_list.append( self.makeDefaultCaption() )
        ndx = len(self.caption_list) - 1
        self.curr_caption_index = ndx

        self.text_pipelines.append( TextPipeline() )
        self.updateVis()


    def deleteCaption( self, index ):
        anag_utils.funcTrace()
        del self.caption_list[index]

        self.vtk_vtk.removeActor(self.text_pipelines[index].actor)
        del self.text_pipelines[index]

        if index > 0:
            self.curr_caption_index = index-1
        else:
            if len(self.caption_list) > 0:
                self.curr_caption_index = 0
            else:
                self.curr_caption_index = None


    def updateVis( self ):
        """
        Update the VTK state of the current caption.
        """
        anag_utils.funcTrace()
        
        # We carry mode (see class _Modes) information around in two places.
        # Each element of self.caption_list has a 'mode' key; it's the mode
        # of that caption.  But also, each TextPipeline instance has a self.mode
        # and it serves to tell us what a caption's "previous mode" was.

        tp = self.text_pipelines[ self.curr_caption_index ]

        # vtkTextMapper
        if not g_modes.isExtrusionMode( self.getMode() ):
            # Set up non-extrusion pipeline, if necessary
            if tp.mode==None or g_modes.isExtrusionMode(tp.mode):
                tp.mapper = vtk.vtkTextMapper()
                if tp.mode != None:
                    self.vtk_vtk.removeActor( tp.actor )
                # FIXME: What's the Mesa equiv of vtkActor2D()?
                tp.actor = vtk.vtkActor2D()
                tp.actor.SetMapper( tp.mapper )
                self.vtk_vtk.addActor( tp.actor )

            text_prop = tp.mapper.GetTextProperty()
    
            tp.mapper.SetInput( self.getText() )
            text_prop.SetFontSize( int(self.getSize()) )
            text_prop.SetOpacity( float(self.getOpacity()/100.0) )
    
            if   self.getFont() == g_fonts.arial:
                text_prop.SetFontFamilyToArial()
            elif self.getFont() == g_fonts.courier:
                text_prop.SetFontFamilyToCourier()
            elif self.getFont() == g_fonts.times:
                text_prop.SetFontFamilyToTimes()
            else:
                assert(None)
    
            text_prop.SetBold( self.getBold() )
            text_prop.SetItalic( self.getItalic() )
    
            if  self.getMode() == g_modes.dynamic2d:
                tp.actor.GetPositionCoordinate().SetCoordinateSystemToWorld()
                tp.actor.GetPositionCoordinate().SetValue(
                    float(self.getXPosition()),
                    float(self.getYPosition()),
                    float(self.getZPosition()) )
            else:
                tp.actor.GetPositionCoordinate(
                    ).SetCoordinateSystemToNormalizedDisplay()
                tp.actor.GetPositionCoordinate().SetValue(
                    float(self.getXPosition()),
                    1.0 - float(self.getYPosition()) )

    
        # Extrusion
        if( g_modes.isExtrusionMode( self.getMode() )
        and (self.getText() != '') ):
            # Set up extrusion pipeline, if necessary
            if tp.mode==None or (not g_modes.isExtrusionMode(tp.mode)):
                tp.mapper = self.vtk_vtk.newMapper()
                tp.mapper.SetInput( tp.normals.GetOutput() )
                if tp.mode != None:
                    self.vtk_vtk.removeActor( tp.actor )
                tp.actor = self.vtk_vtk.newActor()
                tp.actor.SetMapper( tp.mapper )
                self.vtk_vtk.addActor( tp.actor )

            actor_prop = tp.actor.GetProperty()
            actor_prop.SetOpacity( float(self.getOpacity()/100.0) )
    
            extents = self.vtk_data.getDomainExtentsXYZ()
            sizes = {'x':extents[3]-extents[0],
                     'y':extents[4]-extents[1],
                     'z':extents[5]-extents[2] }
            max_size = max( sizes['x'], sizes['y'], sizes['z'] )
            tp.vector_text.SetText( self.getText() )
            tp.actor.SetScale( float(self.getSize()) * 0.01 * max_size)
            tp.actor.SetPosition(
                float(self.getXPosition()),
                float(self.getYPosition()),
                float(self.getZPosition()) )
            tp.actor.SetOrientation(
                float(self.getXRotation()),
                float(self.getYRotation()),
                float(self.getZRotation()) )

        # Common to all modes:
        tp.mode = self.getMode()
        tp.actor.GetProperty().SetColor( self.getColor() )
        self.vtk_vtk.render()


    def localGenericGetter( self, var_name ):
        """
        We make one of these for each element of a caption dictionary, and
        add it to this class' dictionary.
        It's called localGenericGetter because genericGetter is defined in
        the superclass SelfControl.
        """
        anag_utils.funcTrace()
        return self.caption_list[self.curr_caption_index][var_name]

    def localGenericSetter( self, var_name, new_value ):
        """
        See genericSetter()
        """
        anag_utils.funcTrace()
        if self.curr_caption_index != None:
            self.caption_list[self.curr_caption_index][var_name] = new_value
            self.updateVis()        


    def defineAccessors( self ):
        """
        Create getter and setter functions for the various elements of a
        caption dictionary.  Each getter/setter refers to the current caption.
        Setters end with self.updateVis().
        """
        anag_utils.funcTrace()

        names = { 'text':'Text',
                  'size':'Size',
                  'opacity':'Opacity',
                  'font':'Font',
                  'bold':'Bold',
                  'italic':'Italic',
                  'x_position':'XPosition',
                  'y_position':'YPosition',
                  'z_position':'ZPosition',
                  'x_rotation':'XRotation',
                  'y_rotation':'YRotation',
                  'z_rotation':'ZRotation',
                  'color':'Color',
                  'mode':'Mode'
                }
        for k in names.keys():
            getter = lambda self=self, var_name=k:\
                self.localGenericGetter( var_name)
            self.__dict__['get' + names[k]] = getter
            setter = lambda value, var_name=k, self=self:\
                self.localGenericSetter(var_name, value)
            self.__dict__['set' + names[k]] = setter


class TextPipeline:
    """
    Utility class that contains the various VTK sources and filters that it
    takes to create the text.  We use a different set of VTK objects for
    our overlay-plane mode (letters are 2D and don't move with the scene) than
    for our follower mode (letters are 3D and move with the scene).

    One instance of this class is associated with each caption;
    corresponding elements of lists self.caption_list and self.text_pipelines
    go together.  We'd roll them all up into one object, except we want
    persistence, and we can't pickle vtk objects.
    """
    def __init__( self ):
        self.vector_text = vtk.vtkVectorText()

        self.extruder = vtk.vtkLinearExtrusionFilter()
        self.extruder.SetScaleFactor( 0.25 )
        self.extruder.SetExtrusionTypeToNormalExtrusion()
        self.extruder.SetVector(0,0,1)
        self.extruder.SetInput( self.vector_text.GetOutput() )

        self.normals = vtk.vtkPolyDataNormals()
        self.normals.SetInput( self.extruder.GetOutput() )
        self.normals.SetFeatureAngle( 30 )

        self.mode = None # Instance of class _Modes

        # mapper and actor: different types of VTK objects, depending on mode,
        # so need to be redefined in updateVis().


class _Fonts:
    arial   = 'arial'
    courier = 'courier'
    times   = 'times'
g_fonts = _Fonts()


class _Modes:
    static2d  = 'static2d'
    dynamic2d = 'dynamic2d'
    dynamic3d = 'dynamic3d'
    # FIXME: look into vtkCaptionActor2D

    def isExtrusionMode( self, mode ):
        """
        Returns true if arg mode is one that requires the
        vtkLinearExtrusionFilter, false otherwise.
        """
        anag_utils.funcTrace()
        return  mode == self.dynamic3d
g_modes = _Modes()
