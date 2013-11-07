// ============================================================
//    Class EBVolumeCalculator
//
//       Computes the difference between a target
//       volume and the volume of a half-plane intersection.
//       The plane is described by a passed in parameter.
//
//       This is to be used with a root finding algorithm
//       (EBFindPlane).
//
//    (c) Christopher S. Co
// ============================================================

#include "EBVolumeCalculator.h"
using std::cerr;
using std::endl;

// ============================================================
//    Constructors, Destructor
// ============================================================

EBVolumeCalculator :: EBVolumeCalculator ()
   {
      _slicer = NULL ;
      _target = 0.0 ;
      _cellVolume = 0.0 ;
   }

EBVolumeCalculator :: ~EBVolumeCalculator ()
   {
   }

// ============================================================
//    Find bracketing anchor points for cutting plane
// ============================================================

int EBVolumeCalculator :: ObtainBrackets ()
   {
      cerr  << "EBVolumeCalculator::ObtainProperCornerPoint(): "
            << "routine must be re-implemented" << endl ;
      return 0 ;
   }

inline Point EBVolumeCalculator :: GetPlane ( const double t )
   {
      cerr  << "EBVolumeCalculator::GetPlane(): "
            << "routine must be re-implemented" << endl ;
      return PointZero ;
   }

// ============================================================
//    Volume difference calculation
// ============================================================

double EBVolumeCalculator :: volume ( double t )
   {
      cerr  << "EBVolumeCalculator::volume(): "
            << "routine must be re-implemented" << endl ;
      return 0.0 ;
   }

// ============================================================
//    Set Routines
// ============================================================

void EBVolumeCalculator :: normal ( const KIJVector& n )
   {
      _currentCut.SetDirection( n ) ;
   }

void EBVolumeCalculator :: target ( double a )
   {
      if( a < 0.0 || a > 1.0 )
      {
         cerr  << "EBVolumeCalculator::target(): "
               << "target volume must be >= 0.0 and <= 1.0" << endl ;
         return ;
      }
      _target = a * _cellVolume ;
   }

void EBVolumeCalculator :: cellVolume ( double a )
   {
      if( a < 0.0 )
      {
         cerr  << "EBVolumeCalculator::cellVolume(): "
               << "cell volume must be >= 0.0" << endl ;
         return ;
      }
      _cellVolume = a ;
   }

void EBVolumeCalculator :: slicer ( CellIntersection * s )
   {
      if( s )
      {
         _slicer = s ;
      }
   }

// ============================================================
//    Access Routines
// ============================================================

double EBVolumeCalculator :: target ()
   {
      return _target ;
   }

double EBVolumeCalculator :: cellVolume ()
   {
      return _cellVolume ;
   }

CellIntersection * EBVolumeCalculator :: slicer ()
   {
      return _slicer ;
   }

Plane& EBVolumeCalculator :: lastPlane ()
   {
      return _currentCut ;
   }
