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

#include <math.h>
#include "EBSingleCell.h"
#include "CellIntersection.h"

// ============================================================
//    Constructors, Destructor
// ============================================================

EBSingleCell :: EBSingleCell ()
   {
      _cellVol          = 1.0 ;

         // Configure 2D volume calculator
      _compute2D.slicer( &_slicer ) ;
      _compute2D.cell( &_cell2D ) ;

         // Configure 3D volume calculator
      _compute3D.slicer( &_slicer ) ;
      _compute3D.cell( &_cell3D ) ;
      //_compute3D.cellVolume( _cellVol ) ;

         // Configure plane finder
      _finder.volume( &_compute3D ) ;
      _finder.tolerance( 1.0e-5 ) ;
      _finder.cell2D( &_cell2D ) ;
      _finder.cell3D( &_cell3D ) ;

      _axisAlignTol     = 1.0e-5 ;
      _gridMode         = EBSINGLE::GRID3D ;
      _constraintMode   = EBSINGLE::CONSTRAIN_VF_AF ;
      _didSpecialCase   = 0 ;
      _specCaseFace     = 0 ;
   }

EBSingleCell :: ~EBSingleCell ()
   {
   }

// ============================================================
//    Operations
// ============================================================

// TODO Make this cleaner and more readable (?)
int EBSingleCell :: compute ( double VF, double *AF, LineSegment& line )
   {
      double * normal = NULL ;

         // Reset special case flag
      _didSpecialCase = 0 ;

         // Determine normal
      switch( _constraintMode )
      {
         case EBSINGLE::CONSTRAIN_VF_AF :
               //  Derive the normal vector quantity
            normal = new double [2] ;
            _DeriveNormalFromAF( AF, 2, normal ) ;
            break ;
         case EBSINGLE::CONSTRAIN_VF_NORMAL :
            normal = AF ;
            break ;
         case EBSINGLE::CONSTRAIN_DISTANCE :
            normal = AF ;
            break ;
         case EBSINGLE::CONSTRAIN_AF_LS_SUM :
            cerr  << "EBSingleCell::constraintMode(): "
                  << "constraint mode not yet implemented" << endl ;
            return 0 ;
      }

      // This is a hack!!! - TJL
      //if (VF < 1e-10) VF = 0.0;
      if( _equals( VF, 0, tolerance() ) ) VF = 0 ;

         // Test for special cases
      if( _constraintMode != EBSINGLE::CONSTRAIN_DISTANCE )
      {
         if( VF >= 1.0 || VF <= 0.0 )
         {
            _didSpecialCase = 
               _PerformSpecialCaseAnalysis2D( normal, VF, line ) ;
            if( ! _didSpecialCase )
            {
                  //  No EB
               return 0 ;
            }
            if( _constraintMode == EBSINGLE::CONSTRAIN_VF_AF )
            {
               if( normal ) delete [] normal ;
            }
         }
      }

      if( ! _IsValidCell( normal, 2 ) )
      {
         if( _constraintMode == EBSINGLE::CONSTRAIN_VF_AF )
         {
            if( normal ) delete [] normal ;
         }
         return 0 ;
      }
      else
      {
            // Do not compute intersection if distance is zero
         //if( VF == 0.0 )
         //{
            //return 0 ;
         //}
      }

      if( !_didSpecialCase )
      {
         KIJVector planeNormal( normal[0], normal[1], 0 ) ;
         switch( _constraintMode )
         {
            case EBSINGLE::CONSTRAIN_VF_AF :
            case EBSINGLE::CONSTRAIN_VF_NORMAL :
               {
                  Point anchor ;
                  _finder.volume( &_compute2D ) ;
                  _finder.target( VF ) ;
                  anchor = _finder.find( planeNormal ) ;
                  _slicer.mode( CellIntersection::COMPUTE_INTERSECTED_FACES ) ;
                  _lastPlane = Plane( anchor, planeNormal ) ;
                  _slicer.IntersectCellWithPlane
                     ( _lastPlane, _cell2D,
                        line.left(), line.right() ) ;
                  _lastPlane.SetAnchor( line.centroid() ) ;
                  if( _constraintMode == EBSINGLE::CONSTRAIN_VF_AF )
                  {
                     if( normal ) delete [] normal ;
                  }
                  return 1 ;
               }
            case EBSINGLE::CONSTRAIN_DISTANCE :
               {
                  //
                  // NOTE!!! : Greg Miller's data uses negative
                  //   distance values. He also considers the normal
                  //   to point away from the VoF.
                  //   Considering both of these, and the fact that
                  //   by the time we reach this code, we already flipped
                  //   his normal, the following anchor calculation
                  //   is correct
                  //
 
                  Point anchor ;
                  planeNormal.normalize() ;
                  anchor = _cell2D[0] - ( VF * planeNormal ) ;
                  _lastPlane = Plane( anchor, planeNormal ) ;
                  if( _insideCell( _lastPlane ) )
                  {
                     _slicer.mode
                        ( CellIntersection::COMPUTE_INTERSECTED_FACES ) ;
                     _slicer.IntersectCellWithPlane
                        ( _lastPlane, _cell2D,
                           line.left(), line.right() ) ;
                     _lastPlane.SetAnchor( line.centroid() ) ;
                  }
                  else
                  {
cerr  << "EBSingleCell::compute(): "
      << "plane given by distance constraint "
      << "does not intersect cell" << endl ;
                     return 0 ;
                  }
                  return 1 ;
               }
            case EBSINGLE::CONSTRAIN_AF_LS_SUM :
               cerr  << "EBSingleCell::constraintMode(): "
                     << "constraint mode not yet implemented" << endl ;
               return 0 ;
         }
      }

      return 1 ;
   }

