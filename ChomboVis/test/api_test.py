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

## Author: TDSternberg
# Comprehensive API test.
# Launch it from api_test.sh.

#
# Test case numbers correspond to NASA CT O1 requirements doc
# at http://davis.lbl.gov/NASA/o1requirements.html.
#

import anag_utils
import vtk_particles
import chombovis
import visualizable_dataset
import box_layout_data
import vtk_data
import box
import Tkinter
import os
import sys
import time
import random
try: # From v2.3 on, backward compatibility with random
    g_wichmannhill = random.WichmannHill()
    random = g_wichmannhill
except:
    pass
import types

class Test:
    def __init__( self ):
        self.testname = os.environ['CHOMBO_TESTNAME']
        self.testname_hdf5file_dictionary = {
            'ebclip2d':'node2d.hdf5',
            'ebclip3d':'littlegreg.hdf5',
            'Reslice':'test.3d.float.hdf5',
            '2dCell':'test.2d.hdf5',
            '3dCell':'ebtest.3d.hdf5',
            '2dNode':'node2d.hdf5',
            '3dParticles':'particles3d.hdf5',
            'nodata':'nodata.hdf5',
            'nansinfs':'nansinfs.hdf5'    }
        if not self.testname in self.testname_hdf5file_dictionary.keys():
            anag_utils.fatal( "Illegal arg to api_test.sh:",
                              os.environ['CHOMBO_TESTNAME'] )

        self.c = chombovis.this()

        self.c.misc.setRenderWidgetSize( width=500, height=400 )

        self.runTests( self.c )

        self.c.misc.vtkUpdate()
