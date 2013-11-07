//
// This file defines the structures, pointers to which are passed
// to the module functions (declared in bridge_module.h).  All those
// functions are "void f(void *)" so that data_access/LtdlModule::Apply()
// can call them in a simple way.
//
// These structures are thus the interface between data_access/ChomboBridge.cpp
// and chombo_bridge/bridge_module.h.  ChomboBridge.h, however, shouldn't use
// these structures; ChomboBridge is not a module, and its clients are
// normal ChomboVis classes.
//

struct ReadAMRHierarchyArg
{
    char const * hdf5filename;
    int          n_levels;
};
