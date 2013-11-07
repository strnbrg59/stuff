
#include "ChomboHDF5.h"
#include "Ascii2HDF5.h"
#include <iterator>
#include <cassert>

using std::istream_iterator;


/** Reads our proprietary ASCII format from stdin or any other istream, and
 *  produces an HDF5 file in the format ChomboVis likes.
 *  To see a description of our legal ASCII format, look at ChomboVis-x.y.z/
 *  examples/chomboascii.dat.
 *
 *  Ascii2HDF5 is a namespace, not a class.
 *  It would have made an awkward sort of object to design a class around; the
 *  object would only exist while Convert() is reading ascii from stdin.  It
 *  would certainly have made things hard to follow if I'd converted most of the
 *  function arguments -- spaceDim, numLevels, dataCentering, etc -- to class
 *  data members.  Ascii2HDF5 is a process, not an object.
*/
void Ascii2HDF5::Convert( string outfileName, istream & inputStream,
                          bool raw )
{
    Trace t("Convert()");

    //
    // Pick up space_dim, num_levels, component_names and prob_domain.
    // Then dump them (and items we can deduce from them (eg #components))
    // to the HDF5 output.
    //
    int spaceDim, numLevels, numComponents;
    Intvect dataCentering;
    vector<string> componentNames;
    vector<int> probDomain;
    vector<double> origin;
    vector<double> anisotropic;
    ReadGlobalMetadataFromAscii(
                 inputStream, &spaceDim, &numLevels, &numComponents,
                 &componentNames, &dataCentering, &probDomain, &origin,
                 &anisotropic );

    ChomboHDF5<double>::InitializeHDF5Datatypes( spaceDim );    

    hid_t fileHID = H5Fcreate( outfileName.c_str(),
                               H5F_ACC_TRUNC,
                               H5P_DEFAULT,
                               H5P_DEFAULT );

    HDF5GroupID slash_GroupID( fileHID, "/" );
    DumpGlobalMetadataToHDF5( slash_GroupID, spaceDim, numLevels,
                              dataCentering, componentNames, origin,
                              anisotropic );

    //
    // Loop over levels, picking up dx, dt, boxes and data, and writing them out
    // as we read them.
    //
    double dxLevel0 = 0;
    for( int iLevel=0;iLevel<numLevels;++iLevel )
    {
        // Read level's metadata -- dx and boxCorners.
        vector<vector<int> > boxCorners;
        double dx, dt, time;
        Intvect outputGhost;
        ReadLevelMetadataFromAscii( iLevel, spaceDim,
                                    inputStream, &boxCorners, &dx, &dt, &time,
                                    &outputGhost );
        if( iLevel==0 ) dxLevel0 = dx;

        int nBoxes = boxCorners.size();
    
        // Dump it to HDF5.
        // Create HDF5 group and enter its scope.
        ostringstream groupName;
        groupName << StrConsts.level_ << iLevel;
        H5Gcreate( slash_GroupID.GetHid(), groupName.str().c_str(), 0 );
        HDF5GroupID level_GroupID( slash_GroupID.GetHid(),
                                   groupName.str().c_str() );

        // Dump dx, dt and time
        WriteAttribute( level_GroupID, StrConsts.dx, dx );
        WriteAttribute( level_GroupID, StrConsts.dt, dt );
        WriteAttribute( level_GroupID, Consts::time, time );

        //
        // Dump problem domain (for this level).
        //
        BoxSimple boxsimple;
        for( int i=0;i<spaceDim;++i )
        {
            boxsimple.data[i] = int(probDomain[i]*dxLevel0/dx + 0.5);
            boxsimple.data[i+THREE] = 
                int((1+probDomain[i+spaceDim])*dxLevel0/dx + 0.5) - 1;
        }
        Box levelProbDomain( boxsimple, -1 );
        herr_t hstatus;
        WriteAttribute( level_GroupID, StrConsts.prob_domain,
                        levelProbDomain, &hstatus );
        t.Info( "Status from H5Dwrite() = %d.", hstatus );

        //    
        // Dump box data to HDF5.
        //
        {
            BoxSimple * boxsimples = new BoxSimple[ nBoxes ];
            for( int ib=0;ib<nBoxes;++ib )
            {
                for( int i=0;i<spaceDim;++i )
                {
                    boxsimples[ib].data[i] = boxCorners[ib][i];
                    boxsimples[ib].data[i+THREE] = boxCorners[ib][i+spaceDim];
                }
            }
            {
            hsize_t dataset_size( nBoxes );
            hid_t dataspace = H5Screate_simple( 1, &dataset_size, 0 );
            hid_t dataset = H5Dcreate(
                level_GroupID.GetHid(), StrConsts.boxes,
                Box::s_HDF5Datatype, dataspace, H5P_DEFAULT);
            herr_t status = 
              H5Dwrite( dataset, Box::s_HDF5Datatype, dataspace, dataspace,
                        H5P_DEFAULT, boxsimples );
            t.Info( "Status from H5Dwrite() = %d.", status );
            H5Sclose( dataspace );
            H5Dclose( dataset );
            }
            delete[] boxsimples;
        }

        //
        // Data (Current item in cin is now "data:").
        //
        if( componentNames.size() > 0 )
        {
            int nComps = componentNames.size();
            int totalCells = 0;  // Over all boxes-x-components at this level
            vector<int> boxCells;
            for( int iBox=0;iBox<nBoxes;++iBox )
            {
                boxCells.push_back(1);  // per component, this box
                for( int iCorner=0;iCorner<spaceDim;++iCorner )
                {
                    boxCells[iBox] *= (2*outputGhost.m_data[iCorner]
                                       + dataCentering[iCorner]
                                       + 1 + boxCorners[iBox][iCorner+spaceDim] 
                                       - boxCorners[iBox][iCorner]);
                }
                t.Info( "Cells per component in box %d = %d.",
                        iBox, boxCells[iBox] );
                totalCells += boxCells[iBox] * nComps;
            }
            t.Info( "Total cells this level=%d.", totalCells );
    
            hsize_t dataset_size = totalCells;
            hid_t dataspace = H5Screate_simple( 1, &dataset_size, 0 );
            hid_t dataset = H5Dcreate(
                level_GroupID.GetHid(), Consts::data_datatype0,
                H5T_NATIVE_DOUBLE, dataspace, H5P_DEFAULT );
            ch_offset_t hyperslabOffset = 0;
            for( int iBox=0;iBox<nBoxes;++iBox )
            {
                for( int iComp=0;iComp<nComps;++iComp )
                {
                    //
                    // Read one box's worth of data from stdin.
                    //
                    vector<double> data;
                    data.reserve( boxCells[iBox] );

                    // A "raw" ascii file has the field data as raw doubles (but
                    // everything else is its regular human-readable self).
                    // The raw format is produced by operator<<(VisDat), when
                    // so instructed by VisDat::SaveToHDF5().  It's a speed
                    // optimization; otherwise we're left to format every number
                    // when operator<< writes it, and then call atof() upon
                    // reading it back in over here.
                    // The non-raw mode is appropriate for hand-typed text
                    // files.
                    if( raw )
                    {
                        // The raw doubles are sandwiched between a '|' and a
                        // newline.
                        char plentybuf[120];
                        inputStream.getline( plentybuf, 119, '|' );
                        assert( !inputStream.eof() );

                        int nBytes( boxCells[iBox]*sizeof(double) );
                        char buf[ nBytes ];
                        inputStream.read( buf, nBytes );
                        data.resize( boxCells[iBox] );
                        memcpy( &data[0], buf, nBytes );

                        inputStream.getline( plentybuf, 119 ); // trailing '\n'
                    } else
                    {
                       istream_iterator<double> dit(inputStream);
                       data.push_back( *dit );
   
                       for( int i=1;i<boxCells[iBox];++i )
                       {
                           ++dit;
                           if( ! inputStream.good() )
                           {
                               t.FatalError("ios_base stream state not 'good'. "
                                "Must have read some kind of bad data or maybe "
                                "reached eof unexpectedly." );
                           }
                           data.push_back( *dit );
                       }
                    }

                    //
                    // Dump to HDF5.
                    //
                    hsize_t hyperslab_size( boxCells[iBox] );
                    H5Sselect_hyperslab(
                        dataspace, H5S_SELECT_SET, &hyperslabOffset,
                        0, &hyperslab_size, 0 );
                    hid_t hyperslab_space =
                        H5Screate_simple( 1, &hyperslab_size, 0 );
                    herr_t status = 
                      H5Dwrite( dataset, H5T_NATIVE_DOUBLE, hyperslab_space,
                                dataspace, H5P_DEFAULT, &data[0] );
                    t.Info( "Status from H5Dwrite() = %d.", status );
    
                    hyperslabOffset += boxCells[iBox];
                }
            }
            H5Dclose( dataset );
            H5Sclose( dataspace );
        }
        //
        // Create "data_attributes" group under level_% group.
        //
        {
            herr_t err = H5Gcreate( level_GroupID.GetHid(),
                                    Consts::data_attributes, 0);
            HDF5GroupID dataAttributes_GroupID( level_GroupID.GetHid(),
                                                Consts::data_attributes );
            if( err != 0 )
            {
                t.Info( "H5Gcreate( %s ) returned %d.",
                        Consts::data_attributes, err );
            }

            // Dump outputGhost.
            herr_t status;
            WriteAttribute( dataAttributes_GroupID,
                            StrConsts.outputGhost, outputGhost,&status);
            t.Info( "Status from H5Dwrite() of outputGhost = %d.", status );
        }
    }

    //
    // Particles
    //
    HandleParticles( inputStream, slash_GroupID, spaceDim );

    H5Fclose( fileHID );
}


