"""
Example of how to link to a user-supplied C++ function, that takes as arguments
raw pointers to FArrays.
"""

import os
import os.path
import sys
import visualizable_dataset
import module_maker
import anag_utils

g_share = os.getenv('CHOMBOVIS_HOME') + '/share/ChomboVis'
g_lib = os.getenv('CHOMBOVIS_HOME') + '/lib/ChomboVis'
g_ex = '/examples/module_maker'

for precision in 'float', 'double':
    
    hdf5filename = g_share \
                 + '/data/test.3d.' + precision + '.hdf5'
    if not os.path.isfile( hdf5filename ):
        anag_utils.fatal( 'Cannot find', hdf5filename + '.', '  Maybe go into',
            g_share + '/data and type "make"' )

    print "*** Loading ", hdf5filename, "***"

    v=visualizable_dataset.VisualizableDataset( hdf5filename = hdf5filename )

    bld_0=v.getBoxLayoutData(2,0)
    bld_1=v.getBoxLayoutData(2,1)
    print "bld_0.sum()=", bld_0.sum()

    #
    print "    *** Function of one ", precision, "* argument: ***"
    #    
    lib_name = 'userfunc_lib_' + precision
    cpp_name = g_share + g_ex + '/userfunc_monadic_' + precision
    if not os.path.isfile( g_share + g_ex + '/lib' + lib_name + '.a' ):
        anag_utils.fatal( 'Go into', g_share+g_ex, 'and type "make"' )
    m1 = module_maker.makeModule(
        cpp_name + '.cpp',
        extra_CFLAGS='-I' + g_share + g_ex,
        extra_LDFLAGS='-L' + g_share + g_ex + ' -l' + lib_name \
                     +' -L' + g_lib + ' -lchombovis_data', save_cpp=1)
    box_extents = bld_0.getBoxLayout()[0][0] + bld_0.getBoxLayout()[0][1]
    m1.F( box_extents, bld_0.getFArray(0) )
    print "bld_0.sum()=", bld_0.sum()
    

    #
    print "    *** Function of two ", precision, "* arguments: ***"
    #
    cpp_name = g_share + g_ex + '/userfunc_diadic_' + precision
    if not os.path.isfile( g_share + g_ex + '/lib' + lib_name + '.a'):
        anag_utils.fatal( 'Go into', g_share+g_ex, 'and type "make"' )
    m2 = module_maker.makeModule(
        cpp_name + '.cpp',
        extra_CFLAGS='-I' + g_share + g_ex,
        extra_LDFLAGS='-L' + g_share + g_ex + ' -l' + lib_name \
                     +' -L' + g_lib + ' -lchombovis_data' )
    box_extents = bld_0.getBoxLayout()[0][0] + bld_0.getBoxLayout()[0][1]
    m2.F( box_extents, bld_0.getFArray(0), bld_1.getFArray(0),  )
    print "bld_1.sum()=", bld_1.sum()

    
    v = None
    print "\n\n"

sys.exit(0)
