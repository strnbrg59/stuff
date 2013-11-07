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

#ifndef __vtkChomboReader_h
#define __vtkChomboReader_h

#ifndef UNWRAPPED
#include "VTKChomboConfigure.h" // Include configuration header.
#else
#define VTK_VTKChombo_EXPORT
#endif
#include "vtkObject.h"
#include "vtkDataArray.h"
#include "vtkIntArray.h"
#include "vtkStructuredPointsSource.h"
#include <cassert>

class vtkPolyData;
class vtkStructuredPoints;

/** Acts as the data source in most of our VTK pipelines.
 *
 *  Delegates almost everything to its subclass, vtkChomboReaderImpl -- the
 *  m_pimpl member is a pointer to an instance of that subclass.  The reason
 *  we do it this way is that we want to templatize on float precision (REAL_T)
 *  but VTK's Python wrapping
 *  program considers templates a syntax error.  So we have to hide all mention
 *  of templates from the wrapping program.  That means even m_pimpl cannot
 *  be a pointer to vtkChomboReaderImpl<REAL_T>, so we make m_pimpl a
 *  vtkChomboReader* but its dynamic type is always that of pointer to Impl.
 *  Not why does vtkChomboReaderImpl have to be a subclass of vtkChomboReader?
 *  It actually doesn't have to be; there's no C++ interface anywhere (other
 *  than vtkChomboReader itself, which we could change) that requires a
 *  vtkChomboReader* and to which we need to pass a vtkChomboReaderImpl*.
 *  But what's unavoidable is that vtkChomboReaderImpl does have to be the
 *  subclass of something or other, since Impl is templatized and
 *  vtkChomboReader.h needs to declare a pointer to the Impl without using
 *  template syntax.  So, as long as vtkChomboReaderImpl* has to be a subclass,
 *  and since its interface in any case needs to be pretty much the same as
 *  vtkChomboReader's, then vtkChomboReaderImpl's superclass might as well be
 *  vtkChomboReader.  This is clearly the most natural way to do it.  The only
 *  drawback is that the design pattern can be confusing -- that of a class that
 *  holds a pointer to an instance of its subclass.  See, for example, the
 *  comments inside the Impl's destructor.
 *
 *  -- Dimensionality (2 vs 3) and Slicing --
 *  The ChomboReader has very little dimension-specific code.  Mostly, it relies
 *  on the ChomboHDF5 object to deal with dimensionality.  Where ChomboReader
 *  returns an array (typically, indicating box or domain extents) it returns
 *  the same length array -- 3 or 6 -- and lets the Python layer worry about
 *  using, or ignoring, the numbers that are irrelevant in 2D mode.
 *  The only place where something significant is conditioned on the data
 *  dimensionality is SetPiece(); we generate a 2D FAB on the fly there, for
 *  reslice mode.
 *  
*/
class VTK_VTKChombo_EXPORT vtkChomboReader : public vtkStructuredPointsSource
{
  public:
    //
    // Constructor and destructor.
    //
    vtkChomboReader();
    virtual ~vtkChomboReader(); // Only VTK may call this.  Users call Delete().

    //
    // LoadFile: load a new file and construct m_pimpl.  Actually implemented in
    // vtkChomboReader (i.e. not in Impl class).
    // ShareFile: use another ChomboReader's loaded HDF5 file.
    //
    virtual int LoadFile( const char * fname );
    virtual int ShareFile( vtkChomboReader * that );


    //
    // Overloadings of (VTK) base class methods.
    //
    static vtkChomboReader *New();
    vtkTypeMacro(vtkChomboReader,vtkObject);
    virtual void PrintSelf(ostream& os, vtkIndent indent)
        { m_pimpl->PrintSelf( os, indent ); }
    virtual vtkStructuredPoints * GetOutput() { return m_pimpl->GetOutput(); }
    virtual unsigned long GetMTime() { return m_pimpl->GetMTime(); }
    virtual void Execute() { m_pimpl->Execute(); }


