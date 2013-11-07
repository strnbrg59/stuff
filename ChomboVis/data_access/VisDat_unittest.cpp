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

// Putting these up here to avoid using a template specialization before its
// definition.
#include "Intvect.h"
#include "Box.h"
#include "FAB.h"
#include "BoxLayout.h"
#include "BoxLayoutData.h"

#include <iostream>
#include <sstream>
#include <cstdlib>
#include "VisualizableDataset.h"
#include "../utils/Timer.h"
#include "../utils/BoxFinder.h"
#include "../utils/Trace.h"
#include "../utils/cmdline.h"
#include "../utils/StatusCodes.h"
#include "../utils/HeteroMap.h"
using std::cerr;
using std::cout;
using std::endl;

class VectorFunctorBase;

template<class REAL_T> int
templatizedMain( Cmdline const * cfg )
{
    Trace t("templatizedMain()");

    boost::shared_ptr<VisualizableDataset<REAL_T> > visdat(
        new VisualizableDataset<REAL_T>( cfg->Infile() ));

    // Find enclosing boxes.
    int levelFound;
    int const iCoords[3] = {14,19,19};
    REAL_T const rCoords[3] = {-2.14, 4.285, 0.18};
    cerr << "visdat->GetEnclosingBoxNum(2, {14,19,19})="
         << visdat->GetEnclosingBoxNum( 2, &levelFound, iCoords, true ) << endl;
    cerr << "visdat->GetEnclosingBoxNumXYZ(2, {-2.14, 4.285, 0.18})="
         << visdat->GetEnclosingBoxNumXYZ( 2, &levelFound, rCoords, true )
         << endl;
    cerr << "visdat->GetEnclosingBoxNumXYZ(0, {-2.14, 4.285, 0.18})="
         << visdat->GetEnclosingBoxNumXYZ( 0, &levelFound, rCoords, true )
         << endl;
    int b(-10101);
    Intvect foundCell(
        visdat->GetEnclosingCellXYZ( 2, &levelFound, &b, rCoords, true ) );
    cerr << "visdat->GetEnclosingCellXYZ(2, {-2.14, 4.285, 0.18})="
         << foundCell
         << ", levelfound=" << levelFound << ", box=" << b << endl;
    Status status;
    cerr << "visdat->GetDatumXYZ(2,0,{{-2.14, 4.285, 0.18}) = "
         << visdat->GetDatumXYZ(2,0,Triple<REAL_T>(rCoords), &status) << endl;

    vector<pair<double,double> > lineplot( visdat->GetLinePlot(
        Triple<double>(-2.14, 4.285, 0.18),
        Triple<double>(-1.81,4.539,0.43), 6, 0, 2 ) );
    cerr << "Line plot: ";
    for( unsigned i=0;i<lineplot.size();++i )
    {
        cerr << "(" << lineplot[i].first << "," << lineplot[i].second << ") ";
    }
    cerr << endl;

    // Particles.
    vector<string> particleCompNames( visdat->GetParticleComponentNames() );
    cerr << "Particle component names: ";
    for( unsigned i=0; i<particleCompNames.size(); ++i )
    {
        string name( particleCompNames[i] );
        cerr << name << " ";
        visdat->GetParticleComponent( name );
    }
    cerr << endl;

    // BoxLayoutData tests.
    for( int lev=0;lev<visdat->GetNumLevels();++lev )
    {
        for( int c=0; c<visdat->GetNumComponents(); ++c )
        {
            boost::shared_ptr< BoxLayoutDataInterfaceForPython >
                bld( visdat->GetBoxLayoutData(lev,c) );

            cerr << "BLD unit test, level " << lev << ", component " << c<<endl;
            bld->UnitTest(lev,c);
            boost::shared_ptr< BoxLayoutDataInterfaceForPython > bldClone(
                bld->Clone() );

            cerr << "BLD unit test on clone, level " << lev
                 << ", component " << c<<endl;
            bldClone->UnitTest(lev,c);

            // Cropping.
            Box croppingBox1( static_cast<BoxLayoutData<REAL_T> *>(
                bldClone.get())->GetBoxLayout()[0] );
            Intvect loCorner( croppingBox1.GetLoCorner() );
            Intvect hiCorner( croppingBox1.GetHiCorner() );
            hiCorner.m_data[0] += 1;
            hiCorner.m_data[1] -= 1;
            int buf[6] = { loCorner[0], loCorner[1], loCorner[2],
                           hiCorner[0], hiCorner[1], hiCorner[2] };
            Box croppingBox2( buf );
            cerr << "VisDat-unittest: cropping to " << croppingBox2 << endl;
            bldClone->CropToGeneralBox( croppingBox2 );
            cerr << "GetBoxLayoutAsNestedVectors()=" 
                 << bldClone->GetBoxLayoutAsNestedVectors() << endl;
            cerr << "*** done cropping" << endl;

            vector<vector<vector<double> > > vd(
                bldClone->GetFArrayAsVectorMatrix(0) );

        }
    }

    boost::shared_ptr<VisualizableDatasetInterfaceForPython> slicedVisDat(
        visdat->Slice('x',REAL_T(-2.123)) );
    string tmpDir(( string("/tmp/chombovis_") + string(getenv("USER")) ));
    slicedVisDat->SaveToHDF5( tmpDir + string("/slice.hdf5" ) );

    vector<string> comps( visdat->GetComponentNames() );
    comps.erase( comps.begin() );
    vector<int> croplevels;
    croplevels.push_back(1); croplevels.push_back(2);
    Box domain( visdat->GetProbDomain(0) );
    domain.Shrink( Intvect(2,2,2), true );
    boost::shared_ptr<VisualizableDatasetInterfaceForPython> croppedVisDat(
        visdat->CropToGeneralBox(
            domain,
            &croplevels,
//          (vector<int> const *)(0),  // croppingLevels
            &comps ) );
    croppedVisDat->SaveToHDF5( tmpDir + string("/cropped.hdf5") );

    visdat->SaveToHDF5( tmpDir + string("/krunch.hdf5") );

    return 0;
}


int
main( int argc, char * argv[] )
{
    Cmdline cfg( argc, argv );
    Status status;

    string ld_library_path( getenv("LD_LIBRARY_PATH") );
    ld_library_path = string("../lib:") + ld_library_path;
    setenv( "LD_LIBRARY_PATH", ld_library_path.c_str(), 1 );

    Trace t("main()");

    cerr << endl << "**** VisDat unit test ****" << endl;
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
            templatizedMain<float>( &cfg );
        } else
        {
            templatizedMain<double>( &cfg );
        }
    } while( cfg.MemoryLeakTest() );

    return 0;
}