void
Ascii2HDF5::HandleParticles( istream & inputStream, HDF5GroupID const & parent,
                             int spaceDim )
{
    Trace t("Ascii2HDF5::HandleParticles()");
    int numParticleComponents, numParticles;
    Status status;
    ReadParticlesMetadata( inputStream, spaceDim, &numParticleComponents,
        &numParticles, &status );
    if( status == STATUS_EOF )
    {
        t.Error( "Reached unexpected EOF in hdf5 file" );
        return;                               // Early return
    }
    if( status == STATUS_NOT_FOUND)
    {
        return;                               // Early return
    }

    // Create particles group.
    H5Gcreate( parent.GetHid(), Consts::particles, 0 );
    HDF5GroupID particleGroup( parent.GetHid(), Consts::particles );

    DumpParticlesMetadata( particleGroup, numParticleComponents, numParticles );
   
    int const dataChunkSize(1024); // For production, will use a larger number.
   
    for( int iComp=0;iComp<numParticleComponents;++iComp )
    {
        ostringstream ost;
        ost << StrConsts.component_ << iComp;
        EatItem( inputStream, ost.str(), (int*)0 );
        string name;
        EatItem( inputStream, Consts::name, &name );
        // Create HDF5 dataset by that name.
        EatItem( inputStream, Consts::values, (int*)0 );
        // Stream component data, writing chunks out to hdf5.
        VectorStreamIO<double>( inputStream, name.c_str(), particleGroup,
            dataChunkSize, numParticles );
    }
}