#       self.c.misc.flashDialogs()
        self.c.misc.closeAllDialogs()

        filename = '/tmp/chombovis_' + os.getenv('USER') + '/api_test' \
                 + self.testname + '.ppm'
        # Can't use anag_utils.TempFileMgr here unless we can tell api_test.sh
        # about the generated file name.
        if self.testname == '2dNode':
            self.c.misc.hardCopy( filename, mag_factor=2 )
        else:
            self.c.misc.hardCopy( filename, mag_factor=1 )


    def getCroppedVisDat( self, c ):
        """
        Return a VisualizableDataset that's been cropped to a slightly shrunken
        version of the current problem domain, and had its 0th level removed.
        """
        anag_utils.funcTrace()
        visdat = c.reader.getVisualizableDataset()
        crop_box = box.Box(visdat.getProblemDomain(level=0))
        crop_box.grow(-1)

        # Test cropping-by-levels and cropping-by-components, but don't crop
        # from the dataset we're going to continue with.
        if visdat.getNumLevels() > 1:
            crop_levels = tuple(range(0,visdat.getNumLevels()))[1:]
            cropped_visdat = visdat.cropLevels( crop_levels )
            assert( cropped_visdat.getNumLevels() ==
                    visdat.getNumLevels() -1 )
        if visdat.getNumComponents() > 1:
            #print "visdat.getNumComponents()=",visdat.getNumComponents()
            #print "visdat.getComponentNames()=",visdat.getComponentNames()
            comps = list(visdat.getComponentNames())
            #print "comps=", comps
            comps.remove (comps[0] )
            cropped_visdat = visdat.cropComponents( tuple(comps) )
            assert( cropped_visdat.getNumComponents() ==
                    visdat.getNumComponents() - 1 )

        if crop_box.getNumCells() > 100:
            return c.reader.getVisualizableDataset().crop( crop_box )
        else:
            return visdat

    def runTests( self, c ):

        #
        # Test case A3.2.1.
        #
        self.c.misc.setDebugLevel( 4 )
        c.reader.loadHDF5( os.environ['CHOMBOVIS_HOME'] + \
            '/share/ChomboVis/data/' +
            self.testname_hdf5file_dictionary[ self.testname ] )

        if self.testname == 'ebclip2d':
            self.ebclip2dTest( self.c )
            return

        if self.testname == 'ebclip3d':
            self.ebclip3dTest( self.c )
            return

        if self.testname == 'nodata':
            self.nodataTest( self.c )
            return

        if self.testname == 'nansinfs':
            self.nansinfsTest( self.c )
            return

        if self.testname == 'Reslice':
            self.resliceTest( self.c )
            return

        cropped_visdat = self.getCroppedVisDat(c)
        (temp_file,dummy) = anag_utils.g_temp_file_mgr.makeTempFile(
            'cropped', 'hdf5', create=0 )
        cropped_visdat.saveToHDF5( temp_file )
        c.reader.loadHDF5( temp_file )
        c.reader.setCurrentComponent( 0 )
        
        def sqr(x): return x*x
        c.reader.defineNewComponent( 'comp0_squared', sqr,
                                     (c.reader.getComponentNames()[0],) )

        self.dataApiTest( self.c, self.c.reader.getVisualizableDataset() )

        # Special lightweight tests for node centering and particles:
        if self.testname == '2dNode':
            self.nodeCenteringTest( self.c )
            return
        if self.testname == '3dParticles':
            # FIXME: Glyph orientation depends what the optimization mode is
            # set to!
            c.misc.setOptimizationMode( 'speed' )

            self.particlesTest( self.c )
            self.annotationTest( self.c )

            names = c.reader.getComponentNames()
            c.reader.setCurrentComponent(names[0])
            c.iso.toggleVisibility(1) # No geometry to put glyphs on otherwise
            c.iso.setMax(-0.00908282)
            c.cmap.setCmappedRangeMax( 0.75 )

            c.iso.showGlyphs()
            glyphs = c.iso.getGlyphs()
            self.glyphTest( glyphs, c )
            glyphs.setDecimationFactor(1.0)
            glyphs.setGlyphType('arrow')
            c.iso.toggleVisibility(0)

            # Gonna test grid coloring while we're on a good dataset for that...
            c.reader.setVisibleLevelMin(1)
            c.reader.setVisibleLevelMax(2)
            outline_colors  = ( (1,.1,.1),(1,.2,.2),(1,.6,.6) )
            solidbox_colors = ( (.1,.1,1),(.2,.2,1),(.6,.6,1) )
            c.grid.setDetail( 'Bounding boxes' )
            c.grid.toggleSolidBoxes( 1 )
            c.grid.setSolidBoxOpacity( 0.4 )
            c.grid.setSolidBoxShrinkageFactor( -0.2 )
            c.grid.setColorSolidsByLevel(1,prescribed_colors=solidbox_colors)
            c.grid.setColorOutlinesByLevel(1,prescribed_colors=outline_colors)

            self.volumeTest( self.c )

            return

        anag_utils.info( "Grid Test:" )
        self.gridTest( self.c )
        anag_utils.info( "Clipping Test:" )
        self.clipTest( self.c )
        anag_utils.info( "Iso Test:" )
        self.isoTest( self.c )
        anag_utils.info( "Stream Test:" )
        if self.c.reader.is2DMode():
            self.streamTest2D( self.c )
        else:
            self.streamTest3D( self.c )
        anag_utils.info( "Cmap Test:" )
        self.cmapTest( self.c )
        anag_utils.info( "Slice Test:" )
        self.sliceTest( self.c )
        anag_utils.info( "Reader Test:" )
        self.readerTest( self.c )
        if not self.c.reader.is2DMode():
            anag_utils.info( "EB Test:" )
            self.ebTest( self.c )
        anag_utils.info( "Misc Test:" )
        self.miscTest( self.c )

    #
    # Test case A3.3.2.
    #
    def gridTest( self, c ):
        """ Grids API test """
        c.grid.setDetail( 'Bounding boxes' )
        c.grid.toggleDomainBox( on_off = 0 )
        c.grid.setColor( rgb=(0.1,0.2,0.3))
        c.grid.toggleClipping( on_off=1 )
        c.grid.showGUI()
        c.grid.setLineWidth( 2 )
        # That doesn't have any effect anymore.  But it does if you put it
        # right after the call to tkUpdate() in miscTest().  In a small
        # program I'm unable to replicate this problem.

    #
    # Test case A3.3.3.
    #
    def isoTest( self, c ):
        """ Iso surfaces API test. """
        c.iso.toggleVisibility( 1 )
        c.iso.toggleLOD( 0 )
        c.iso.toggleLOD( 1 )
        if c.reader.is2DMode():
            c.iso.setMin( -0.05 )
            c.iso.setMax( -0.01 )
            c.iso.setNum( 12 )
            c.iso.setLineWidth(2)
        else:
            c.iso.setMin( 0.15 )
            c.iso.setMax( 0.90 )
            c.iso.setNum( 3 )
            c.iso.toggleClipping( on_off=1 )

        if c.reader.is2DMode():
            c.iso.toggleShadeByValue( 0 )
            c.iso.setConstantColor( rgb = (1.0,0.5,0.5) )
        else:
            c.iso.setCrossColoration( 'zmomentum-0' )
        c.iso.setOpacity( 0.7 )
        c.iso.showGUI()
