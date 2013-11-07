#include "ChomboBridge.h"
#include "LtdlModule.h"
#include "../utils/Trace.h"
#include "../utils/StatusCodes.h"
#include "ChomboHDF5.h"
#include "ChomboHDF5_subclasses.h"
#include <cassert>
#include <iostream>
using std::cout; using std::cerr;

#include "../chombo_bridge/void_pointers.h"
//#include "VisualizableDataset.h"


/** Based on dimensionality of hdf5file, loads appropriate library
    from chombo_bridge directory (that wraps a corresponding Chombo library).
*/
ChomboBridge::ChomboBridge( char const * hdf5filename )
  : m_module(0),
    m_dim(0),
    m_precision(0)
{
    Trace t("ChomboBridge ctor");
    if( ++s_count > 1 )
    {
        t.Warning( "Constructed more than one ChomboBridge object.  Could "
            "result in duplicate (and conflicting) symbols.");
    }

    Status status;
    int dim;
    int precision;

    ChomboHDF5FileType filetype;
    status = ChomboHDF5DiscoverMetaparameters(
        hdf5filename, &dim, &precision, &filetype );
    if( status != STATUS_OK )
    {
        t.FatalError( "%s", StatusName( status ).c_str() );
    }
    assert( dim==2 || dim==3 );


    t.Info() << hdf5filename << ": dim=" << dim
         << ", precision=" << precision
         << ", filetype=" << filetype << '\n';
    m_dim = dim;
    m_precision = precision;


    if( dim == 2 )
    {
        m_module = new LtdlModule( "libbridge2" );
    } else
    {
        m_module = new LtdlModule( "libbridge3" );
    }
}


/** Uses arg dim to figure out which module to load.
*/
ChomboBridge::ChomboBridge( int dim, int precision )
  : m_module(0),
    m_dim(dim),
    m_precision(precision)
{
    Trace t("ChomboBridge ctor");

    assert( precision==1  ||  precision==2 ); // Not used yet, though
    if( ++s_count > 1 )
    {
        t.Warning( "Constructed more than one ChomboBridge object.  Could "
            "result in duplicate (and conflicting) symbols.");
    }

    if( dim == 2 )
    {
        m_module = new LtdlModule( "libbridge2" );
    } else
    {
        m_module = new LtdlModule( "libbridge3" );
    }
}


ChomboBridge::~ChomboBridge()
{
    --s_count;
    delete m_module;
}

int ChomboBridge::s_count = 0;

//
// Chombo-using functions from here on:
//

int
ChomboBridge::IntVectTest( int i )
{
    assert( m_module->HasSymbol( "intvecttest" ) );
    m_module->Apply( "intvecttest", &i );
    return i;
}


int
ChomboBridge::amrtools_ReadAMRHierarchyHDF5( char const * hdf5filename )
{
    assert( m_module->HasSymbol( "amrtools_ReadAMRHierarchyHDF5" ) );

    ReadAMRHierarchyArg arg;
    arg.hdf5filename = hdf5filename;

    // It would be nice if we didn't need to tell Chombo how many levels the
    // dataset has.
/*
    if( m_precision == 1 )
    {
        VisualizableDataset<float> visdat( hdf5filename );
        arg.n_levels = visdat.GetNumLevels();
    } else
    {
        VisualizableDataset<double> visdat( hdf5filename );
        arg.n_levels = visdat.GetNumLevels();
    }
*/

    arg.n_levels = 3;
    m_module->Apply( "amrtools_ReadAMRHierarchyHDF5", &arg );

    return 0;
}
