#ifndef _EBSurfaceLib_H_
#define _EBSurfaceLib_H_

// ============================================================
//    Class TriVar
//
//       Simple data structure to allow reference to
//       objects with x-, y-, and z-components in array
//       or component form.
//
//    Class GridInfo
//
//       Simple data structure to maintain information
//       about the grid over which the EB is computed.
//
//    Class EBSurfaceGenerator
//
//       Computes a surface based on fraction information
//       over one uniform rectilinear grid.
//
//       Solutions currently available:
//         o Piece-wise planar
//         o Piece-wise planar with capping (stitching)
//
//    (c) Christopher S. Co, 2002
// ============================================================

#include <vector>
#include "LineSegment.h"
#include "CVPolygon.h"
#include "Plane.h"
#include "EBSingleCell.h"
#include "EBCapper.h"
#include "EBCache.h"

using namespace std ;

template<class Scalar>
class TriVar
{
   public :
      Scalar   x, y, z ;
      Scalar& operator[] ( int ) ;
} ;   // end class TriVar

class GridInfo
{
   public :
         // The grid info, with two modes of reference
      TriVar<double>    origin ;
      TriVar<double>    space ;
      TriVar<int>       dim ;

         // Constructors, Destructor
      GridInfo () ;
} ;   // end class GridInfo

class Indexer
{
   public :
      virtual int index ( int i, int j, int k )
         {
            return 0 ;
         }
      virtual void reverse ( int id, int& i, int& j, int& k )
         {
         }
} ;   // end class Indexer

class EBSurfaceGenerator
{
   public :

      typedef std::vector<int>           IntArray ;
      typedef std::vector<double>        DoubleArray ;
      typedef std::vector<Plane>         PlaneArray ;
      typedef std::vector<LineSegment>   LineArray ;
      typedef std::vector<CVPolygon>       PolyArray ;
      typedef TriVar<int>           Coord ;
      typedef std::vector<Coord>         CoordArray ;

   private :

         // Grid Info
      GridInfo       _grid ;

         // Surface data (geometry)
         //   and ids for which cell they belong to
      DoubleArray    _surface ;
      IntArray       _startAddress ;
      IntArray       _cellIDs ;

         // Computation helpers
      EBSingleCell   _calculator ;
      EBCapper       _capper ;
      EBCache        _cache[2] ;
      double         _cvf ;      // current cell's VoF
      double         _caf[6] ;   // current cell's AF or normal info
      char           _doCapping ;

         // Internal Helpers
      template<class Real>
      Real* _getAddress ( Real *, unsigned int, int, int, int ) ;
      template<class Real>
      void _setCellPtrs ( Real *, Real * ) ;
      template<class Real>
      inline int _IsCovered( Real vf )
         {
            return ( vf <= 0 ) ;
         }
      template<class Real>
      inline int _IsRegular( Real vf )
         {
            return ( vf >= 1 ) ;
         }

      void _transform ( int, int, int, LineSegment& ) ;
      void _transform ( int, int, int, CVPolygon& ) ;
      void _transform ( int, int, int, Plane& ) ;
      void _transform ( int, int, int, LineSegment&, Plane& ) ;
      void _transform ( int, int, int, CVPolygon&, Plane& ) ;
      void _pushOntoArray ( LineSegment&, int, int ) ;
      void _pushOntoArray ( CVPolygon&, int, int ) ;
      void _pushOntoArray ( LineArray&, int, int ) ;
      void _pushOntoArray ( PolyArray&, int, int ) ;
      int _IsValidCell( double *, int ) ;
      template<class Real>
      int _computeSingleCell2D
         (
            Real *, Real *, Real *,
            unsigned int, unsigned int, unsigned int,
            int, int,
            int,
            IntArray&,
            IntArray&,
            EBCache&,
            EBCache&
         ) ;
      template<class Real>
      int _computeSingleCell3D
         (
            Real *, Real *, Real *,
            unsigned int, unsigned int, unsigned int,
            int, int, int,
            int,
            IntArray&,
            IntArray&,
            EBCache&,
            EBCache&
         ) ;
      template<class Real>
      int _compute2D ( Real *, Real *, Real *,
         unsigned int, unsigned int, unsigned int ) ;
      template<class Real>
      int _compute3D ( Real *, Real *, Real *,
         unsigned int, unsigned int, unsigned int ) ;

   public :

      Indexer *      index ; // User must define this

         // Constructors, Destructor
      EBSurfaceGenerator () ;
      ~EBSurfaceGenerator () ;

         // Operations
      template<class Real>
      int compute
         (
            Real *,        // IN : VF data
            Real *,        // IN : Dist data
            Real *,        // IN : AF data OR normal data
            unsigned int,  // IN : VF stride (offset to next VF in VF array)
            unsigned int,  // IN : Dist stride (offset to next item in array)
            unsigned int   // IN : AF stride (offset to next AF in AF array)
         ) ;

         // Set / Access Routines
      unsigned int size() ;
      double* line ( int ) ;
      double* poly ( int, int& ) ;
      int*    cellId ( int ) ;
      void setGrid ( GridInfo& ) ;
      void gridMode ( int m )       { _calculator.gridMode( m ) ; }
      void constraintMode ( int m ) { _calculator.constraintMode( m ) ; }
      void capping ( int c )        { _doCapping = ( c ? 1 : 0 ) ; }
      int capping()                 { return int(_doCapping) ; }
} ;   // end class EBSurfaceGenerator

#include "EBSurfaceLib.C"

#endif