    //
    // Information about the entire HDF5 dataset.
    //
    virtual int * GetDataCentering() const
        { return m_pimpl->GetDataCentering();}
    virtual double * GetCoordinateOrigin() const
        { return m_pimpl->GetCoordinateOrigin(); }
    virtual int * GetDataCenteringPermuted() const
        { return m_pimpl->GetDataCenteringPermuted();}
    virtual int * GetDomainExtents() const
        { return m_pimpl->GetDomainExtents(); }
    virtual double * GetDomainExtentsXYZ() const
        { return m_pimpl->GetDomainExtentsXYZ(); }
    virtual int GetNumComponents() const
        { return m_pimpl->GetNumComponents(); }
    virtual const char * GetComponentName( int c ) const
        { return m_pimpl->GetComponentName( c ); }
    virtual int * GetBoxOrigin( int level, int boxNum ) const
        { return m_pimpl->GetBoxOrigin( level, boxNum ); }
    virtual double * GetBoxExtentsXYZ( int level, int boxNum,
                                       int visiblePartOnly ) const
        { return m_pimpl->GetBoxExtentsXYZ( level, boxNum, visiblePartOnly ); }
    virtual int * GetBoxDims( int level, int boxNum ) const
        { return m_pimpl->GetBoxDims( level, boxNum ); }
    virtual double GetDatum( int level, int boxNum, int component,
                             int i, int j, int k ) const
        { return m_pimpl->GetDatum( level, boxNum, component, i,j,k ); }
    virtual int GetFArray( int level, int boxNum, int component ) const
        { return m_pimpl->GetFArray( level, boxNum, component ); }
    virtual int      GetEnclosingBoxNumXYZ( int level,
                                            double x, double y, double z )
        { return m_pimpl->GetEnclosingBoxNumXYZ( level, x, y, z ); }
    virtual int      GetLevelNumBoxes(int level) const
        { return m_pimpl->GetLevelNumRealBoxes( level ); }
    virtual int      GetLevelNumRealBoxes(int level) const
        { return m_pimpl->GetLevelNumBoxes( level ); }
    virtual double * GetLevelDx(int level) const
        { return m_pimpl->GetLevelDx( level ); }
    virtual double   GetLevelDt(int level) const
        { return m_pimpl->GetLevelDt( level ); }
    virtual double   GetLevelTime(int level) const
        { return m_pimpl->GetLevelTime( level ); }
    virtual int      GetNumLevels() const { return m_pimpl->GetNumLevels(); }
    virtual double * GetVisibleRange() const
        { return m_pimpl->GetVisibleRange(); }
    virtual double * GetConstrainedVisibleRange( double lo, double hi ) const
        { return m_pimpl->GetConstrainedVisibleRange( lo, hi ); }
    virtual double * GetLevelComponentRange( int level, int componentNum) const
        { return m_pimpl->GetLevelComponentRange( level, componentNum);}
    virtual int      GetNumEBs() const { return m_pimpl->GetNumEBs(); }
    virtual void     SetEBIndex( int i ) { m_pimpl->SetEBIndex( i ); }
    virtual int *    GetDimensions( int level, int boxNum, int axisNum,
                              int /*0|1*/ padded, int /*0|1*/ realBox ) const
        { return m_pimpl->GetDimensions( level,boxNum,axisNum,padded,realBox); }
    virtual double * GetSpacing( int level, int axisNum ) const
        { return m_pimpl->GetSpacing( level, axisNum ); }
    virtual double * GetOrigin( int level, int boxNum, int axisNum,
                                int padded, int realBox) const
        { return m_pimpl->GetOrigin( level,boxNum,axisNum,padded,realBox ); }
    virtual double * GetBounds( int level, int boxNum, int axisNum,
                                int padded, int realBox) const
        { return m_pimpl->GetBounds( level,boxNum,axisNum,padded,realBox ); }
    virtual void SetEBIsVisible( int i ) { m_pimpl->SetEBIsVisible( i ); }
    virtual int  GetEBIsVisible() { return m_pimpl->GetEBIsVisible(); }
    virtual int  PlaneIntersectsBox(
                     int level, int box,
                     char planeDirection, double planePosition ) const
        { return m_pimpl->PlaneIntersectsBox( level, box, planeDirection,
                                                          planePosition ); }

    int GetNumDims() const { return m_numDims; }      // 2 or 3.  3 in reslice.
    int GetPrecision() const { return m_precision; }  // 1 or 2


    //
    // Information about the state of the vtkChomboReader.  In the functions
    // GetVisibleBox*(), arg boxNum is an index into m_visibleFABs.
    //
    virtual int      GetNumPieces() const { return m_pimpl->GetNumPieces(); }
    virtual int      LevelIsVisible( int level ) const
        { return m_pimpl->LevelIsVisible( level ); }
    virtual int      GetCurrentComponentNum() const
        { return m_pimpl->GetCurrentComponentNum(); }
    virtual const char * GetCurrentComponentName() const
        { return m_pimpl->GetCurrentComponentName(); }
    virtual int      GetVisibleBoxLevel(int boxNum) const
        { return m_pimpl->GetVisibleBoxLevel( boxNum ); }
    virtual int      GetVisibleBoxNum(int boxNum) const
        { return m_pimpl->GetVisibleBoxNum( boxNum ); }
    virtual int      IsSlicingMode() const { return m_pimpl->IsSlicingMode(); }
    virtual int      Is3DMode() const { return m_pimpl->Is3DMode(); }
    virtual char     GetSlicingDirection() const
        { return m_pimpl->GetSlicingDirection(); }
    virtual double   GetSlicingPosition() const
        { return m_pimpl->GetSlicingPosition(); }
    virtual int      GetAlwaysUseRealBoxes() const
        { return m_pimpl->GetAlwaysUseRealBoxes(); }
    virtual double * GetAnisotropicFactors() const
        { return m_pimpl->GetAnisotropicFactors(); }
    virtual int      GetAlwaysUsePaddedBoxes() const
        { return m_pimpl->GetAlwaysUsePaddedBoxes(); }
    virtual int      GetGhostCellsSuppliedByUser() const
        { return m_pimpl->GetGhostCellsSuppliedByUser(); }
    virtual double   GetDistanceToEB( double x, double y, double z,
                                      int level, int ebID ) const
        { return m_pimpl->GetDistanceToEB(x,y,z,level,ebID); }
    virtual int      GetCurrentPieceIndex() const
        { return m_pimpl->GetCurrentPieceIndex(); }
    virtual int      GetCurrentPieceLevel() const
        { return m_pimpl->GetCurrentPieceLevel(); }
    virtual int      GetCurrentPieceBoxNum() const
        { return m_pimpl->GetCurrentPieceBoxNum(); }
    virtual int      CurrentPieceIsPadded() const
        { return m_pimpl->CurrentPieceIsPadded(); }
    virtual int      CurrentPieceIsReal() const
        { return m_pimpl->CurrentPieceIsReal(); }

