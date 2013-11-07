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

# File: vtk_print.py
# Author: TDSternberg
# Created: 8/20/01

"""
Writes the VTK image to a file, using the vtkRenderLargeImage technique.
This is a stopgap until we can do it by the OpenGL feedback mode technique.

This used to produce CGM output.
But we're going to abandon this CGM strategy.  There are bugs in even the
Tcl version of ChomboVis, and Brian thinks they'd be hard to fix (requiring
work at the GL level among other things).

"""

import tkMessageBox

import anag_utils
from self_control import SelfControl
import vtkpython

class VtkPrint( SelfControl ):
    """
    Produce computer graphics metafile (CGM) format output.
    """
    def __init__(self, dep_dict):
        SelfControl.__init__( self, dep_dict, metadata=
          [ {'name':'file_formats', 'get':1}
          ] )
        self.decls = anag_utils.Declarations( "decls", instance=self )

        self.file_formats = (
            ('eps', None, 'encapsulated Postscript'),
            ('ppm', vtkpython.vtkPNMWriter(), 'PPM'),
            ('bmp', vtkpython.vtkBMPWriter(), 'bitmap'),
            ('tiff', vtkpython.vtkTIFFWriter(), 'TIFF'),
            ('iv' , vtkpython.vtkIVWriter(), 'Inventor'),
            ('stl', vtkpython.vtkSTLWriter(), 'Stereo-lithography'),
            ('mcubes', vtkpython.vtkMCubesWriter(), 'marching cubes'),
            ('pdt', vtkpython.vtkPolyDataWriter(), 'VTK PolyData'),
            ('rg', vtkpython.vtkRectilinearGridWriter(),
                  'VTK RectilinearGrid'),
            ('sg', vtkpython.vtkStructuredGridWriter(),
                  'VTK StructuredGrid'),
            ('sp', vtkpython.vtkStructuredPointsWriter(),
                  'VTK StructuredPoints'),
            ('ug', vtkpython.vtkUnstructuredGridWriter(),
                  'VTK UnstructuredGrid'),
            ('byu', vtkpython.vtkBYUWriter(), 'MOVIE.BYU'))

        # I got this list of writers from the _VTK User's Guide_, May 2001
        # edition, p.186.  On that page, they also list a "vtkFieldDataWriter",
        # but I don't see one in vtk3.2.
        # A dictionary would be more convenient, but I care about the order.


    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()

    def _refresh( self ):
        anag_utils.funcTrace()

    def cleanup( self ):
        anag_utils.funcTrace()


    def hardCopy( self, outfile_name, mag_factor, format_extension ):
        """
        Save the image to a file.
        When in offscreen=1 mode, args mag_factor and format_extension have no
        effect.  To set the size in offscreen mode, you need to call SetSize()
        on the RenderWindow, quite early in the game.  If you call it here, you
        end up with a garbage image.

        Arg format_extension can be 'ppm', 'bmp', 'tiff', or 'eps'.  Setting
        it to 'eps' results in a whole different kind of picture -- an
        encapsulated postscript thing with vector graphics (for some of what's
        been rendered anyway, and mag_factor doesn't do anything in this case).
        """
        anag_utils.funcTrace()

        self.vtk_vtk.render()

        # Case of encapsulated postscript.
        if format_extension == 'eps':
            if self.cmd_line.getTexture() == 1:
              try:
                response = tkMessageBox.askquestion( message =
                    "You are using texture mapping, and the postscript output "
                    "generator does not understand texture mapping.  The best "
                    "thing to do now is to exit ChomboVis, restart it with "
                    "'texture=0' on the command line, and then go back to this "
                    "'save image' menu option.  "
                    "Knowing all that, do you nonetheless wish to ignore this "
                    " warning and proceed to save a Postscript file anyway?" )
                if response == 'no':
                    return
              except:
                    anag_utils.excepthook()
            gl2ps = vtkpython.vtkGL2PSExporter()
            gl2ps.SetRenderWindow( self.vtk_vtk.getRenWin() )

            # This one causes the whole scene to appear blank, whether we use
            # texture-mapping or not:
            # gl2ps.SetWrite3DPropsAsRasterImage(1)

            gl2ps.SetFileFormatToEPS()
            gl2ps.SetPS3Shading(1)
            gl2ps.SetBestRoot(1)

#           gl2ps.SetDrawBackground(1)
            gl2ps.SetFilePrefix( outfile_name[:-4] )
            gl2ps.Write()
            return

    
        # If mag_factor>1, the legend gets duplicated just like extraneous
        # X windows.  So just remove it.    
        if mag_factor != 1:
            self.vtk_cmap.hideLegend()

        if self.cmd_line.getOffScreen() == 1:
            w2if = vtkpython.vtkWindowToImageFilter()
            w2if.SetInput( self.vtk_vtk.getRenWin() )
            self.vtk_vtk.getRenWin().Render()
            w2if.Update()
            pnmWriter = vtkpython.vtkPNMWriter()
            pnmWriter.SetInput( w2if.GetOutput() )
            pnmWriter.SetFileName( outfile_name )
            pnmWriter.Write()
        else:
            render_large = vtkpython.vtkRenderLargeImage()
            render_large.SetInput( self.vtk_vtk.getRenderer() )
            render_large.SetMagnification( mag_factor )

            format_dictionary = {}
            for item in self.file_formats:
                format_dictionary[item[0]] = (item[1],item[2])
            writer = format_dictionary[format_extension][0]
            writer.SetFileName( outfile_name )
            writer.SetInput( render_large.GetOutput() )
            writer.Write()