// TODO Make this cleaner and more readable (?)
int EBSingleCell :: compute ( double VF, double *AF, CVPolygon& inter )
   {
      double * normal = NULL ;

         // Reset special case flag
      _didSpecialCase = 0 ;

         // Determine normal
      switch( _constraintMode )
      {
         case EBSINGLE::CONSTRAIN_VF_AF :
               //  Derive the normal vector quantity
            normal = new double [3] ;
            _DeriveNormalFromAF( AF, 3, normal ) ;
            break ;
         case EBSINGLE::CONSTRAIN_VF_NORMAL :
            normal = AF ;
            break ;
         case EBSINGLE::CONSTRAIN_DISTANCE :
            normal = AF ;
            break ;
         case EBSINGLE::CONSTRAIN_AF_LS_SUM :
            cerr  << "EBSingleCell::constraintMode(): "
                  << "constraint mode not yet implemented" << endl ;
            return 0 ;
      }

      // This is a hack!!! - TJL
      //if (VF < 1e-10) VF = 0.0;
      if( _equals( VF, 0, tolerance() ) ) VF = 0 ;

         // Test for special cases
      if( _constraintMode != EBSINGLE::CONSTRAIN_DISTANCE )
      {
         if( VF >= 1.0 || VF <= 0.0 )
         {
            _didSpecialCase = 
               _PerformSpecialCaseAnalysis3D( normal, VF, inter ) ;
            if( ! _didSpecialCase )
            {
                  //  No EB
               return 0 ;
            }
            if( _constraintMode == EBSINGLE::CONSTRAIN_VF_AF )
            {
               if( normal ) delete [] normal ;
            }
         }
      }
      else
      {
            // Do not compute intersection if distance is zero
         //if( VF == 0.0 )
         //{
            //return 0 ;
         //}
      }

      if( ! _IsValidCell( normal, 3 ) )
      {
         if( _constraintMode == EBSINGLE::CONSTRAIN_VF_AF )
         {
            if( normal ) delete [] normal ;
         }
         return 0 ;
      }

      if( !_didSpecialCase )
      {
         KIJVector planeNormal( normal[0], normal[1], normal[2] ) ;
         switch( _constraintMode )
         {
            case EBSINGLE::CONSTRAIN_VF_AF :
            case EBSINGLE::CONSTRAIN_VF_NORMAL :
               {
                  Point anchor ;
                  _finder.volume( &_compute3D ) ;
                  _finder.target( VF ) ;
                  anchor = _finder.find( planeNormal ) ;
                  _slicer.mode( CellIntersection::COMPUTE_INTERSECTED_FACES ) ;
                  _lastPlane = Plane( anchor, planeNormal ) ;
                  _slicer.IntersectCellWithPlane
                     ( _lastPlane, _cell3D, inter ) ;
                  _lastPlane.SetAnchor( inter.Centroid() ) ;
                  if( _constraintMode == EBSINGLE::CONSTRAIN_VF_AF )
                  {
                     if( normal ) delete [] normal ;
                  }
                  return 1 ;
               }
            case EBSINGLE::CONSTRAIN_DISTANCE :
               {
                  //
                  // NOTE!!! : Greg Miller's data uses negative
                  //   distance values. He also considers the normal
                  //   to point away from the VoF.
                  //   Considering both of these, and the fact that
                  //   by the time we reach this code, we already flipped
                  //   his normal, the following anchor calculation
                  //   is correct
                  //

                  Point anchor ;
                  planeNormal.normalize() ;
                  anchor = _cell3D[0] - ( VF * planeNormal ) ;
                  _lastPlane = Plane( anchor, planeNormal ) ;
                  if( _insideCell( _lastPlane ) )
                  {
                     _slicer.mode
                        ( CellIntersection::COMPUTE_INTERSECTED_FACES ) ;
                     _slicer.IntersectCellWithPlane
                        ( _lastPlane, _cell3D, inter ) ;
                     _lastPlane.SetAnchor( inter.Centroid() ) ;
                  }
                  else
                  {
cerr  << "EBSingleCell::compute(): "
      << "plane given by distance constraint "
      << "does not intersect cell" << endl ;
                     return 0 ;
                  }
                  return 1 ;
               }
            case EBSINGLE::CONSTRAIN_AF_LS_SUM :
               cerr  << "EBSingleCell::constraintMode(): "
                     << "constraint mode not yet implemented" << endl ;
               return 0 ;
         }
      }

      return 1 ;
   }

