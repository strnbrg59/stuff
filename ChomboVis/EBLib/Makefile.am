#
# The library we build here is only needed if we also build libChomboVTK.so.
#
if VTK
    eb_LTLIBRARIES = libEBSurfaceSingleCell.la
    libEBSurfaceSingleCell_la_SOURCES = \
        CellIntersection.C \
        ConnectedHexahedron.C \
        EB2DVolumeCalculator.C \
        EB3DVolumeCalculator.C \
        EBCache.C \
        EBCapper.C \
        EBFindPlane.C \
        EBSingleCell.C \
        EBVolumeCalculator.C \
        Hexahedron.C \
        KIJVector.C \
        LineSegment.C \
        Plane.C \
        Point.C \
        CVPolygon.C \
        Tetrahedron.C
	
    ebdir = $(pkglibdir)
    EXTRA_DIST = *.h *.C
	
    AM_CPPFLAGS = -Wall -Wno-deprecated
endif VTK