void
Ascii2HDF5::ReadParticlesMetadata( istream & inputStream, int spaceDim,
    int * numComponents, int * numParticles, Status * status )
{
    Trace t("ReadParticlesGroup()");
    EatItem( inputStream, string(Consts::particles), (string *)0, status );
    if( (*status == STATUS_NOT_FOUND) || (*status == STATUS_EOF) )
    {
        t.Info( "Did not find particles.  OK." );
        return;
    }
    t.Info( "Found particles:" );
    // If got here, then there's a particles group.

    EatItem( inputStream, string(StrConsts.num_components),  numComponents );
    EatItem( inputStream, string(Consts::num_particles), numParticles );
}


void
Ascii2HDF5::DumpParticlesMetadata(
    HDF5GroupID const & group, int numComponents, int numParticles )
{
    WriteAttribute( group, StrConsts.num_components, numComponents );
    WriteAttribute( group, Consts::num_particles, numParticles );
}


void
Ascii2HDF5::ReadLevelMetadataFromAscii(
           int iLevel, int spaceDim, istream & inputStream,
           vector<vector<int> > * boxCorners,
           double *dx, double *dt, double *time,
           Intvect * outputGhost )
{
    Trace t("ReadLevelMetadataFromAscii");

    // level_%
    ostringstream groupName;
    groupName << StrConsts.level_ << iLevel;
    EatItem( inputStream, groupName.str(), (int *)0 );

    // dx, dt and time
    EatItem( inputStream, string(StrConsts.dx), dx );
    EatItem( inputStream, string(StrConsts.dt), dt );
    EatItem( inputStream, string(Consts::time), time );

    // outputGhost
    EatItem( inputStream, string(StrConsts.output_ghost), (int *)0 );
    int temp[3] = {0,0,0};
    istream_iterator<int> iit( inputStream );
    temp[0] = *iit;
    for( int i=1;i<spaceDim;++i )
    {
        ++iit;
        temp[i] = *iit;
    }
    *outputGhost = Intvect( temp );


    // boxes
    EatItem( inputStream, StrConsts.boxes, (int *)0 );
    istream_iterator<string> sit( inputStream );
    int iBox = 0;
    while( (string(Consts::data) + ":") != (*sit) )
    {
        vector<int> temp;
        boxCorners->push_back( temp );
        (*boxCorners)[iBox].push_back( atoi( sit->c_str() ) );
        for( int i=1;i<2*spaceDim;++i )
        {
            int val = atoi( (++sit)->c_str());
            (*boxCorners)[iBox].push_back( val );
        }
        ++sit;
        ++iBox;
    }

    int nBoxes = iBox;
    t.Info( "nBoxes=%d", nBoxes );
}


