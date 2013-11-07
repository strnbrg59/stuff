/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** This software is copyright (C) by the Lawrence Berkeley
** National Laboratory.  Permission is granted to reproduce
** this software for non-commercial purposes provided that
** this notice is left intact.
** 
** It is acknowledged that the U.S. Government has rights to
** this software under Contract DE-AC03-765F00098 between
** the U.S. Department of Energy and the University of
** California.
**
** This software is provided as a professional and academic
** contribution for joint exchange.  Thus it is experimental,
** is provided ``as is'', with no warranties of any kind
** whatsoever, no support, no promise of updates, or printed
** documentation.  By using this software, you acknowledge
** that the Lawrence Berkeley National Laboratory and
** Regents of the University of California shall have no
** liability with respect to the infringement of other
** copyrights by any part of this software.
**
*/
// Author: Ted Sternberg

#include <iostream>
#include <cstdlib>
#include "../utils/Trace.h"
#include "../utils/cmdline.h"
#include "../utils/StatusCodes.h"
#include "ChomboBridge.h"

using std::cerr;
using std::cout;
using std::endl;

int
main( int argc, char * argv[] )
{
    Cmdline cfg( argc, argv );
    Trace t("main()");

  {
    cerr << '\n';
    t.Info( "Initializing ChomboBridge from hdf5 file..." );
    ChomboBridge bridge( cfg.Infile().c_str() );
    bridge.IntVectTest( 17 );

    // The implementation of this, in bridge_module.cpp, isn't right:
    // bridge.amrtools_ReadAMRHierarchyHDF5( cfg.Infile().c_str() );
  }

  // Serious problem.
  // You get duplicate symbols (from the Chombo layer) if you try to have both
  // 2- and a 3-dimensional Chombo libraries in play at the same time.
  // The names of the libbridge functions are different all right, the trouble
  // is the Chombo functions *they* call.  
  // Surprisingly, this is a problem even if I let the 2D ChomboBridge go out
  // of scope before trying to construct a 3D one.  Apparently the Chombo
  // symbols don't get completely cleared out of there.  There's a hint of that
  // in the libltdl documentation (http://asis.web.cern.ch/asis/products/GNU.DVP/libtool-1.3.2/libtool_10.html)
  // where it says "Unresolved symbols in the module are resolved
  // using its dependency libraries (not implemented yet) and previously
  // dlopened modules."
  // We do load the correct version (2D vs 3D) of libbridge.  The problem is
  // the Chombo functions it, in turn calls.  The following code will
  // demonstrate the effect.
  {
    cerr << '\n';
    t.Info( "Initializing ChomboBridge from explicit (dim,precision)..." );
    ChomboBridge bridge( 3, 2 );
    bridge.IntVectTest( 17 );
  }
  // We could overcome this problem by putting Chombo into a namespace that
  // includes CH_SPACEDIM in its name, but there's no easy way to do that
  // without editing (and checking in a new version of) every single file in the
  // ANAG Chombo source tree.
}