// ===============================================================
//    Equality helper
// ===============================================================

int EBSingleCell :: _equals ( double a, double b, double tol )
   {
      return ( fabs(a-b)<=(tol) ? 1 : 0 ) ;
   }

// ===============================================================
//    Determines if a point is inside the cell
// ===============================================================

int EBSingleCell :: _insideCell ( const Plane& p )
   {
      unsigned int i ;
      int in,out,on ;
      int inside ;

      in = out = on = 0 ;
      switch( _gridMode )
      {
         case EBSINGLE::GRID2D :
            for( i=0; i<_cell2D.size(); i++ )
            {
               if( p.IsIn( _cell2D[i] ) )
                  in++ ;
               else
                  out++ ;
               //if( p.IsOut( _cell2D[i] ) )
                  //out++ ;
               //else
                  //on++ ;
            }
            break ;
         case EBSINGLE::GRID3D :
            for( i=0; i<_cell3D.size(); i++ )
            {
               if( p.IsIn( _cell3D[i] ) )
                  in++ ;
               else
                  out++ ;
               //if( p.IsOut( _cell3D[i] ) )
                  //out++ ;
               //else
                  //on++ ;
            }
            break ;
      }

      //if( on > 0 )
         //inside = 0 ;
      //else
      if( in == 0 || out == 0 )
      {
         inside = 0 ;
      }
      else
      {
         inside = 1 ;
      }

      return inside ;
   }

// ===============================================================
//    Derives a normal from area fraction info
//
//    Reference page 8 of ebmain.ps in ChomboDoc for
//    and explanation of the calculations.
//
//    n indicates the number of dimensions (2 or 3)
// ===============================================================

void EBSingleCell :: _DeriveNormalFromAF
   (
      double * AF,
      const int n,
      double * normal
   )
   {
      int h, l ;
      const int dim = n*2 ;
      double A_I = 0.0 ;

         //  Calculate A_I
      for( h=1, l=0; l<dim; h+=2, l+=2 )
      {
         A_I += ( AF[h] - AF[l] ) * ( AF[h] - AF[l] ) ;
      }
      A_I = sqrt( A_I ) ;

         //  Compute components of normal
      for( h=1, l=0; h<dim; h+=2, l+=2 )
      {
         normal[h/2] = ( AF[h] - AF[l] ) / A_I ;
      }
   }

// ===============================================================
//    Checks if the cell is valid
//    n indicates the number of dimensions (2 or 3)
// ===============================================================

int EBSingleCell :: _IsValidCell ( double * norm, int n )
   {
      KIJVector v ;

      if( n == 2 )
      {
         v = KIJVector( norm[0], norm[1], 0 ) ;
      }
      else
      {
         v = KIJVector( norm[0], norm[1], norm[2] ) ;
      }

      if( dot(v,v) < 0.1 )
         return 0 ;

      return 1 ;
   }

// ===============================================================
//    Special case analysis -- Axis aligned normals and VF = [0|1]
// ===============================================================