/** Reads from an ascii file. */
void
Ascii2HDF5::ReadGlobalMetadataFromAscii(
             istream & inputStream,
             int * spaceDim, int * numLevels, int * numComponents,
             vector<string> * componentNames, Intvect * dataCentering,
             vector<int> * probDomain, vector<double> * origin,
             vector<double> * anisotropic )
{
    Trace t("ReadGlobalMetadataFromAscii()");

    // spaceDim
    EatItem( inputStream, StrConsts.space_dim, spaceDim );

    // numLevels
    EatItem( inputStream, StrConsts.num_levels, numLevels );

    // numComponents
    EatItem( inputStream, StrConsts.num_components, numComponents );

    { // componentNames
        EatItem( inputStream, Consts::component_names, (int *)0 );
        if( *numComponents > 0 )
        {
            istream_iterator<string> sit( inputStream );
            string name( *sit );
            componentNames->push_back( name );
            for( int i=1;i<(*numComponents);++i )
            {
                ++sit;
                assert( sit != istream_iterator<string>() );
                name = *sit;
                componentNames->push_back( name );
            }
        } else
        {
            t.Info( "No components.  OK" );
        }
    }

    // data_centering
    int iCentering;
    EatItem( inputStream, StrConsts.data_centering, &iCentering );
    assert( IsLegalDataCentering( iCentering ) );
    *dataCentering = DataCenteringInt2Intvect( iCentering, *spaceDim );

    { // probDomain
        EatItem( inputStream, StrConsts.prob_domain, (int *)0 );
        istream_iterator<int> iit( inputStream );
        probDomain->push_back( *iit );
        for( int i=1;i<2*(*spaceDim);++i )
        {
            ++iit;
            int val = *iit;
            probDomain->push_back( val );
        }
    }

    { // origin
        EatItem( inputStream, StrConsts.origin, (int *)0 );
        istream_iterator<double> dit( inputStream );
        origin->push_back( *dit );
        for( int i=1;i<(*spaceDim);++i )
        {
            ++dit;
            double val = *dit;
            origin->push_back( val );
        }
    }

    { // anisotropic
        EatItem( inputStream, StrConsts.anisotropic, (int *)0 );
        istream_iterator<double> dit( inputStream );
        anisotropic->push_back( *dit );
        for( int i=1;i<(*spaceDim);++i )
        {
            ++dit;
            double val = *dit;
            anisotropic->push_back( val );
        }
    }
}


