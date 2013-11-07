#include <iostream>
#include <string>

// Chombo headers
#include "IntVect.H"
#include "Box.H"
#include "AMRIO.H"

#include "bridge_module.h"


// Interesting: you need either 'extern "C"' or EXPORT (or both), to
// get the proper function name into the library.

void EXPORT(intvecttest)( int * dummy )
{
    std::cerr << "intvecttest(), " << CH_SPACEDIM << "d version\n";

    int ivVals[3] = {817,818,823};
    IntVect iv( ivVals );
    std::cerr << "iv=" << iv << '\n';
}

void EXPORT(amrtools_ReadAMRHierarchyHDF5)( ReadAMRHierarchyArg const * args )
{
    Vector<DisjointBoxLayout> vectGridsin;
    Vector<LevelData<FArrayBox>* > dataPtrsin;
    for( int lev=0; lev<args->n_levels; ++lev )
    {
        dataPtrsin.push_back( new LevelData<FArrayBox> );
    }
    Box domainin;
    Vector<int> refRatioin;
    int numlevelsin;

    ReadAMRHierarchyHDF5(std::string(args->hdf5filename), vectGridsin,
                         dataPtrsin, domainin, refRatioin, numlevelsin);

    // clean up
    for( int lev=0; lev<args->n_levels; ++lev )
    {
        delete dataPtrsin[lev];
    }
}