int EBSingleCell :: _PerformSpecialCaseAnalysis2D
   (
      const double   normal[2],     // IN : Plane normal
      const double   VF,            // IN : Volume fraction
      LineSegment&   line           // OUT: the EB
   )
   {
      unsigned char  specialCase = 0x0 ;
      double         component = 0.0 ;

         //  Encoding of the problem
         //  There are 3 (boolean) parameters:
         //     MSB - along X (if not, then along Y)
         //         - Normal component ( >0 or <0 )
         //     LSB - VF ( >0 or <0 )
      const unsigned char  axisMask = 0x4 ;
      const unsigned char  compMask = 0x2 ;
      const unsigned char  vfMask   = 0x1 ;

// TODO perhaps find a way to use the iterators (not high priority)
      const int CaseTable[8][2] =
         {
            { 1, 0 },   // Along Y
            { 2, 3 },
            { 3, 2 },
            { 0, 1 },
            { 0, 3 },   // Along X
            { 1, 2 },
            { 2, 1 },
            { 3, 0 }
         } ;
      const int FaceTable[8] =
         {
            -1,   // Along Y
            -3,
             3,
             1,
            -4,   // Along X
            -2,
             2,
             4
         } ;

      int alongX, alongY ;

      alongX = ! _equals( normal[0], 0.0, _axisAlignTol )
               && _equals( normal[1], 0.0, _axisAlignTol ) ;
      alongY = _equals( normal[0], 0.0, _axisAlignTol )
               && ! _equals( normal[1], 0.0, _axisAlignTol ) ;

      if( (!alongX) && (!alongY) )
      {
         return 0 ;
      }

      if( alongX )
      {
         specialCase |= axisMask ;
         component = normal[0] ;
      }
      else
      if( alongY )
      {
         component = normal[1] ;
      }
      else
      {
         cerr << "PerformSpecialCaseAnalysis2D: unknown case" << endl ;
         exit( 1 ) ;
      }

      if( VF > 0.0 )
      {
         specialCase |= vfMask ;
      }
      if( component > 0.0 )
      {
         specialCase |= compMask ;
      }

      line.left( _cell2D[ CaseTable[specialCase][0] ] ) ;
      line.right( _cell2D[ CaseTable[specialCase][1] ] ) ;
      _specCaseFace = FaceTable[specialCase] ;

      return 1 ;
   }

int EBSingleCell :: _PerformSpecialCaseAnalysis3D
   (
      const double   normal[3],     // IN : Plane normal
      const double   VF,            // IN : Volume fraction
      CVPolygon&       inter          // OUT: the EB
   )
   {
      unsigned char  specialCase = 0x0 ;
      double         component = 0.0 ;

         //  Encoding of the problem
         //  There are 4 (boolean) parameters:
         //     MSB - along X or Z
         //         - along Y
         //         - Normal component ( >0 or <0 )
         //     LSB - VF ( >0 or <0 )
      const unsigned char  axisMask = 0x8 ;
      const unsigned char  YaxisMask= 0x4 ;
      const unsigned char  compMask = 0x2 ;
      const unsigned char  vfMask   = 0x1 ;

// TODO perhaps find a way to use the iterators (not high priority)
      const int CaseTable[12][4] =
         {
            { 0, 2, 6, 4 },   // along Z
            { 3, 7, 5, 1 },
            { 1, 5, 7, 3 },
            { 4, 6, 2, 0 },
            { 0, 4, 5, 1 },   // along Y
            { 2, 6, 7, 3 },
            { 3, 7, 6, 2 },
            { 1, 5, 4, 0 },
            { 0, 1, 3, 2 },   // along X
            { 5, 7, 6, 4 },
            { 4, 6, 7, 5 },
            { 2, 3, 1, 0 }
         } ;

      const int FaceTable[12] =
         {
            -5,   // along Z
            -6,
             6,
             5,
            -3,   // along Y
            -4,
             4,
             3,
            -1,   // along X
            -2,
             2,
             1
         } ;

      int alongX, alongY, alongZ ;

      alongX = ! _equals( normal[0], 0.0, _axisAlignTol )
               && _equals( normal[1], 0.0, _axisAlignTol )
               && _equals( normal[2], 0.0, _axisAlignTol ) ;
      alongY = _equals( normal[0], 0.0, _axisAlignTol )
               && ! _equals( normal[1], 0.0, _axisAlignTol )
               && _equals( normal[2], 0.0, _axisAlignTol ) ;
      alongZ = _equals( normal[0], 0.0, _axisAlignTol )
               && _equals( normal[1], 0.0, _axisAlignTol )
               && ! _equals( normal[2], 0.0, _axisAlignTol ) ;

      if( (!alongX) && (!alongY) && (!alongZ) )
      {
         return 0 ;
      }

      if( alongX )
      {
         specialCase |= axisMask ;
         component = normal[0] ;
      }
      else
      if( alongY )
      {
         specialCase |= YaxisMask ;
         component = normal[1] ;
      }
      else
      if( alongZ )
      {
         component = normal[2] ;
      }
      else
      {
         cerr << "PerformSpecialCaseAnalysis3D: unknown case" << endl ;
         exit( 1 ) ;
      }

      if( VF > 0.0 )
      {
         specialCase |= vfMask ;
      }
      if( component > 0.0 )
      {
         specialCase |= compMask ;
      }

      if( specialCase >= 12 )
      {
         cerr << "PerformSpecialCaseAnalysis3D: unknown case" << endl ;
         exit( 1 ) ;
      }

      Point polyPts[4] ;

      polyPts[0] = _cell3D[ CaseTable[specialCase][0] ] ;
      polyPts[1] = _cell3D[ CaseTable[specialCase][1] ] ;
      polyPts[2] = _cell3D[ CaseTable[specialCase][2] ] ;
      polyPts[3] = _cell3D[ CaseTable[specialCase][3] ] ;

      inter = CVPolygon( polyPts, 4 ) ;

      _specCaseFace = FaceTable[specialCase] ;

      return 1 ;
   }

