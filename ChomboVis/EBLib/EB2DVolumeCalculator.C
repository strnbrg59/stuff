// ============================================================
//    Class EB2DVolumeCalculator
//
//       Computes the difference between a target
//       volume and the volume of a half-plane intersection.
//       The plane is described by a passed in parameter.
//
//       This is to be used with a root finding algorithm
//       (EBFindPlane).
//
//       Uses a tetrahedral decomposition to compute the
//       volume.
//
//    (c) Christopher S. Co
// ============================================================

#include "EB2DVolumeCalculator.h"
using std::cerr;
using std::endl;

// ============================================================
//    Constructors, Destructor
// ============================================================

EB2DVolumeCalculator :: EB2DVolumeCalculator ()
   : EBVolumeCalculator()
   {
      _cell2D = NULL ;
   }

// ============================================================
//    Find bracketing anchor points for cutting plane
// ============================================================

int EB2DVolumeCalculator :: ObtainBrackets ()
   {
      KIJVector   a, b ;
      KIJVector   n = _currentCut.direction() ;
      int   i ;

      if( _cell2D == NULL )
      {
         cerr  << "EB2DVolumeCalculator::ObtainBrackets(): "
               << "must have valid pointer to cell" << endl ;
         return 0 ;
      }

      CVPolygon& cell = *(_cell2D) ;
// TODO Use an iterator from ConnectedHexahedron instead.
      int   connectivity[4][2] =
         {
            { 1, 3 } ,     //  Offsets of points in "cell"
            { 0, 2 } ,     //  that are connected to the i-th
            { 1, 3 } ,     //  cell point
            { 2, 0 }
         } ;

         //  The proper "corner" point has acute angles with
         //    the 3 vectors formed by it with its neighboring
         //    points.
         //  NOTE: We cheated... we know the internal storage
         //    structure of the hexahedron class here.
      for( i=0; i<4; i++ )
      {
         a = cell[connectivity[i][0]] - cell[i] ;
         b = cell[connectivity[i][1]] - cell[i] ;

         if( dot( n, a ) >= 0 && dot( n, b ) >= 0 )
         {
            _lb = i ;
            break ;
         }
      }
      for( i=0; i<4; i++ )
      {
         a = cell[connectivity[i][0]] - cell[i] ;
         b = cell[connectivity[i][1]] - cell[i] ;

         if( dot( -n, a ) >= 0 && dot( -n, b ) >= 0 )
         {
            _ub = i ;
            return 1 ;
         }
      }

      cerr  << "EB2DVolumeCalculator::ObtainBrackets(): "
            << "You should not see this message." << endl ;

      return 0 ;
   }

inline Point EB2DVolumeCalculator :: GetPlane ( const double t )
   {
      if( _cell2D == NULL )
      {
         cerr  << "EB2DVolumeCalculator::GetPlane(): "
               << "must have valid pointer to cell" << endl ;
         return PointZero ;
      }

      CVPolygon& cell = *(_cell2D) ;
      return affine( cell[_lb], cell[_ub], t ) ;
   }

// ============================================================
//    Volume difference calculation
// ============================================================

double EB2DVolumeCalculator :: volume ( double t )
   {
      if( _slicer == NULL )
      {
         cerr  << "EB3DVolumeCalculator::volume(): "
               << "need access to cell slicer" << endl ;
         return 0.0 ;
      }

      double volume = 0.0 ;

      if( t == 0.0 )
      {
         volume = _cellVolume ;
      }
      else
      if( t == 1.0 )
      {
         volume = 0.0 ;
      }
      else
      {
         if( _cell2D )
         {
            Point anchor ;
            CVPolygon & cell = *_cell2D ;
            Point ignore1, ignore2 ;

            anchor = GetPlane( t ) ;
            _currentCut.SetAnchor( anchor ) ;
            _slicer->mode( CellIntersection::COMPUTE_INTERSECTING_VOLUME ) ;
            _slicer->IntersectCellWithPlane
               (
                  _currentCut,
                  cell,
                  ignore1, ignore2
               ) ;
            volume = _slicer->VolumeOfIntersection2D() ;
         }
         else
         {
            cerr  << "EB2DVolumeCalculator::volume(): "
                  << "no cell for volume calculation." << endl ;
         }
      }

      return volume-_target ;
   }

// ============================================================
//    Set / Access Routines
// ============================================================

void EB2DVolumeCalculator :: cell ( CVPolygon * c )
   {
      if( c )
      {
         _cell2D = c ;
         _cellVolume = _cell2D->Area() ;
      }
   }
