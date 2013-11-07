#ifndef _EBSingleCell_h_
#define _EBSingleCell_h_

// ============================================================
//    Class EBSingleCell
//
//       Computes a planar fit to solve the EB problem
//       given fraction data inside a single hexahedral
//       cell.
//
//       Different constraints can be met, so this class
//       is highly configurable.
//
//    (c) Christopher S. Co
// ============================================================

#include <vector>
#include "Point.h"
#include "KIJVector.h"
#include "Plane.h"
#include "LineSegment.h"
#include "CVPolygon.h"
#include "Hexahedron.h"
#include "CellIntersection.h"
#include "EBFindPlane.h"
#include "EB2DVolumeCalculator.h"
#include "EB3DVolumeCalculator.h"

using namespace std ;

namespace EBSINGLE
{
      // Grid modes
   const int GRID2D                    = 0 ;
   const int GRID3D                    = 1 ;
      // Constraint modes
   const int CONSTRAIN_VF_AF           = 2 ;
   const int CONSTRAIN_VF_NORMAL       = 3 ;
   const int CONSTRAIN_DISTANCE        = 4 ;
   const int CONSTRAIN_AF_LS_SUM       = 5 ;
} ;

class EBSingleCell
{
   public :

      typedef  std::vector<int>       IntArray ;

   protected :

      Hexahedron     _cell3D ;            // 3D cell used for intersection
      CVPolygon        _cell2D ;            // 2D cell used for intersection
      double         _cellVol ;           // 2D/3D cell volume

      int            _gridMode ;          // Grid mode (2D/3D)
      int            _constraintMode ;    // Constraint to satisfy
      double         _axisAlignTol ;      // Tolerance of axis alignment

      CellIntersection     _slicer ;      // Cell intersecter
      EBFindPlane          _finder ;      // Iterative root finder
      EB2DVolumeCalculator _compute2D ;   // Computes 2D cell volume
      EB3DVolumeCalculator _compute3D ;   // Computes 3D cell volume

      Plane                _lastPlane ;   // Last plane used for intersection

      int            _didSpecialCase ;    // Flags when special case was done
      int            _specCaseFace ;      // ID of cell face used in spec. case

         // Internal helpers
      void _DeriveNormalFromAF ( double *, const int, double * ) ;
      int _IsValidCell( double *, int ) ;
      int _PerformSpecialCaseAnalysis2D
         ( const double[2], const double, LineSegment& ) ;
      int _PerformSpecialCaseAnalysis3D
         ( const double[3], const double, CVPolygon& ) ;
      int _equals ( double, double, double ) ;
      int _insideCell ( const Plane& ) ;

   public :

         // Constructors, Destructor
      EBSingleCell () ;
      ~EBSingleCell () ;

         // Operations
      int compute ( double, double*, LineSegment& ) ;
      int compute ( double, double*, CVPolygon& ) ;

         // Set Routines
      void gridMode ( int ) ;
      void constraintMode ( int ) ;
      void tolerance ( double ) ;
      void axisAlignTolerance ( double ) ;
      void setCellDimensions( double, double, double ) ;

         // Access Routines
      int gridMode () ;
      int constraintMode () ;
      double tolerance () ;
      double axisAlignTolerance () ;
      void getCellDimensions( double&, double&, double& ) ;
      Plane ebPlane () ;
      IntArray facesIntersected () ;
      Point intersectionCentroid () ;
      CVPolygon& cell2D ()      { return _cell2D ; }
      Hexahedron& cell3D ()   { return _cell3D ; }
} ;

#endif