void
Ascii2HDF5::DumpGlobalMetadataToHDF5( HDF5GroupID const & slashGroup,
    int spaceDim, int numLevels, Intvect const & dataCentering,
    vector<string> const & componentNames, vector<double> const & origin,
    vector<double> const & anisotropic )
{
    Trace t("DumpGlobalMetadataToHDF5()");

    WriteAttribute( slashGroup, Consts::num_components,
                    int(componentNames.size()) );
    WriteAttribute( slashGroup, StrConsts.num_levels, numLevels );
    WriteAttribute( slashGroup, StrConsts.data_centering,
        DataCenteringIntvect2Int(dataCentering, spaceDim) );
    Triple<double> originRealvect;
    for( int i=0;i<spaceDim;++i ) originRealvect.m_data[i] = origin[i];
    WriteAttribute( slashGroup, StrConsts.origin, originRealvect );
    Triple<double> anisotropicRealvect;
    for( int i=0;i<spaceDim;++i ) anisotropicRealvect.m_data[i]=anisotropic[i];
    WriteAttribute( slashGroup, StrConsts.anisotropic,
                    anisotropicRealvect );


    // Component names.
    for( unsigned int iComp=0;iComp<componentNames.size();++iComp )
    {
        ostringstream component_number;
        component_number << StrConsts.component_ << iComp;
        WriteAttribute( slashGroup, component_number.str().c_str(),
               componentNames[iComp] );
    }

    //
    // File type -- just one type for now; not supporting new EB format yet.
    //
    WriteAttribute( slashGroup, StrConsts.filetype,
                                StrConsts.filetype_name );


    //
    // Group "Chombo_global" -- for testReal and space_dim.
    //
    herr_t err = H5Gcreate( slashGroup.GetHid(), Consts::Chombo_global, 0 );
    if( err != 0 )
    {
        t.Info( "H5Gcreate( %s ) returned %d.",  Consts::Chombo_global, err );
    }
    HDF5GroupID chomboglobal_GroupID( slashGroup.GetHid(),
                                      Consts::Chombo_global );
    
    WriteAttribute( chomboglobal_GroupID, Consts::testReal, double(3.14) );
    WriteAttribute( chomboglobal_GroupID, Consts::SpaceDim, spaceDim );
}

void
Ascii2HDF5::WriteAttribute( HDF5GroupID const & group, char const * name,
                            double val, herr_t * a_status /* =0 */)
{
    Trace t("WriteAttribute(double)");
    hid_t aid = H5Screate(H5S_SCALAR);
    hid_t attr = H5Acreate( group.GetHid(), name, H5T_NATIVE_DOUBLE,
                            aid, H5P_DEFAULT );
    double valCopy( val );
    H5Awrite( attr, H5T_NATIVE_DOUBLE, &valCopy );
    H5Aclose( attr);
    H5Sclose( aid );
}        

void
Ascii2HDF5::WriteAttribute( HDF5GroupID const & group, char const * name,
                            int val, herr_t * a_status /* =0 */)
{
    Trace t("WriteAttribute(int)");
    hid_t aid = H5Screate(H5S_SCALAR);
    hid_t attr = H5Acreate( group.GetHid(), name,
                            H5T_NATIVE_INT, aid, H5P_DEFAULT );
    int valCopy( val );
    herr_t status = H5Awrite( attr, H5T_NATIVE_INT, &valCopy );
    H5Aclose( attr );
    H5Sclose( aid );
    if( a_status ) { *a_status = status; }
}        