#       c.iso.withdrawGUI()


    #
    # Test case A3.3.4 (part 1).
    #
    def streamTest3D( self, c ):
        c.stream.setComponentMapU( 'fraction-0' )
        c.stream.setComponentMapV( 'density-0' )
        c.stream.setComponentMapW( 'xmomentum-0' )
        c.stream.setSeeds( n_seeds=8, xy_rotation=-52, xz_rotation=38,
              x_translation=10.2, y_translation=29.1, z_translation=47 )
        c.stream.setIntegrateForward( on_off = 0 )
        c.stream.setMaxPointsPerLine( 543 )
        c.stream.setRelativeStepsize( 0.314 )
        c.stream.setFieldThreshold( 4.5e-07 )
        c.stream.setMaxLevel( 1 )
        c.stream.setColor( rgb = (0.5,1.0,0.0) )
        c.stream.setLineWidth(9)
        c.stream.showTubes()
        c.stream.hideTubes()
        c.stream.showTubes()
        c.stream.setTubeResolution(3)
        c.stream.setSeedSize(0)
        c.stream.setSeedSize(10)
        c.stream.setAlgorithm( 'Clipped Step' )
        c.stream.showGUI()
        c.stream.withdrawGUI()

        c.stream.showGlyphs()
        glyphs = c.stream.getGlyphs()
        self.glyphTest( glyphs, c )

    #
    # Test case A3.3.4 (part 2).
    #
    def streamTest2D( self, c ):
        c.stream.setComponentMapU( 'density' )
        c.stream.setComponentMapV( 'energy-density' )
        c.stream.setSeeds( n_seeds=14, xy_rotation=-35,
                           x_translation=0.36, y_translation=0.48 )
        c.stream.setIntegrateForward( on_off = 1 )
        c.stream.setMaxPointsPerLine( 543 )
        c.stream.setRelativeStepsize( 0.314 )
        c.stream.setFieldThreshold( 4.5e-07 )
        c.stream.setMaxLevel( 0 )
        c.stream.setColor( rgb = (0.9,0.0,0.4) )
        c.stream.setAlgorithm( 'Fixed Step' )
        c.stream.setLineWidth(2)
        c.stream.setSeedSize(0)
        c.stream.setSeedSize(10)
        c.stream.showGUI()
        c.stream.withdrawGUI()

    #
    # Test case A3.3.6.
    #
    def ebTest( self, c ):
        c.eb.setBlankCoveredCells(1)
        cur_comp = c.reader.getCurrentComponent()
        c.reader.setCurrentComponent( vtk_data.g_ebBlankCoveredName )
            # Used to dump core -- gotta make sure it never does again.
        c.reader.setCurrentComponent( cur_comp )
        c.eb.toggleVisibility( on_off = 1 )
        c.eb.toggleShadeByValue( on_off = 0 )
        c.eb.toggleLOD(0)
        c.eb.toggleLOD(1)
        c.eb.setColor( rgb=(0,0,1) )
        if c.reader.is2DMode():
            c.eb.selectBoundary( boundary_num=0, on_off=1 )
        else:
            c.eb.selectBoundary( boundary_num=0, on_off=1 )
            c.eb.selectBoundary( boundary_num=1, on_off=1 )
            c.eb.toggleClipping( on_off=1 )
        c.eb.setCapping( 1 )
        c.eb.setNormalOutward( 1 )
        c.eb.setOpacity( 0.7 )
        c.eb.setLineWidth(2)
        c.eb.showGUI()
