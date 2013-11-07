import anag_utils
import algorithms
import vtk

class AnagBoxClipper:
    """
    Does what vtkClipPolyData is supposed to do when clipping against a vtkBox.

    A pipeline of four vtkClipPolyData's, each clipping against a vtkPlane.
    Because clipping against a vtkBox gives clipping limited by the resolution
    of the PolyData.
    """
    def __init__( self, axis ):
        anag_utils.funcTrace()
        self.axis = axis # normal axis
        self.clippers = []
        self.clipping_planes = []
        for i in range(0,4):
            self.clipping_planes.append( vtk.vtkPlane() )
            self.clippers.append( vtk.vtkClipPolyData() )
            self.clippers[i].SetInsideOut(1)
            self.clippers[i].SetClipFunction( self.clipping_planes[i] )
            if i > 0:
                self.clippers[i].SetInput( self.clippers[i-1].GetOutput() )
        self.SetClipBoxBounds( float('-inf'), float('inf'),
                               float('-inf'), float('inf') )


    def SetClipBoxBounds( self, xlo, xhi, ylo, yhi ):
        """
        Interprets x and y as the "other two axes", given self.axis as the
        normal axis (and assuming a right-handed coordinate system).
        """
        anag_utils.funcTrace()
        self.xlo = xlo
        self.xhi = xhi
        self.ylo = ylo
        self.yhi = yhi

        origins = [ (0, self.ylo, 0),       # bottom edge
                    (self.xhi, 0, 0),       # right edge
                    (0, self.yhi, 0 ),      # top edge
                    (self.xlo, 0, 0 ) ]     # left edge
        normals = [ ( 0, -1, 0),
                    ( 1,  0, 0),
                    ( 0,  1, 0),
                    (-1,  0, 0) ]
        # These origins and normals are correct for the z slice.  For x and y
        # we permute as follows:
        for e in range(0,4):
            if   self.axis == 'x':
                origins[e] = (0, origins[e][0], origins[e][1] )
                normals[e] = (0, normals[e][0], normals[e][1] )
            elif self.axis == 'y':
                origins[e] = (origins[e][1], 0, origins[e][0] )
                normals[e] = (normals[e][1], 0, normals[e][0] )

            self.clipping_planes[e].SetOrigin( origins[e] )
            self.clipping_planes[e].SetNormal( normals[e] )

    #
    # From here on down, VTK-like methods (which is why they're capitalized).
    #

    def SetInput( self, polydata ):
        anag_utils.funcTrace()
        self.clippers[0].SetInput( polydata )

    def SetOutput( self, next_pipeline_stage ):
        anag_utils.funcTrace()
        next_pipeline_stage.SetInput( self.clippers[3].GetOutput() )

    def GetOutput( self ):
        anag_utils.funcTrace()
        return self.clippers[3].GetOutput()

    def SetValue( self, x ):
        anag_utils.funcTrace()
        self.clip_value = x
        for i in range(0,4):
            self.clippers[i].SetValue(x)

    def GetValue( self ):
        anag_utils.funcTrace()
        return self.clip_value

    def Update( self ):
        anag_utils.funcTrace()
        self.clippers[3].Update()

    def SetInsideOut( self, one_or_zero ):
        anag_utils.funcTrace()
        assert( one_or_zero == 0  or  one_or_zero == 1 )
        for i in range(0,4):
            self.clippers[i].SetInsideOut( one_or_zero )

    def GetRange( self ):
        anag_utils.funcTrace()
        result = self.clippers[3].GetOutput().GetScalarRange()
        return result
