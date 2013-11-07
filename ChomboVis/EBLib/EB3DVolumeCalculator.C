// ============================================================
//    Class EB3DVolumeCalculator
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

#include "EB3DVolumeCalculator.h"
using std::cerr;
using std::endl;

// ============================================================
//    Constructors, Destructor
// ============================================================

EB3DVolumeCalculator :: EB3DVolumeCalculator ()
   : EBVolumeCalculator()
   {
      _cell3D = NULL ;
   }

/*
EB3DVolumeCalculator :: ~EB3DVolumeCalculator ()
   {
   }
*/

// ============================================================
//    Find bracketing anchor points for cutting plane
// ============================================================

int EB3DVolumeCalculator :: ObtainBrackets ()
   {
      KIJVector   a, b, c ;
      KIJVector   n = _currentCut.direction() ;
      int   i ;

      if( _cell3D == NULL )
      {
         cerr  << "EB3DVolumeCalculator::ObtainBrackets(): "
               << "must have valid pointer to cell" << endl ;
         return 0 ;
      }

      Hexahedron& cell = *(_cell3D) ;
// TODO Use an iterator from ConnectedHexahedron instead.
      int   connectivity[8][3] =
         {
            { 1, 2, 4 } ,     //  Offsets of points in "cell"
            { 0, 3, 5 } ,     //  that are connected to the i-th
            { 0, 3, 6 } ,     //  cell point
            { 1, 2, 7 } ,
            { 0, 5, 6 } ,
            { 1, 4, 7 } ,
            { 2, 4, 7 } ,
            { 3, 5, 7 } 
         } ;

         //  The proper "corner" point has acute angles with
         //    the 3 vectors formed by it with its neighboring
         //    points.
         //  NOTE: We cheated... we know the internal storage
         //    structure of the hexahedron class here.
      for( i=0; i<8; i++ )
      {
         a = cell[connectivity[i][0]] - cell[i] ;
         b = cell[connectivity[i][1]] - cell[i] ;
         c = cell[connectivity[i][2]] - cell[i] ;

         if( dot( n, a ) >= 0 && dot( n, b ) >= 0 && dot( n, c ) >= 0 )
         {
            _lb = i ;
            break ;
         }
      }
      for( i=0; i<8; i++ )
      {
         a = cell[connectivity[i][0]] - cell[i] ;
         b = cell[connectivity[i][1]] - cell[i] ;
         c = cell[connectivity[i][2]] - cell[i] ;

         if( dot( -n, a ) >= 0 && dot( -n, b ) >= 0 && dot( -n, c ) >= 0 )
         {
            _ub = i ;
            return 1 ;
         }
      }

      cerr  << "EB3DVolumeCalculator::ObtainProperCornerPoint(): "
            << "You should not see this message." << endl ;
cerr << n << endl ;

      return 0 ;
   }

inline Point EB3DVolumeCalculator :: GetPlane ( const double t )
   {
      if( _cell3D == NULL )
      {
         cerr  << "EB3DVolumeCalculator::GetPlane(): "
               << "must have valid pointer to cell" << endl ;
         return PointZero ;
      }

      Hexahedron& cell = *(_cell3D) ;
      return affine( cell[_lb], cell[_ub], t ) ;
   }

// ============================================================
//    Volume difference calculation
// ============================================================

double EB3DVolumeCalculator :: volume ( double t )
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
         if( _cell3D )
         {
            Point anchor ;
            Hexahedron & cell = *_cell3D ;
            CVPolygon ignore ;

            anchor = GetPlane( t ) ;
            _currentCut.SetAnchor( anchor ) ;
            _slicer->mode( CellIntersection::COMPUTE_INTERSECTING_VOLUME ) ;
            _slicer->IntersectCellWithPlane
               (
                  _currentCut,
                  cell,
                  ignore
               ) ;
            volume = _slicer->VolumeOfIntersection3D() ;
         }
         else
         {
            cerr  << "EB3DVolumeCalculator::volume(): "
                  << "no cell for volume calculation." << endl ;
         }
      }
      return volume-_target ;
   }

// ============================================================
//    Set / Access Routines
// ============================================================

void EB3DVolumeCalculator :: cell ( Hexahedron * c )
   {
      if( c )
      {
         _cell3D = c ;
         _cellVolume = _cell3D->Volume() ;
      }
   }