#       c.eb.withdrawGUI()

        c.eb.showGlyphs()
        g=c.eb.getGlyphs()
        g.setDecimationFactor(2.0)
        g.setScaleFactor(0.4)

    def ebclip2dTest( self, c ):
        self.testname = 'ebclip2d'
        c.reader.loadHDF5( os.environ['CHOMBOVIS_HOME'] + \
            '/share/ChomboVis/data/' +
            self.testname_hdf5file_dictionary[ self.testname ] )
        c.reader.setCurrentComponent( 0 )
        c.eb.setClippingIsovalue( 3.9 )
        c.eb.setLevelsetComponent( 0 )
        c.eb.setFluid2(1)
        c.eb.setColor((1,0,1))
        c.eb.setLineWidth(7)
        c.eb.setUseGhostCells(1)
        c.eb.toggleVisibility( 1 )
        c.eb.setBlankCoveredCells(1)

        c.iso.setCrossColoration( c.vtk_data.getComponentList()[0] )
        c.iso.setConstantColor( rgb=(0,0,1) )
        c.iso.toggleVisibility( 1 )
        c.iso.toggleClipping( 1 )

        for lev in range(c.reader.getNumLevels()):
            c.reader.showAllBoxes(lev)
            c.reader.hideAllBoxes(lev)
            c.reader.hideBox(lev,0)
            c.reader.showBox(lev,0)
            c.reader.hideBoxes(lev,(0,))
            c.reader.showBoxes(lev,(0,))
        c.reader.hideBox(0,1)


    def ebclip3dTest( self, c ):
        self.testname = 'ebclip3d'
        c.reader.loadHDF5( os.environ['CHOMBOVIS_HOME'] + \
            '/share/ChomboVis/data/' +
            self.testname_hdf5file_dictionary[ self.testname ] )
        c.reader.setCurrentComponent( 1 )
        c.eb.setFluid2(0)
        c.eb.setClippingIsovalue( 0.5 )
        c.eb.setLevelsetComponent( 0 )
        c.eb.setUseGhostCells( 1 )
        c.eb.toggleVisibility(1)
        c.eb.setBlankCoveredCells(1)
        c.eb.toggleShadeByValue(1)
        c.misc.setViewParams(
            camera_position=(-16.599, -76.449, 92.014),
            axes_camera_position=(-2.486, -7.197, 5.116),
            focal_point=(15.0, 15.0, 27.0),
            world_point=(0.0, 0.0, 0.0, 0.0),
            display_point=(0.0, 0.0, 0.0),
            view_up=(-0.401, 0.618, 0.675),
            parallel_scale=30.0,
            clipping_range=(3.162, 3198.256))
        c.cmap.setCmappedRangeMin(0.0)
        c.cmap.setCmappedRangeMax(1.0)

        c.iso.toggleVisibility( 1 )
        c.iso.setConstantColor( rgb=(1,0,1) )
        c.iso.toggleShadeByValue( 0 )
        c.iso.setMin(1.288347)
        c.iso.setNum(1)
        c.iso.setCrossColoration( c.vtk_data.getComponentList()[0] )
        c.iso.toggleClipping( 1 )

        c.reader.setVisibleLevelMax(1)

        for lev in range(c.reader.getNumLevels()):
            c.reader.showAllBoxes(lev)
            c.reader.hideAllBoxes(lev)
            c.reader.hideBox(lev,0)
            c.reader.showBox(lev,0)
            c.reader.hideBoxes(lev,(0,))
            c.reader.showBoxes(lev,(0,))


    def glyphTest( self, vtk_glyphs, c ):
        vtk_glyphs.setConstantColor((1,0,1))
        n = c.reader.getNumComponents()
        names = c.reader.getComponentNames()
        vtk_glyphs.setColoringComponent( names[1] )
        if c.reader.is2DMode():
            vtk_glyphs.setOrientationalComponents([names[n-1], names[(n-2)%n]])
        else:
            vtk_glyphs.setOrientationalComponents([names[n-1], names[(n-2)%n],
                                               names[(n-3)%n]])
        vtk_glyphs.setScaleFactor(2.0)



    #
    # Test case A3.4.4.
    #
    def clipTest( self, c ):
        c.clip.showGUI()
        c.clip.withdrawGUI()
        c.clip.defineClipPlane( direction='x', position=23,
                                altitude=20, azimuth=235 )
        c.clip.togglePlaneVisibility( 1 )

    def cmapTest( self, c ):
        c.cmap.setBlackOutliers(1)
        c.cmap.loadUserColormap( os.environ['CHOMBOVIS_HOME'] +\
                                 '/share/ChomboVis/data/reverse.cmap' )
        c.cmap.setActiveColormap( 'User file' )
        c.cmap.setOutlierColors(((0,0,0,1),(1,1,1,1)))
        minval = c.cmap.getCmappedRangeMin()
        maxval = c.cmap.getCmappedRangeMax()
        c.cmap.setCmappedRangeMin( 0.9*minval + 0.1*maxval )
        c.cmap.setCmappedRangeMax( 0.1*minval + 0.9*maxval )
        c.cmap.setBackgroundColor( rgb=(0.0,0.9,1.0) )
        assert( (0.0,0.9,1.0) == c.cmap.getBackgroundColor() )
        c.cmap.hideLegend()
        c.cmap.showGUI()
        c.cmap.withdrawGUI()

    def readerTest( self, c ):
        anag_utils.funcTrace()
        if c.reader.is2DMode():        # ebtest.2d.hdf5

            #
            # Test case A3.4.1.
            #
            c.reader.setCurrentComponent( 'comp0_squared' )
            # Produced with defineNewComponent()

            #
            # Test case A3.4.2.
            #
            c.reader.setVisibleLevelMin(0)
            assert( 0 == c.reader.getVisibleLevelMin() )
            c.reader.setVisibleLevelMax(1)
            assert( 1 == c.reader.getVisibleLevelMax() )

            sys.stderr.write( 'datum=' +
                str(c.reader.getDatum( component='x-momentum',
                level=1, box_num=14, i=1, j=3 )) + '\n' )
            extents = c.reader.getBoxExtents(level=0, box_num=0)
            sys.stderr.write( 'box extents=' 
                    + "{'i':" + str(extents['i']) + ',' +
                       "'j':" + str(extents['j']) + "}" + "\n" )
        else:                    # ebtest.3d.hdf5
            c.reader.setCurrentComponent( 'density-0' )
            c.reader.setVisibleLevelMin(1)
            assert( 1 == c.reader.getVisibleLevelMin() )
            c.reader.setVisibleLevelMax(1)
            assert( 1 == c.reader.getVisibleLevelMax() )
            sys.stderr.write( 'datum=' +
                str(c.reader.getDatum( component='fraction-0',
                level=1, box_num=1, i=0, j=10, k=11)) + '\n' )
            extents = c.reader.getBoxExtents(level=1, box_num=6)
            sys.stderr.write( 'box extents=' + 
                    "{'i':" + str(extents['i']) + ',' +
                     "'j':" + str(extents['j']) + ',' +
                     "'k':" + str(extents['k']) + "}" + "\n" )

        c.reader.useGhostCells(1)

        domain_bounds = c.reader.getDomainBounds()
        sys.stderr.write( 'c.reader.getDomainBounds()=' + str(
          (domain_bounds['x'], domain_bounds['y'], domain_bounds['z'])) + '\n' )
        sys.stderr.write( 'c.reader.is2DMode()=' + str(c.reader.is2DMode())+'\n')
        sys.stderr.write( 'components=' + str(c.reader.getComponentNames())+'\n')
        sys.stderr.write( 'num levels =' +str(c.reader.getNumberOfLevels())+'\n')
        for l in range(0, c.reader.getNumberOfLevels()):
            sys.stderr.write( 'num boxs at level ' + str(l) + '=' +
                              str(c.reader.getNumberOfBoxes(l)) + '\n' )

        c.reader.showGUI

        c.reader.withdrawGUI
        c.reader.setAnisotropicFactors(0.9, 1.05, 1.2 )


    #
    # Test case A3.3.1.
    #
    def sliceTest( self, c ):
        if not self.c.reader.is2DMode():
            c.slice.showGUI()
            c.slice.withdrawGUI()
            c.slice.setOffset( 0.181 )
            uniqizer = c.slice.newPlane( 'x' )
            assert( uniqizer == 1 )
            x_pos = 0.33 * c.reader.getDomainBounds()['x'][0] +\
                    0.67 * c.reader.getDomainBounds()['x'][1]
            c.slice.setPosition( x_pos, 'x', 1 )
            assert( c.slice.getPosition( 'x', 1 ) == x_pos )


    def volumeTest( self, c ):
        c.volume.setComponent( c.reader.getComponentNames()[0] )