void
Ascii2HDF5::WriteAttribute( HDF5GroupID const & group, char const * name,
                            string val, herr_t * a_status /* =0 */)
{
    Trace t("WriteAttribute(string)");
    hid_t aid = H5Screate(H5S_SCALAR);
    hid_t s_type = H5Tcopy(H5T_C_S1);
    H5Tset_size(s_type, val.size() );
    H5Eset_auto(0,0);
    hid_t attr = H5Acreate( group.GetHid(), name, s_type, aid, H5P_DEFAULT );
    char * valCopy = new char[val.size()+1];
    strcpy( valCopy, val.c_str() );
    herr_t status = H5Awrite( attr, s_type, valCopy );
    delete [] valCopy;
    H5Aclose( attr );
    H5Tclose( s_type );
    H5Sclose( aid );
    if( a_status ) { *a_status = status; }
}


void
Ascii2HDF5::WriteAttribute( HDF5GroupID const & group, char const * name,
                   Intvect const & val, herr_t * a_status /* =0 */)
{
    Trace t("WriteAttribute(Intvect)");
    hid_t aid = H5Screate(H5S_SCALAR);
    hid_t attr = H5Acreate( group.GetHid(), name, Intvect::s_HDF5Datatype,
                            aid, H5P_DEFAULT );
    Intvect tempVal( val );
    herr_t status = H5Awrite( attr, Intvect::s_HDF5Datatype,
                              &tempVal );
    H5Aclose( attr );
    H5Sclose( aid );
    if( status ) { *a_status = status; }
}

void
Ascii2HDF5::WriteAttribute( HDF5GroupID const & group, char const * name,
                   Triple<double> const & val, herr_t * a_status /* =0 */)
{
    Trace t("WriteAttribute(Triple<double>)");
    hid_t aid = H5Screate(H5S_SCALAR);
    hid_t attr = H5Acreate( group.GetHid(), name,
        Triple<double>::s_HDF5Datatype, aid, H5P_DEFAULT );
    Triple<double> tempVal( val );
    herr_t status = H5Awrite( attr, Triple<double>::s_HDF5Datatype,
                              &tempVal );
    H5Aclose( attr );
    H5Sclose( aid );
    if( status ) { *a_status = status; }
}

void
Ascii2HDF5::WriteAttribute( HDF5GroupID const & group, char const * name,
                   Box const & val, herr_t * a_status /* =0 */)
{
    Trace t("WriteAttribute(Box)");
    hid_t aid = H5Screate(H5S_SCALAR);
    hid_t attr = H5Acreate(
        group.GetHid(), name,
        Box::s_HDF5Datatype, aid, H5P_DEFAULT );
    BoxSimple bs( val );
    herr_t status = H5Awrite( attr, Box::s_HDF5Datatype, &bs );
    H5Aclose( attr );
    H5Sclose( aid );
    if( status ) { *a_status = status; }
}


/** Dumps a piece of a vector to hdf5.  Called after ReadVectorChunk.  Size of
 *  buf is number of items to write.  Arg offset is an offset into the complete
 *  vector we're building up in hdf5.
 *
 *  Template specialization necessary because we have to say H5T_NATIVE_DOUBLE.
*/
namespace Ascii2HDF5 {
template<> void
WriteVectorChunk<double>( vector<double> const & buf, int offset,
    hid_t dataset, hid_t dataspace )
{
    Trace t("Ascii2HDF5::WriteVectorChunk<double>()");
    /*
    // For now, dump to stdout.
    for( unsigned i=0;i<buf.size();++i )
    {
        cout << buf[i] << " ";
    }
    cout << endl;
    */

    hsize_t hyperslabSize( buf.size() );
    ch_offset_t hyperslabOffset( offset );
    H5Sselect_hyperslab( dataspace, H5S_SELECT_SET, &hyperslabOffset, 0, 
        &hyperslabSize, 0 );
    hid_t hyperslabSpace = H5Screate_simple( 1, &hyperslabSize, 0 );
    herr_t status =
        H5Dwrite( dataset, H5T_NATIVE_DOUBLE, hyperslabSpace, dataspace,
                  H5P_DEFAULT, &buf[0] );
    t.Info( "Status from H5Dwrite()=%d.", status );
}


/** create() and create_simple.
 *  Implemented as a template specialization because it needs to say, for
 *  example, H5T_NATIVE_DOUBLE.
 */
template<> void
InitializeHDF5Dataset<double>(
    int totalSize, HDF5GroupID const & group, char const * datasetName,
    hid_t *dataspace, hid_t * dataset )
{
    Trace t("Ascii2HDF5::InitializeHDF5Dataset()");
    hsize_t size( totalSize );
    *dataspace = H5Screate_simple( 1, &size, 0 );
    *dataset = H5Dcreate( group.GetHid(), datasetName, H5T_NATIVE_DOUBLE,
                          *dataspace, H5P_DEFAULT );
}

}