// ============================================================
//    Set Routines
// ============================================================

void EBSingleCell :: gridMode ( int m )
   {
      switch( m )
      {
         case EBSINGLE::GRID2D :
         case EBSINGLE::GRID3D :
            _gridMode = m ;
            return ;
         default :
            cerr  << "EBSingleCell::gridMode(): "
                  << "invalid grid mode" << endl ;
      }
   }

void EBSingleCell :: constraintMode ( int m )
   {
      switch( m )
      {
         case EBSINGLE::CONSTRAIN_VF_AF :
         case EBSINGLE::CONSTRAIN_VF_NORMAL :
         case EBSINGLE::CONSTRAIN_DISTANCE :
            _constraintMode = m ;
            break ;
         case EBSINGLE::CONSTRAIN_AF_LS_SUM :
            cerr  << "EBSingleCell::constraintMode(): "
                  << "constraint mode not yet implemented" << endl ;
            break ;
         default :
            cerr  << "EBSingleCell::constraintMode(): "
                  << "invalid constraint mode" << endl ;
      }
   }

void EBSingleCell :: tolerance ( double t )
   {
      if( t >= 0.0 )
      {
         _finder.tolerance( t ) ;
         return ;
      }
      cerr  << "EBSingleCell::tolerance(): "
            << "tolerance should be >= 0.0" << endl ;
   }

void EBSingleCell :: axisAlignTolerance ( double t )
   {
      if( t >= 0.0 )
      {
         _axisAlignTol = t ;
         return ;
      }
      cerr  << "EBSingleCell::axisAlignTolerance(): "
            << "tolerance should be >= 0.0" << endl ;
   }

void EBSingleCell :: setCellDimensions ( double x, double y, double z )
   {
      if( _gridMode == EBSINGLE::GRID3D )
      {
         Point corners[8] =
         {
            Point( 0.0, 0.0, 0.0 ),
            Point( 0.0, 0.0,   z ),
            Point( 0.0,   y, 0.0 ),
            Point( 0.0,   y,   z ),
            Point(   x, 0.0, 0.0 ),
            Point(   x, 0.0,   z ),
            Point(   x,   y, 0.0 ),
            Point(   x,   y,   z )
         } ;

         _cell3D = Hexahedron( corners ) ;
         //_cellVol = x * y * z ;
         //_compute3D.cellVolume( _cellVol ) ;
         _compute3D.cellVolume( x * y * z ) ;
      }
      else
      {
         Point corners[4] =
         {
            Point( 0.0, 0.0, 0.0 ),
            Point(   x, 0.0, 0.0 ),
            Point(   x,   y, 0.0 ),
            Point( 0.0,   y, 0.0 )
         } ;

         _cell2D = CVPolygon( corners, 4 ) ;
         //_cellVol = x * y ;
         //_compute2D.cellVolume( _cellVol ) ;
         _compute2D.cellVolume( x * y ) ;
      }
   }

// ============================================================
//    Access Routines
// ============================================================

int EBSingleCell :: gridMode ()
   {
      return _gridMode ;
   }

int EBSingleCell :: constraintMode ()
   {
      return _constraintMode ;
   }