#       c.volume.setMinLevel(0)
        c.volume.setMaxLevel(1)
        c.volume.setXferPoints( ((0,1.0),(160,0.99), (161,0.0)) )
        c.volume.toggleVisibility(1)


    def miscTest( self, c ):
        if c.cmd_line.getOffScreen() != 1:
            c.misc.tkUpdate()

        #
        # Test case A3.4.3.
        bg = c.cmap.getBackgroundColor()
        c.cmap.setBackgroundColor( rgb=(1,1,1) )
        c.misc.setDebugLevel( 5 )
        c.cmap.setBackgroundColor( bg )

        c.misc.setDebugLevel( 4 )

        #
        # Test case A3.4.5.
        #
        if c.reader.is2DMode():
            c.misc.setViewParams(
                camera_position=(0.48, 0.48, 2.98),
                axes_camera_position=(0.48, 0.48, 2.98),
                focal_point=(0.48, 0.48, -0.25),
                world_point=(0.48, 0.48, -0.25, 1.0),
                display_point=(199.0, 200.0, 0.17),
                view_up=(0.0, 1.0, 0.0),
                parallel_scale=0.51,
                clipping_range=(0.67, 15.64))
        else:
            c.misc.setViewParams(
                camera_position=(-40.85, -70.84, 58.11),
                axes_camera_position=(-40.85, -70.84, 58.11),
                focal_point=(22.26, 12.06, 37.38),
                world_point=(22.28, 12.10, 37.59, 1.0),
                display_point=(201.0, 200.0, 0.85),
                view_up=(-0.79, 0.60, -0.025),
                parallel_scale=30.0,
                clipping_range=(16.86, 813.49))

        # Line plot
        domainXYZ = c.reader.getDomainBounds()
        p0 = [ (domainXYZ['x'][0]+domainXYZ['x'][1])/2.0,
                domainXYZ['y'][0]*0.9 + domainXYZ['y'][1]*0.1 ]
        p1 = [ (domainXYZ['x'][0]+domainXYZ['x'][1])/2.0,
                domainXYZ['y'][0]*0.1 + domainXYZ['y'][1]*0.9 ]

        if not c.reader.is2DMode():
            p0.append( (domainXYZ['z'][0] + domainXYZ['z'][1])/2.0 )
            p1.append( (domainXYZ['z'][0] + domainXYZ['z'][1])/2.0 )
        lineplot = c.misc.makeLinePlot( 
            tuple(p0), tuple(p1), 5,
            c.reader.getNumComponents()-1, c.reader.getNumLevels()-1 )
        anag_utils.info( "lineplot(", p0, ",", p1, ")=", lineplot.getData() )

        # Generating a discrimination component.
        if c.vtk_data.getNumEBs() == 0: # No old-style EBs
            comp = c.vtk_data.getComponentList()[0]
            rmin = c.vtk_data.getRangeMin( comp )
            rmax = c.vtk_data.getRangeMin( comp )
            c.eb.registerCoveredCellsDiscriminator( comp,
                rmin*0.6 + rmax*0.4, rmin*0.4 + rmax*0.6, 0 )

        if '__covered-0' in c.reader.getComponentNames():
            anag_utils.info( "About to make databrowser, and we have "
                             "__covered-0" )
        c.misc.setDatabrowserFontFamily('Times')
        c.misc.setDatabrowserFontStyle('italic')
        c.misc.setDatabrowserFontSize(24)
        databrowser = c.misc.makeDatabrowser(
            component=c.reader.getComponentNames()[0], level=0, box_num=0 )
        c.misc.withdrawDatabrowser(
            component=c.reader.getComponentNames()[0], level=0, box_num=0 )

        #
        # Test case A3.2.2.
        #
        (temp_state_file,dummy) = anag_utils.g_temp_file_mgr.makeTempFile(
            'api_test', 'state', create=0 )

        c.misc.saveState( temp_state_file )
        for a in 'x','y','z': c.slice.toggleVisibility( on_off=0, axis=a )
        c.misc.restoreState( temp_state_file )

        Tkinter.Button( c.misc.getTopWindow(), text="test butt" ).pack()


    def annotationTest( self, c ):

        c.annotation.addNewCaption()
        c.annotation.setText( 'static2d' )
        c.annotation.setSize( 24 )
        c.annotation.setOpacity( 75 )
        for font in 'arial', 'courier', 'times':
            c.annotation.setFont( font )
        c.annotation.setItalic(1)
        c.annotation.setColor( red=1.0, green=0.9, blue=0.0 )
        c.annotation.setPosition( x=0.75, y=0.85 )

        c.annotation.addNewCaption()
        c.annotation.setMode( 'dynamic2d' )
        c.annotation.setText( 'dynamic2d' )
        c.annotation.setSize(32)
        c.annotation.setFont( 'courier' )
        c.annotation.setBold( 1 )
        c.annotation.setColor( red=1.0, green=0.1, blue=0.9 )
        c.annotation.setPosition( x=-2.312, y=3.933, z=0.080 )

        c.annotation.addNewCaption()
        c.annotation.setMode( 'dynamic3d' )
        c.annotation.setText( 'dynamic3d' )
        c.annotation.setSize(13)
        c.annotation.setColor( red=0.0, green=0.9, blue=0.9 )
        c.annotation.setPosition( x=-2.480, y=4.152, z=0.152 )
        c.annotation.setOrientation( x=30.0, y=-5.0, z=5.0 )
        
        c.misc.vtkUpdate()


    def nodataTest( self,c ):
        """
        For data file with no field data -- just BoxLayout's.
        """
        self.testname = 'nodata'
        c.reader.loadHDF5( os.environ['CHOMBOVIS_HOME'] + \
            '/share/ChomboVis/data/' +
            self.testname_hdf5file_dictionary[ self.testname ] )

        sliced_visdat = c.reader.getVisualizableDataset().slice('x',1.9)
        (temp_slice,dummy) = anag_utils.g_temp_file_mgr.makeTempFile(
            'sliced', 'hdf5', create=0 )
        sliced_visdat.saveToHDF5( temp_slice )
        c.reader.loadHDF5( temp_slice )
        c.reader.setVisibleLevelMin(1)
        c.reader.setVisibleLevelMin(0)
        c.reader.setVisibleLevelMax(1)
        self.gridTest( c )
        c.grid.setDetail( 'All cells' )
        c.grid.setColorOutlinesByLevel(1)


    def nansinfsTest( self,c ):
        """
        For data file with a bunch of NaNs and infs and -infs in it.
        """
        self.testname = 'nansinfs'
        c.reader.loadHDF5( os.environ['CHOMBOVIS_HOME'] + \
            '/share/ChomboVis/data/' +
            self.testname_hdf5file_dictionary[ self.testname ] )
        c.reader.setCurrentComponent( 2 )
        c.cmap.setBackgroundColor( (1,1,0.5) )
        c.cmap.setBlackOutliers(1)
        c.cmap.setOutlierColors( ((0,0,0,1),(1,1,1,1)) )


    def resliceTest( self,c ):
        c.misc.setDebugLevel( 4 )
        c.reader.loadHDF5( os.environ['CHOMBOVIS_HOME'] + \
            '/share/ChomboVis/data/' +
            self.testname_hdf5file_dictionary[ self.testname ] )
        c.reader.setCurrentComponent( 0 )
        c.reader.setVisibleLevelMax( c.reader.getNumLevels()-1 )
        self.gridTest(c)
        self.isoTest(c)


    def nodeCenteringTest( self, c ):
        """
        Despite the name, the main activity in this test is loading new hdf5
        files -- twice.  (The last file to be loaded is, however, the 2dNode
        one.)
        """
        c.misc.setDebugLevel( 4 )
        self.testname = '2dNode'
        c.reader.loadHDF5( os.environ['CHOMBOVIS_HOME'] + \
            '/share/ChomboVis/data/' +
            self.testname_hdf5file_dictionary[ self.testname ] )
        c.reader.setCurrentComponent( 0 )
        c.reader.setVisibleLevelMax(2)
        self.cmapTest( c )
        self.gridTest( c )
        c.grid.setDetail( 'All cells' )


    def dataApiTest( self, c, visdat, sliced_already=None ):
        """
        Fixme: gotta make this much more robust, and gotta eliminate all those
        magic numbers.
        """
        anag_utils.funcTrace()

        n_comps = c.reader.getNumberOfComponents()
        lev  = max(0, c.reader.getNumberOfLevels() - 2)
        comp = n_comps-1
        bld = visdat.getBoxLayoutData( lev, comp )
        bld_clone = bld.clone()
        bl =  visdat.getBoxLayout( lev, comp )
        assert( bl == bld.getBoxLayout() )

        anag_utils.info( "BoxLayout dx =", bl.getDx() )
        anag_utils.info( "BoxLayout origin =", bl.getOrigin() )
        anag_utils.info( "BoxLayout at level", lev, "=", bl )
        anag_utils.info( "output ghost =", visdat.getOutputGhost() )
        anag_utils.info( "data centering =", visdat.getDataCentering() )
        anag_utils.info( "dimensionality=", visdat.getDimensionality() )
        anag_utils.info( "precision=", visdat.getPrecision() )
        anag_utils.info( "component names =", visdat.getComponentNames() )
        anag_utils.info( "# components =", visdat.getNumComponents() )
        anag_utils.info( "origin =", visdat.getOrigin() )
        for l in range(0, c.reader.getNumberOfLevels()):
            anag_utils.info( "Problem domain at level", l, "=",
                             visdat.getProblemDomain(l) )
            anag_utils.info( "Dx at level", l, "=", visdat.getDx(l) )
            anag_utils.info( "Dt at level", l, "=", visdat.getDt(l) )
            anag_utils.info( "Time at level", l, "=", visdat.getTime(l) )

        box0 = bl[0]
        anag_utils.info( "Box 0 =", box0 )

        #
        # Testing summary statistics.
        #
        anag_utils.info( "Summary statistics before lots of operations:" )
        self.dataApiTestSummaryStatistics( bld )

        #
        # Testing pointwise operators.
        #
        random.seed(1)
        for op in (
              bld.abs,
              bld.__iadd__, bld.__isub__, bld.__imul__,  bld.__idiv__,
              bld.exp,bld.__ipow__,bld.log,
              bld.sin, bld.cos, bld.tan, bld.asin, bld.acos, bld.atan,
              bld.sinh, bld.cosh, bld.tanh, bld.asinh, bld.acosh, bld.atanh,
              bld.setDatum,
              bld.__add__, bld.__sub__, bld.__mul__,  bld.__div__,
              bld.__radd__, bld.__rsub__, bld.__rmul__,
              bld.__rdiv__, bld.__pow__, bld.__rpow__
          ):
            # Apply op.  Some op's take no args, some take one, some more.
            # We find out by trying zero args and, if that throws an exception,
            # we try one arg, and if that fails, try more args.
            try:
                self.dataApiTestApplicator( op, bld, bl, (), lev, comp )
            except:
                self.dataApiTestApplicator( op, bld, bl, (2.5,), lev, comp )
            try:
                self.dataApiTestApplicator( op, bld, bl, (bld,), lev, comp )
            except:
                pass

            #anag_utils.info( "Summary statistics after", str(op).split()[2] )

        anag_utils.info( "Summary statistics after all these operations:" )
        self.dataApiTestSummaryStatistics( bld )
        anag_utils.info( "Summary statistics for bld_clone (should match up)"
            "with summary statistics before the operations:" )
        self.dataApiTestSummaryStatistics( bld_clone )

        if (not c.reader.is2DMode()) and (not sliced_already):
            for axis in 'x', 'y', 'z':
                axis_position = (  c.reader.getDomainBounds()[axis][0]
                                 + c.reader.getDomainBounds()[axis][1] )/2.01
                anag_utils.info( "Slicing axis", axis,
                                 "at position", axis_position )
                sliced_visdat = visdat.slice(axis, axis_position)
                self.dataApiTest( c, sliced_visdat, sliced_already=1 )
                (temp_slice,dummy) = anag_utils.g_temp_file_mgr.makeTempFile(
                    'sliced', 'hdf5', create=0 )
                sliced_visdat.saveToHDF5( temp_slice )


    def dataApiTestSummaryStatistics( self, bld ):
        anag_utils.funcTrace()
        bld.clamp(0)
        anag_utils.info( "min(clamped on 0)=", bld.min() )
        anag_utils.info( "max(clamped on 0)=", bld.max() )
        anag_utils.info( "sum(clamped on 0)=", bld.sum() )
        anag_utils.info( "sumOfSquares(clamped on 0)=", bld.sumOfSquares() )
        anag_utils.info( "stdDev(clamped on 0)=", bld.stdDev() )
        anag_utils.info( "getNumCells(clamped on 0)=", bld.getNumCells() )

        bld.clamp( tuple(range(0, bld.getNumBoxes())) )
        anag_utils.info( "min(clamped on all)=", bld.min() )
        anag_utils.info( "max(clamped on all)=", bld.max() )
        anag_utils.info( "sum(clamped on all)=", bld.sum() )
        anag_utils.info( "sumOfSquares(clamped on all)=", bld.sumOfSquares() )
        anag_utils.info( "stdDev(clamped on all)=", bld.stdDev() )
        anag_utils.info( "getNumCells(clamped on all)=", bld.getNumCells() )
        anag_utils.info( "getNumBoxes(clamped on all)=", bld.getNumBoxes() )

        bld.unclamp()
        anag_utils.info( "min()=", bld.min() )
        anag_utils.info( "max()=", bld.max() )
        anag_utils.info( "sum()=", bld.sum() )
        anag_utils.info( "sumOfSquares()=", bld.sumOfSquares() )
        anag_utils.info( "stdDev()=", bld.stdDev() )
        anag_utils.info( "getNumCells()=", bld.getNumCells() )
        anag_utils.info( "getNumBoxes()=", bld.getNumBoxes() )


        # Clamp and crop on a general rectangle -- stretch and shrink box 0.
        cropbox = bld.getBoxLayout()[0]
        locorner = list(cropbox[0])
        hicorner = list(cropbox[1])
        locorner[0] += (hicorner[0]-locorner[0])/2
        hicorner[1] += (hicorner[1]-locorner[1])/2
        cropbox = box.Box((tuple(locorner), tuple(hicorner)))
        bld.clamp( cropbox )
        tot = bld.sum()
        bld.unclamp()
        bld_copy_for_cropping = bld.clone()
        bld_copy_for_cropping.crop( cropbox )
        crop_tot = bld_copy_for_cropping.sum()
        anag_utils.info( "bld.sum(clamped on", cropbox, ")=", tot )
        anag_utils.info( "bld.sum(cropped to", cropbox, ")=", crop_tot )
        

    def dataApiTestApplicator( self, op, bld, bl, args, level, comp ):
        """
        Apply operator op to args, and report results before and after.
        Arg bld is the BoxLayoutData on which op is a function.
        """
        anag_utils.funcTrace()

        rand_box_num = random.randrange(0, bld.getNumBoxes())
        box = bl[rand_box_num]
        rand_i = random.randrange( box[0][0], box[1][0]+1 ) - box[0][0]
        rand_j = random.randrange( box[0][1], box[1][1]+1 ) - box[0][1]
        if len(box[0]) == 3:
            rand_k = random.randrange( box[0][2], box[1][2]+1 ) - box[0][2]
        else:
            rand_k = 0
        
        bld.clamp( rand_box_num )
        anag_utils.info( "Clamped on box#", rand_box_num )

        # Ensure we don't try undefined operations, or operations that will
        # give us extremely large or small results.
        if   op in (bld.log, bld.__idiv__):
            bld.abs()
            bld += 1.0
        elif op in (bld.asin, bld.acos, bld.atan, bld.exp, bld.sinh, bld.cosh,
                    bld.acosh, bld.atanh, bld.__ipow__,
                    bld.__rdiv__, bld.__pow__, bld.__rpow__):
            # map domain to [0,1)
            bld -= bld.min()
            bld += 0.75
            bld /= bld.max()*1.001
            if op == bld.acosh:
                bld += 1.0

        if (op == bld.setDatum) and (len(args)==1):
            args = args + (rand_box_num, rand_i,rand_j,rand_k)

        old_value = bld.getDatum( rand_box_num, rand_i,rand_j,rand_k )
        retval = apply( op, args ) # Doing this first, so if fails 
                                   # we don't print stuff.
        anag_utils.info( "Value at level", level, ", comp", comp,
            ", box", rand_box_num, ",(i,j,k)=",(rand_i,rand_j,rand_k),"=",
            old_value )
        
        # Python 2.1/2.2 interoperability:
        if str(op).split()[0] == '<bound': op_str_elem = 2
        else:                              op_str_elem = 1
        if len(args)==1 and isinstance(args[0],box_layout_data.BoxLayoutData):
            anag_utils.info( "Applying", str(op).split()[op_str_elem], "to",
                             str(args[0]).split()[0])
        else:
            anag_utils.info( "Applying", str(op).split()[op_str_elem],"to",args)
        if (retval==None) or (retval == bld):
            anag_utils.info( "Value is now",
                 bld.getDatum( rand_box_num, rand_i,rand_j,rand_k ))
        else: # Covers case of non-in-place arithmetic.
            anag_utils.info( "Value is now",
                 retval.getDatum( rand_box_num, rand_i,rand_j,rand_k ))
        bld.unclamp()
        bld.getFArrayAsTupleMatrix(0) # Real NumPy test yet to come
        bld.getFArrayAsTupleMatrix(0) # Just to see if releasing the FAB caused
                                      # any memory corruption.


    def particlesTest( self, c ):
        """
        Quick test on data with particles.
        """
        anag_utils.info( 'Number of particles =', c.particles.getNumParticles())
        anag_utils.info( 'Number of particle components =',
                         c.particles.getNumParticleComponents() )
        anag_utils.info( 'Particle component names=',
                         c.particles.getComponentNames() )
        c.reader.setCurrentComponent( c.reader.getNumberOfComponents()-1 )
        c.reader.setVisibleLevelMin(1)
        c.particles.showGUI()
        c.particles.withdrawGUI()
        c.slice.toggleVisibility( on_off=0, axis='z' )
        c.slice.toggleVisibility( on_off=0, axis='y' )
        c.particles.setFilteringComponent( 'position_x' )
        c.particles.setFilteringComponentMinAndMax( -2.12, -2.08 )
        c.particles.setMarkerType( 'cones' )
        c.particles.setGlyph3dResolution( 6 )
        c.particles.setPositionalComponents( ('position_x','position_y','mass'))
        c.particles.setGlyphScaleAndColorMode(
            vtk_particles.g_glyph_scale_modes.none )
        c.particles.setGlyphScaleAndColorMode(
            vtk_particles.g_glyph_scale_modes.colorOnly )
        c.particles.setGlyphScaleAndColorMode(
            vtk_particles.g_glyph_scale_modes.colorAndScale )
        c.particles.setGlyphScalingComponent( 'charge' )
        c.particles.setGlyphOrientationComponents(
            ('position_x', 'position_y', 'position_z') )
        c.misc.setViewParams(
camera_position=(-2.1290107111601637, 4.2344260826246956, 2.0291915377360579),
axes_camera_position=(-0.20273527521853443, 0.18271377707001973, 9.1702515456506823),
focal_point=(-2.0863687098026276, 4.1959952712059021, 0.10038127796724439),
world_point=(-2.096698522567749, 4.1964163780212402, 0.10014444589614868, 1.0),
display_point=(349.0, 150.0, 0.67521488666534424),
view_up=(0.041123109212778355, 0.99897351269868573, -0.018995020798161271),
parallel_scale=0.799999952316,
clipping_range=(1.1223348198606635, 2.9505801390739732))
        c.particles.setOpacity( 0.5 )
        c.cmap.showLegend()

t = Test()
sys.exit(0)
