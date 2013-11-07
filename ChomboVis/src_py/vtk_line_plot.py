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

# File: line_plot.py
# Author: TDSternberg
# Created: 11/10/03

"""
Object that encapsulates a line segment through a ChomboVis data field.
See also MiscApi.makeLinePlot().
"""

import anag_utils
import self_control
import vtk_line_segment

class VtkLinePlot( self_control.SelfControl ):
#Cut to here
    """
    Encapsulates two columns of numbers -- the abscissas and ordinates for a
    graph.
    Knows how to display (or hide) its position in the VTK window.
    Use multiGnuplot() (below) to produce a Gnuplot-friendly file.

    Do not try to construct instances of this class directly.  The approved way
    to obtain an instance is from the return value of MiscApi.makeLinePlot().
    See usage example below, under multiGnuplot().
    """

#Cut from here
    def __init__( self, data, p0, p1, dep_dict ):
        anag_utils.funcTrace()
        self_control.SelfControl.__init__( self, dep_dict, [] )
        self.data = data
        self.p0 = p0
        self.p1 = p1
        self.line_segment = vtk_line_segment.VtkLineSegment( color=(1,1,0),
            dep_dict={'vtk_vtk':self.vtk_vtk} )
#Cut to here
    

    def turnOn( self ):
        """
        Draw a yellow line segment over this line plot's domain, in the VTK
        window.
        """
        anag_utils.funcTrace()
        self.line_segment.turnOn( endpoints = self.p0 + self.p1 )

    def turnOff( self ):
        """
        Remove the yellow line that self.turnOn() draws.
        """
        anag_utils.funcTrace()
        self.line_segment.turnOff()


    def toFile( self, outfilename ):
        """
        Prints data in gnuplot-friendly format.
        """
        anag_utils.funcTrace()
        outfile = open(outfilename, 'w')
        for p in self.data:
            outfile.write( str(p[0]) + ' ' + str(p[1]) + '\n' )


    def getData( self ):
        """
        Returns the basic numbers -- abscissa and ordinate pairs -- as a tuple
        of pairs (2-tuples).
        """
        return self.data


def multiGnuplot( lineplots, outfilename ):
    """
    This function dumps all the data of a collection of lineplots into a file
    that gnuplot will draw as so many differently-colored graphs.  There's much
    more you can do with gnuplot, but this might be a convenient place to start.

    Arg lineplots is a tuple of VtkLinePlot objects.

    Example 1:
    $ cat examples/lineplot.py
    import chombovis
    import vtk_line_plot
    c=chombovis.latest()

    lineplots = []
    for i in range(0,3):
        p0 = (-2.14, 4.285+i/10.0, -0.31)
        p1 = (-2.14, 4.285+i/10.0,  0.49)
        lineplots.append( c.misc.makeLinePlot( p0,p1,8,0,2 ) )
    vtk_line_plot.multiGnuplot( lineplots, 'multi.gp' )
    $ chombovis -i user_script=examples/lineplot.py data/particles3d.hdf5
    >>> ^d
    $ gnuplot multi.gp -

    This will work with gnuplot 4.0.  If you have gnuplot 3.x, you might
    get an error.  In that case, edit the first line of multi.gp so that
    instead of
        plot '-' with lines, '-' with lines, '-' with lines
    it reads
        plot '-0' with lines, '-1' with lines, '-2' with lines


    Example 2:
        Under the ChomboVis source tree, examples/lineplot_multifile.py is a
        program that loops over a set of hdf5 files, creating one gnuplot- or
        matlab-format file out of lineplots made from the data in each file.
    """
    anag_utils.funcTrace()
    outfile = open( outfilename, 'w' )
    
    outfile.write( "plot " )
    for i in range(0,len(lineplots)):
        outfile.write( "'-' with lines" )
        if i < len(lineplots)-1:
            outfile.write( ', ' )
    outfile.write( '\n' )

    for lp in lineplots:
        for p in lp.getData():
            outfile.write( str(p[0]) + ' ' + str(p[1]) + '\n' )
        outfile.write( 'eod\n' )
