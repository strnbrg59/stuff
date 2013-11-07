#include "void_pointers.h"

extern "C" {

#if   CH_SPACEDIM == 2
#define EXPORT(symbol)  libbridge2_LTX_ ## symbol
#elif CH_SPACEDIM == 3
#define EXPORT(symbol)  libbridge3_LTX_ ## symbol
#else
Whoa!  CH_SPACEDIM either illegal or undefined.
#endif

void EXPORT(intvecttest)( int * );

void EXPORT(amrtools_ReadAMRHierarchyHDF5)( ReadAMRHierarchyArg const * args );

} // extern "C"