/** From a stream (usually stdin) read a chunk of vector data, storing it in
 *  buf.  After this, we call WriteVectorChunk() to dump the same data to hdf5.
 *  The size of buf is the number of items we attempt to read.
*/
template<class T> void
Ascii2HDF5::ReadVectorChunk( istream & inputStream, vector<T> * buf )
{
    std::istream_iterator<T> it( inputStream );
    (*buf)[0] = *it;
    for( unsigned i=1;i<buf->size();++i )
    {
        ++it;
        (*buf)[i] = *it;
    }
}


/** Stream in vector data from ascii, and write it out to hdf5.  We alternately
 *  read and write chunkSize items.
*/
template<class T> void
Ascii2HDF5::VectorStreamIO( istream & inputStream, char const * datasetName,
    HDF5GroupID const & group, int chunkSize, int totalSize )
{
    Trace t("VectorStreamIO()");

    hid_t dataspace, dataset;
    InitializeHDF5Dataset<T>( totalSize, group, datasetName,
                              &dataspace, &dataset );    

    int numWholeChunks = totalSize/chunkSize;
    int remainderChunkSize = totalSize % chunkSize;

    if( numWholeChunks > 0 )
    {
        vector<T> chunk(chunkSize);
        for( int i=0;i<numWholeChunks;++i )
        {
            ReadVectorChunk( inputStream, &chunk );
            WriteVectorChunk( chunk, i*chunkSize, dataset, dataspace );
        }
    }
    
    if( remainderChunkSize > 0 )
    {
        vector<T> remainderChunk(remainderChunkSize);
        ReadVectorChunk( inputStream, &remainderChunk );
        WriteVectorChunk( remainderChunk, chunkSize*numWholeChunks,
                          dataset, dataspace );
    }
}


/** Grab a string and a T from inputStream.  Verify the string matches
 *  expectedString + ":", and copy the next item into *value.  If value==NULL,
 *  just read the string.
 *  If status!=NULL, set a status.
 */
template<class T> void
Ascii2HDF5::EatItem( istream & inputStream, string expectedString, T * value,
                     Status * status /*=0*/ )
{
    Trace t("EatItem()");

    if( inputStream.eof() )
    {
        if( status )
        {
            *status = STATUS_EOF;
            return;                              // Early return!
        } else
        {
            t.FatalError( "Reached eof on inputStream." );
        }
    }

    std::istream_iterator<string> sit( inputStream );
    string theString( *sit );
    t.Info( "Read in |%s|.", theString.c_str() );
    if( theString != (expectedString + string(":")) )
    {
        if( status )
        {
            *status = STATUS_NOT_FOUND;
            return;                              // Early return!
        } else
        {
            t.FatalError( "%s does not match |%s|",
                      (expectedString+string(":")).c_str(), theString.c_str() );
        }
    }

    if( status ) *status = STATUS_OK;

    if( value )
    {
        std::istream_iterator<T> tit( inputStream );
        *value = *tit;
    }
}


//
// Explicit template instantiation: not needed, as this stuff is used
// only in the ascii2hdf5 executable (ascii2hdf5-main.cpp), and is therefore
// instantiated there.  Ascii2hdf5.cpp doesn't get compiled into any library.
//