double EBSingleCell :: tolerance ()
   {
      return _finder.tolerance() ;
   }

double EBSingleCell :: axisAlignTolerance ()
   {
      return _axisAlignTol ;
   }

void EBSingleCell :: getCellDimensions ( double& x, double& y, double& z )
   {
      if( _gridMode == EBSINGLE::GRID3D )
      {
         x = _cell3D[7].x() ;
         y = _cell3D[7].y() ;
         z = _cell3D[7].z() ;
      }
      else
      {
         x = _cell2D[2].x() ;
         y = _cell2D[2].y() ;
         z = 0.0 ;
      }
   }

Plane EBSingleCell :: ebPlane ()
   {
      switch( _gridMode )
      {
         case EBSINGLE::GRID2D :
            if( _didSpecialCase )
            {
               const Plane PlaneTable[4] =
                  {
                     Plane( _cell2D[0], KIJVector(  0,-1, 0 ) ),
                     Plane( _cell2D[1], KIJVector(  1, 0, 0 ) ),
                     Plane( _cell2D[2], KIJVector(  0, 1, 0 ) ),
                     Plane( _cell2D[3], KIJVector( -1, 0, 0 ) )
                  } ;
               Plane  spPlane = PlaneTable[ abs(_specCaseFace)-1 ] ;
               KIJVector norm = spPlane.direction() ;
               if( norm.x() )
               {
                  if( _specCaseFace < 0 ) norm.x( - fabs(norm.x()) ) ;
                  else                    norm.x( fabs(norm.x()) ) ;
               }
               else
               if( norm.y() )
               {
                  if( _specCaseFace < 0 ) norm.y( - fabs(norm.y()) ) ;
                  else                    norm.y( fabs(norm.y()) ) ;
               }
               spPlane.SetDirection( norm ) ;
               return spPlane ;
            }
            //return _compute2D.lastPlane() ;
            return _lastPlane ;
         case EBSINGLE::GRID3D :
            if( _didSpecialCase )
            {
               CVPolygon face ;
               _cell3D.Face( abs(_specCaseFace)-1, face ) ;
               Plane  spPlane(face) ;
               KIJVector norm = spPlane.direction() ;
               if( norm.x() )
               {
                  if( _specCaseFace < 0 ) norm.x( - fabs(norm.x()) ) ;
                  else                    norm.x( fabs(norm.x()) ) ;
               }
               else
               if( norm.y() )
               {
                  if( _specCaseFace < 0 ) norm.y( - fabs(norm.y()) ) ;
                  else                    norm.y( fabs(norm.y()) ) ;
               }
               else
               if( norm.z() )
               {
                  if( _specCaseFace < 0 ) norm.z( - fabs(norm.z()) ) ;
                  else                    norm.z( fabs(norm.z()) ) ;
               }
               spPlane.SetDirection( norm ) ;
               return spPlane ;
            }
            //return _compute3D.lastPlane() ;
            return _lastPlane ;
      }
      cerr  << "EBSingleCell::ebPlane(): "
            << "invalid grid mode" << endl ;
      //return _compute3D.lastPlane() ;
      return _lastPlane ;
   }

EBSingleCell::IntArray EBSingleCell :: facesIntersected ()
   {
      if( _didSpecialCase )
      {
         int i, ebFace ;
         IntArray faceIndices ;
         const int ifaces2D[][2] =
            {
               { 1, 3 },
               { 2, 0 },
               { 3, 1 },
               { 0, 2 }
            } ;
         const int ifaces3D[][4] =
            {
               { 2, 5, 3, 4 },   // A
               { 4, 3, 5, 2 },   // B
               { 1, 5, 0, 4 },   // C
               { 4, 0, 5, 1 },   // D
               { 0, 3, 1, 2 },   // E
               { 2, 1, 3, 0 }    // F
            } ;
         ebFace = abs(_specCaseFace)-1 ;
         switch( _gridMode )
         {
            case EBSINGLE::GRID2D :
               for( i=0; i<2; i++ )
               {
                  faceIndices.push_back( ifaces2D[ebFace][i] ) ;
               }
               break ;
            case EBSINGLE::GRID3D :
               for( i=0; i<4; i++ )
               {
                  faceIndices.push_back( ifaces3D[ebFace][i] ) ;
               }
               break ;
         }
         return faceIndices ;
      }
      return _slicer.facesIntersected() ;
   }

Point EBSingleCell :: intersectionCentroid ()
   {
      return _slicer.centroid() ;
   }
