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

#ifndef INCLUDED_VISUALIZABLEDATASET_H
#define INCLUDED_VISUALIZABLEDATASET_H

#include "Intvect.h"
#include "ChomboHDF5.h"
#include "Box.h"
#include "BoxLayout.h"
#include "BoxLayoutData.h"
#include "FAB.h"
#include "../utils/BoxFinder.h"
#include "../utils/VectorFunctor.h"
#include "../utils/StatusCodes.h" // Wrapped funcs can't return void.
#include "./python/VisualizableDatasetInterfaceForPython.h"
#include <boost/shared_ptr.hpp>
#include "../utils/ostream_stdio_adaptor.h"


/** An in-memory representation of the data we'd ordinarily store in an hdf5
 *  file.  Goes to class ChomboHDF5 to load itself from the actual file.  Once
 *  loaded, can be modified (where ChomboHDF5 is not meant to be modifiable).
 *  Goes to ChomboHDF5 just once to load metadata, but repeatedly to load FABs
 *  and particle components (as needed; we cache some of those too here).
*/
template<class REAL_T> class VisualizableDataset
    : public VisualizableDatasetInterfaceForPython
{
  public:
    VisualizableDataset( string infilename );
    boost::shared_ptr<VisualizableDatasetInterfaceForPython>
        Slice( char axis, double axisPos ) const;
    boost::shared_ptr<VisualizableDatasetInterfaceForPython> CropToGeneralBox(
        Box const & croppingBox,
        vector<int> const * croppingLevels=0,
        vector<string> const * croppingComponents=0 ) const;
    virtual ~VisualizableDataset();

    
    // I/O: see also operator<<(ostream, VisualizableDataset).
    Status SaveToHDF5( string outfilename, bool ascii=false ) const;


    // Accessors for global data.
    int    GetDimensionality() const;
    int    GetNumLevels() const;
    int    GetNumComponents() const;
    string GetComponentName( int componentNum ) const;
    int    GetComponentNum( string componentName ) const;
    vector<string> GetComponentNames() const;
    int    GetTotalNumBoxes() const;
    int    GetTotalNumRealBoxes() const;
    int    GetGhostCellsSuppliedByUser() const;
    Intvect const & GetDataCentering() const;
    Triple<REAL_T> const & GetOrigin() const;
    vector<double> GetOriginAsVector() const;
    bool   GetRawAscii() const { return m_rawAscii; }
    void   PrintFabUseCounts() const;


    // Accessors for by-level data.
    void             GetDx( REAL_T * dx, int level ) const;
    Triple<REAL_T>   GetDx( int level ) const;
    vector<double>   GetDxAsVector( int level ) const;
    double           GetDt( int level ) const;
    double           GetTime( int level ) const;
    Intvect const &  GetOutputGhost() const;
    Box const &      GetProbDomain( int level )      const;
    int              GetNumBoxes( int level )        const;
    int              GetNumRealBoxes( int level )    const;
    Box              GetsBox( int level, int boxNum, bool padded ) const;
    Box              GetsRealBox( int level, int boxNum, bool padded ) const;
    vector<int> const &
                     GetConstituentSubdividedBoxnums(
                        int level, int realBoxNum ) const;
    bool             FabIsEmpty( int level, int boxNum, int component,
                                 bool padded ) const;
    bool             RealFabIsEmpty( int level, int boxNum, int component,
                                     bool padded ) const;
    boost::intrusive_ptr<FAB<REAL_T> >
        GetFAB( int level, int boxNum, int component, bool padded ) const;
    boost::intrusive_ptr<FAB<REAL_T> >
        GetRealFAB( int level, int boxNum, int component, bool padded ) const;
    REAL_T           GetDatum( int level, int boxNum, int component,
                               Intvect coords ) const;
    REAL_T           GetDatumXYZ( int finestLevelToSearch, int component,
                                  Triple<REAL_T> coords, Status * status) const;
    boost::shared_ptr< BoxLayoutDataInterfaceForPython > GetBoxLayoutData(
                        int level, int component,
                        bool real=true, bool contrapad=false ) const;
    BoxLayout<REAL_T> const * GetBoxLayout( int level, bool real=true ) const;
    vector<vector<vector<int> > > GetBoxLayoutAsNestedVectors(
                         int level, bool real=true) const;
    int              GetEnclosingBoxNum( int level, int const coords[],
                                         bool useRealBoxes ) const;
    int              GetEnclosingBoxNumXYZ( int level, REAL_T const coords[],
                                            bool useRealBoxes ) const;
    int              GetEnclosingBoxNum( int finestLevelToSearch,
                        int * levelAtWhichFoundBox, int const coords[],
                        bool useRealBoxes ) const;
    int              GetEnclosingBoxNumXYZ( int finestLevelToSearch,
                        int * levelAtWhichFoundBox,
                        REAL_T const coords[],
                        bool useRealBoxes ) const;
    Intvect          GetEnclosingCellXYZ( int finestLevelToSearch,
                        int * levelAtWhichFoundBox, int * boxNum,
                        REAL_T const coords[], bool useRealBoxes) const;

    Status           GenerateGhostValue( REAL_T * ghostVal,
                        Intvect const & globalCoords, int level, int component )
                         const;
    Intvect          ScaleGlobalCoords(
                        Intvect coords, int fromLevel, int toLevel ) const;
    Triple<REAL_T>   GetAnisotropic() const;
    int              GetPrecision() const;

    vector< pair<REAL_T,REAL_T> > GetLinePlotGeneric(
                        Triple<REAL_T> const & p0, Triple<REAL_T> const & p1,
                        int n, int component, int finestLevel ) const;
    vector< pair<double,double> > GetLinePlot(
                        Triple<double> const & p0, Triple<double> const & p1,
                        int n, int component, int finestLevel ) const;
    REAL_T           GetDistanceToEB( REAL_T x, REAL_T y, REAL_T z,
                                      int level, int ebID ) const;

    // State
    int              SetDebugLevel( int d ) const;
    void             SetProbDomain( Box const &, int level );
    void             SetAnisotropic( REAL_T x, REAL_T y, REAL_T z );
    int              DefineNewComponent( string name,
                                         VectorFunctorBase * callable,
                                         vector<string> const & argNames );
    int              DefineNewComponent( string name,
                                         VectorFunctorBase * callable,
                                         vector<int> const & argNums );
    int              RedefineNewComponent( string name,
                                         VectorFunctorBase * callable,
                                         vector<string> const & argNames );
    int              RedefineNewComponent( string name,
                                         VectorFunctorBase * callable,
                                         vector<int> const & argNums );
    int              PinFAB(  boost::intrusive_ptr<FAB<REAL_T> > const &) const;
    int              UnpinFAB( boost::intrusive_ptr<FAB<REAL_T> > const&) const;
    void             FreeUnusedFABs();
    void             SetOptimizationMode( int memory_or_speed );

    // Particles.
    int              GetNumParticles() const;
    int              GetNumParticleComponents() const;
    boost::shared_array<REAL_T> GetParticleComponent( string name ) const;
    vector<string> const & GetParticleComponentNames() const;
    void             ReleaseParticleComponent( string name );
    vector<REAL_T>   GetParticleCoordinates( unsigned particleNum ) const;
    REAL_T           GetParticleCoordinate( unsigned particleNum,
                                            string componentName ) const;

  private:
    //
    // Functions
    //
    VisualizableDataset();                              // Used by Slice().
    VisualizableDataset( VisualizableDataset const & ); // unimplemented
    VisualizableDataset &
        operator=( VisualizableDataset const & );       // unimplemented

    boost::shared_array<REAL_T> LoadFArrayFromDisk( int level, int boxNum,
                                                    int component ) const;
    boost::shared_array<REAL_T> SynthesizeFArray(   int level, int boxNum,
                                                    int component ) const;
    int PopSyntheticComponent( string name );
    void ValidateChomboHDF5Format( Status * ) const;
    void InitRealBoxLayout( int level );
    void InitBoxLayoutData( int level );
    void VerifyBoxAlignment( int level ) const;
    void SubdivideBoxes( int level );
    boost::shared_array<REAL_T> MakePaddedFArray(
        boost::intrusive_ptr<FAB<REAL_T> > const & unpaddedFAB,
        int level, int component ) const;
    boost::shared_array<REAL_T> MakeGhostStrippedFArray(
        Intvect const & userSuppliedGhosts, int level,
        boost::intrusive_ptr<FAB<REAL_T> > const & paddedFab ) const;
    int GetHyperslabSize( int level, int boxNum ) const;
    boost::shared_ptr<ChomboHDF5<REAL_T> const> GetChomboHDF5() const;
    Intvect        XYZ2IJK( int level, Triple<REAL_T> xyz, Status * status = 0 )
                       const;
    Triple<REAL_T> IJK2XYZ( int level, int boxNum, Intvect ijk ) const;
    void SetRawAscii( bool b ) const { m_rawAscii = b; }

    //
    // Data
    //
    GlobalMetadata<REAL_T> m_globalMetadata;
    vector<LevelMetadata<REAL_T> > m_levelMetadataVector;

    vector< BoxLayout<REAL_T> > m_boxLayouts;     // by level
    vector< BoxLayout<REAL_T> > m_realBoxLayouts; // by level
    // m_boxLayouts and m_realBoxLayouts are unpadded, whether there are user-
    // provided ghost cells or not.

    vector< map< int, vector<int> > > m_real2subdividedBoxes;
        // <real boxnum, vector<constituent subdivided boxnums>>, by level.

    boost::shared_ptr< BoxLayoutData<REAL_T> > * * m_BLDs; // by level, by comp
    boost::shared_ptr< BoxLayoutData<REAL_T> > * * m_realBLDs;
    boost::shared_ptr< BoxLayoutData<REAL_T> > * * m_paddedBLDs;
    boost::shared_ptr< BoxLayoutData<REAL_T> > * * m_paddedRealBLDs;

    mutable FAB_Pinner< boost::intrusive_ptr< FAB<REAL_T> > > m_pinnedFabs;
    
    class NewComponentSynthesisInfo
    {
      public:
        ~NewComponentSynthesisInfo();
        void Insert( string name, VectorFunctor<REAL_T> const * callable,
                     vector<string> argNames );
        void Replace( string name, VectorFunctor<REAL_T> const * callable,
                     vector<string> argNames );
        void Remove( string name );
        bool ContainsName( string name ) const;
        VectorFunctor<REAL_T> const * GetCallable( string name ) const;
        vector<string> const & GetArgNames( string name ) const;
      private:
        typedef map<string, pair<VectorFunctor<REAL_T> const *,
                                 vector<string> > > SynthesisInfoRep;
        SynthesisInfoRep m_rep;
    };
    NewComponentSynthesisInfo m_newComponentSynthesisInfo;

    boost::shared_ptr<ChomboHDF5<REAL_T> const> m_chomboHDF5;

    mutable BoxFinder * m_boxFinder;    
    mutable BoxFinder * m_realBoxFinder;    

    bool m_skipBoxSubdivision;
    Intvect m_userSuppliedGhost;
    Intvect m_generatedGhost;
    
    int m_optimizationMode;
    mutable bool m_rawAscii;

    // Particles.
    ParticleMetadata m_particleMetadata;
    mutable map<string, boost::shared_array<REAL_T> > m_particleComponents;
        // Keys are component names.  Mutable means only exception to strict
        // constness is the caching that LoadParticleComponent() does, and
        // GetParticleComponent() relies on.
    void LoadParticleComponent( string componentName ) const;

};

template<class REAL_T, class OSTREAM_T> OSTREAM_T &
    operator<<( OSTREAM_T & out, VisualizableDataset<REAL_T> const & visdat );


#endif // INCLUDED_VISUALIZABLEDATASET_H
