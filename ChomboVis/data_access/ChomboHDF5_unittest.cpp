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

#include "Intvect.cpp"
#include "Box.cpp"

#include <iostream>
#include <sstream>
#include <cstdlib>
#include "ChomboHDF5.h"
#include "ChomboHDF5_subclasses.h"
#include "../utils/Timer.h"
#include "../utils/BoxFinder.h"
#include "../utils/Trace.h"
#include "../utils/cmdline.h"
#include "../utils/StatusCodes.h"
#include "../utils/HeteroMap.h"
#include <boost/shared_ptr.hpp>

using std::cerr;
using std::cout;
using std::endl;


template<class REAL_T> int
templatizedMain( Cmdline const * cfg, ChomboHDF5FileType filetype )
{
    Trace t("templatizedMain()");
    Status status;

    boost::shared_ptr<ChomboHDF5<REAL_T> > infile;
    if( filetype == old_chombo )
    {
        t.Info( "filetype = old_chombo" );
        infile.reset( new OldChomboHDF5<REAL_T>( cfg->Infile().c_str(),
                                                 &status ) );
    } else
    if( filetype == new_eb_chombo )
    {
        t.Info( "filetype = new_eb_chombo" );
        t.Info( "Not ready for prime time.  EBChomboHDF5 ctor scans in a few things\n"
                "but then gags when it gets to 'Ghost'.  We'll return to this later.\n"
                "Exiting now." );
        exit(0);
        infile.reset( new EBChomboHDF5<REAL_T>( cfg->Infile().c_str(),
                                                &status ) );
    } else
    {
        t.FatalError( "Illegal filetype" );
    }

    if( status != STATUS_OK )
    {
        cerr << StatusName( status ) << endl;
        t.FatalError( "%s.", StatusName( status ).c_str() );
    }

    GlobalMetadata<REAL_T> globalMeta( infile->GetGlobalMetadata() );

    cout << "GlobalMetadata: \n" << globalMeta << endl;
    vector<LevelMetadata<REAL_T> > levelsMeta;
    cout << "Levels metadata:" << endl;
    for( int lev=0; lev<globalMeta.m_numLevels; ++lev )
    {
        levelsMeta.push_back( infile->GetLevelMetadata( lev ) );
        cout << "  Level " << lev << " metadata:\n" << levelsMeta[lev] << endl;

        cout << "A few items from the fab: ";

        REAL_T * farray = new REAL_T[ infile->GetHyperslabSize(lev,0) ];
        infile->ReadFArray( farray, lev, 0, 0 );
        for( int j=0;j<infile->GetHyperslabSize(lev,0);++j )
        {
            cout << farray[j] << " ";
        }
        delete [] farray;

        cout << endl;
    }

    ParticleMetadata particleMetadata( infile->GetParticleMetadata() );
    for( int i=0;i<particleMetadata.m_numParticles;++i )
    {
        vector<REAL_T> coords( infile->ReadParticleCoordinates(i) );
        for( unsigned j=0; j<coords.size(); ++j )
        {
            cout << coords[j] << " ";
        }
        cout << endl;
    }

    return 0;
}

int
main( int argc, char * argv[] )
{
    Cmdline cfg( argc, argv );
    
    string ld_library_path( getenv("LD_LIBRARY_PATH") );
    ld_library_path = string("../lib:") + ld_library_path;
    setenv( "LD_LIBRARY_PATH", ld_library_path.c_str(), 1 );

    Trace t("main()");

    Status status;
    int dummy;
    int precision;
    t.Info( "*** infile=%s", cfg.Infile().c_str() );
    ChomboHDF5FileType filetype;
    status = ChomboHDF5DiscoverMetaparameters(
        cfg.Infile().c_str(), &dummy, &precision, &filetype );
    if( status != STATUS_OK )
    {
        t.FatalError( "%s", StatusName( status ).c_str() );
    }

    do
    {
        if( precision == 1 )
        {
            templatizedMain<float>( &cfg, filetype );
        } else
        {
            templatizedMain<double>( &cfg, filetype );
        }
    } while( cfg.MemoryLeakTest() );

    return 0;
}