    //
    // Control over the state of the vtkChomboReader.
    //
    virtual int  SetPiece( int boxNum, char axis, double axis_position )
        { return m_pimpl->SetPiece( boxNum, axis, axis_position ); }
    virtual void ShowLevel(int i) { m_pimpl->ShowLevel( i ); }
    virtual void HideLevel(int i) { m_pimpl->HideLevel( i ); }
    virtual void HideBox(int lev, int b ) { m_pimpl->HideBox( lev, b ); }
    virtual void ShowBox(int lev, int b ) { m_pimpl->ShowBox( lev, b ); }
    virtual void HideBoxes(int lev, vtkIntArray * bb )
        { m_pimpl->HideBoxes( lev, bb ); }
    virtual void ShowBoxes(int lev, vtkIntArray * bb )
        { m_pimpl->ShowBoxes( lev, bb ); }
    virtual void HideAllBoxes(int lev ) { m_pimpl->HideAllBoxes( lev ); }
    virtual void ShowAllBoxes(int lev ) { m_pimpl->ShowAllBoxes( lev ); }
    virtual void SetCurrentComponentName( const char * name )
        { m_pimpl->SetCurrentComponentName( name ); }
    virtual void SetCurrentComponentNum( int n )
        { m_pimpl->SetCurrentComponentNum( n ); }
    virtual void SetSlicingDirection( char direction )
        { m_pimpl->SetSlicingDirection( direction ); }
    virtual void SetSlicingPosition( double position )
        { m_pimpl->SetSlicingPosition( position ); }
    virtual void SetAlwaysUseRealBoxes( int yesno )
        { m_pimpl->SetAlwaysUseRealBoxes( yesno ); }
    virtual void SetAlwaysUsePaddedBoxes( int yesno )
        { m_pimpl->SetAlwaysUsePaddedBoxes( yesno ); }
    virtual void SetVtkHierarchicalBoxHack( int yesno )
        { m_pimpl->SetVtkHierarchicalBoxHack( yesno ); }
    virtual void PinFAB( int level, int boxNum, int component ) const
        { m_pimpl->PinFAB( level, boxNum, component ); }
    virtual void UnpinFAB( int level, int boxNum, int component ) const
        { m_pimpl->UnpinFAB( level, boxNum, component ); }
    virtual void SetAnisotropicFactors( float x, float y, float z )
        { m_pimpl->SetAnisotropicFactors( x,y,z ); }
    virtual void SetDebugLevel( int d ) const { m_pimpl->SetDebugLevel( d ); }
    virtual void SetOriginShift( double dx0, double dy0, double dz0,
                                 double dxL, double dyL, double dzL )
        { m_pimpl->SetOriginShift( dx0, dy0, dz0, dxL, dyL, dzL ); }
    virtual void SetOptimizationMode( int memory_or_speed )
        { m_pimpl->SetOptimizationMode( memory_or_speed ); }


    // Output
    virtual void DumpAscii() const { m_pimpl->DumpAscii(); }
    virtual void DumpHDF5( const char * outfilename ) const
        { m_pimpl->DumpHDF5( outfilename ); }


    // Particles
    virtual int /* ptr to REAL_T */ GetParticleComponent( const char * name )
      const { return m_pimpl->GetParticleComponent( name ); }
    virtual double GetParticleCoordinate( int particleNumber,
                                          const char * compName ) const
        { return m_pimpl->GetParticleCoordinate( particleNumber, compName ); }
    virtual int GetNumParticles() const { return m_pimpl->GetNumParticles(); }
    virtual int GetNumParticleComponents() const
        { return m_pimpl->GetNumParticleComponents(); }
    virtual const char * GetParticleComponentName( int serialNumber ) const
        { return m_pimpl->GetParticleComponentName( serialNumber ); }
    virtual double GetParticleComponentRangeMin( const char * name ) const
        { return m_pimpl->GetParticleComponentRangeMin( name ); }
    virtual double GetParticleComponentRangeMax( const char * name ) const
        { return m_pimpl->GetParticleComponentRangeMax( name ); }


    // Bridge to Data API.
    virtual int /* VisualizableDataset* */ GetVisualizableDatasetPtr() const
        { return m_pimpl->GetVisualizableDatasetPtr(); }

  private:
    int m_numDims;
    int m_precision;
    vtkChomboReader * m_pimpl; // Always a vtkChomboReaderImpl.
};

#endif
