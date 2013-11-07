#ifndef INCLUDED_ASCII2HDF5_H
#define INCLUDED_ASCII2HDF5_H

#include "ChomboHDF5.h"
#include "../utils/Consts.h"

//
// Below here, stuff to support conversion from ChomboVis ascii to hdf5 format.
// See examples/chomboascii.dat for documentation on the ChomboVis ascii format.
//
namespace Ascii2HDF5
{
    static OldConsts StrConsts;

    // The public interface:
    void Convert( string outfile_name, istream & input_stream,
                  bool raw=false );

    //-----------------------------------------------------------
    // Implementation details from here down...
    //


    //
    // Reading
    //
    template<class T> void EatItem(
        istream & input_stream, string expectedString, T * value, Status * s=0);
    void ReadGlobalMetadataFromAscii(
             istream & inputStream,
             int * spaceDim, int * numLevels, int * numComponents,
    	     vector<string> * componentNames, Intvect * centering,
	         vector<int> * probDomain, vector<double> * origin,
             vector<double> * anisotropic );
    void ReadLevelMetadataFromAscii(
        int iLevel, int spaceDim, istream & inputStream,
        vector<vector<int> > * boxCorners, double *dx, double *dt, double *time,
        Intvect * outputGhost );

    void HandleParticles( istream &, HDF5GroupID const & parent, int spaceDim );
    void ReadParticlesMetadata( istream & inputStream, int spaceDim,
        int * numComponents, int * numParticles, Status * status );
    template<class T> void ReadVectorChunk( istream &, vector<T> * buf );

    //
    // Writing
    //

    void DumpGlobalMetadataToHDF5( HDF5GroupID const & slashGroup, int spaceDim,
        int numLevels, Intvect const & centering,
        vector<string> const & componentNames,
        vector<double> const & origin, vector<double> const & anisotropic );

    void DumpParticlesMetadata( HDF5GroupID const & group, int numComponents,
        int numParticles );
    
    void WriteAttribute( HDF5GroupID const & group, char const * name,
                         double val,                 herr_t * status = 0 );
    void WriteAttribute( HDF5GroupID const & group, char const * name,
                         int val,                    herr_t * status = 0 );
    void WriteAttribute( HDF5GroupID const & group, char const * name,
                         string val,                 herr_t * status = 0 );
    void WriteAttribute( HDF5GroupID const & group, char const * name,
                         Intvect const & val,        herr_t * status = 0 );
    void WriteAttribute( HDF5GroupID const & group, char const * name,
                         Triple<double> const & val, herr_t * status = 0 );
    void WriteAttribute( HDF5GroupID const & group, char const * name,
                         Box const & val,            herr_t * status = 0 );

    template<class T> void WriteVectorChunk( vector<T> const & buf, int offset,
                                             hid_t dataset, hid_t dataspace );
    template<class T> void VectorStreamIO( istream &, char const * datasetName,
                                           HDF5GroupID const &, int chunkSize,
                                           int totSize );
    template<class T> void InitializeHDF5Dataset( int totalSize, 
                                                  HDF5GroupID const & group,
                                                  char const * datasetName,
                                                  hid_t *dataspace,
                                                  hid_t * dataset );    
};

#endif // INCLUDED_ASCII2HDF5_H
