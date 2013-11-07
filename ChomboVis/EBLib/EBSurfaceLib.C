// ============================================================
//    EBSurfaceLib
//
//    (c) Christopher S. Co, 2002
// ============================================================

#include <iostream>
#include <vector>
#include "KIJVector.h"
#include "EBSurfaceLib.h"
#include "EBSingleCell.h"

using namespace std ;

// ============================================================
//    Class TriVar
// ============================================================

template<class Scalar>
Scalar& TriVar<Scalar> :: operator[] ( int i )
   {
      switch( i )
      {
         case 0 :    return x ;  break ;
         case 1 :    return y ;  break ;
         case 2 :    return z ;  break ;
         default :
            cerr  << "TriVar::operator[]: "
                  << "invalid index value - " << i << endl ;
      }
      return x ;
   }

// ============================================================
//    Class GridInfo
// ============================================================

// ============================================================
//    Constructors, Destructor
// ============================================================

GridInfo :: GridInfo ()
   {
      origin[0]   = origin[1] = origin[2] = 0.0 ;
      space[0]    = space[1]  = space[2]  = 1.0 ;
      dim[0]      = dim[1]    = dim[2]    = 0 ;
   }

// ============================================================
//    Class EBSurfaceGenerator
// ============================================================

// ============================================================
//    Constructors, Destructor
// ============================================================

EBSurfaceGenerator :: EBSurfaceGenerator ()
   {
      index = NULL ;
      _doCapping = 1 ;
   }

EBSurfaceGenerator :: ~EBSurfaceGenerator ()
   {
   }

// ============================================================
//    Internal Helpers
// ============================================================

void debug( int i, int j, int k, double * vf, double * af )
   {
cerr << i << " " << j << " " << k << endl ;
if( vf && af )
{
   cerr << *vf << endl << "( "
      << af[0] << ", "
      << af[1] << ", "
      << af[2] << ", "
      << af[3] << ", "
      << af[4] << ", "
      << af[5] << " )"
      << endl ;
}
   }

template<class Real>
Real * EBSurfaceGenerator :: _getAddress
   (
      Real *         start,   // IN : starting address
      unsigned int   stride,  // IN : size between objects
      int i, int j, int k     // IN : indices
   )
   {
      return (Real*)(((char*)(start)) + index->index( i,j,k ) * stride) ;
   }

template<class Real>
void EBSurfaceGenerator :: _setCellPtrs
   (
      Real *   vf,   // IN : VoF address
      Real *   af    // IN : AF or normal address
   )
   {
      int i ;
      const int cmode = _calculator.constraintMode() ;
      const int gmode = _calculator.gridMode() ;
      if(   cmode == EBSINGLE::CONSTRAIN_DISTANCE ||
            cmode == EBSINGLE::CONSTRAIN_VF_NORMAL )
      {
         _cvf = double( *vf ) ;
         if( gmode == EBSINGLE::GRID2D )
         {
            for( i=0; i<2; i++ )
            {
               _caf[i] = double( af[i] ) ;
            }
         }
         else
         {
            for( i=0; i<3; i++ )
            {
               _caf[i] = double( af[i] ) ;
            }
         }
      }
      else
      {
         _cvf = double( *vf ) ;
         if( gmode == EBSINGLE::GRID2D )
         {
            for( i=0; i<4; i++ )
            {
               _caf[i] = double( af[i] ) ;
            }
         }
         else
         {
            for( i=0; i<6; i++ )
            {
               _caf[i] = double( af[i] ) ;
            }
         }
      }
   }

void EBSurfaceGenerator :: _transform
   (
      int i, int j, int k, // IN    : location of cell
      LineSegment& eb      // IN/OUT: object to translate
   )
   {
      KIJVector   translate
         (
            (i-0.5)*_grid.space.x+_grid.origin.x,
            (j-0.5)*_grid.space.y+_grid.origin.y
         ) ;

      eb += translate ;
   }

void EBSurfaceGenerator :: _transform
   (
      int i, int j, int k, // IN    : location of cell
      CVPolygon& eb          // IN/OUT: object to translate
   )
   {
      KIJVector   translate
         (
            (i-0.5)*_grid.space.x+_grid.origin.x,
            (j-0.5)*_grid.space.y+_grid.origin.y,
            (k-0.5)*_grid.space.z+_grid.origin.z
         ) ;

      eb += translate ;
   }

void EBSurfaceGenerator :: _transform
   (
      int i, int j, int k, // IN    : location of cell
      Plane&   ebPlane     // IN/OUT: object to translate
   )
   {
      KIJVector   translate
         (
            (i-0.5)*_grid.space.x+_grid.origin.x,
            (j-0.5)*_grid.space.y+_grid.origin.y
         ) ;

      ebPlane.SetAnchor( ebPlane.anchor() + translate ) ;
   }

void EBSurfaceGenerator :: _transform
   (
      int i, int j, int k, // IN    : location of cell
      LineSegment& eb,     // IN/OUT: object to translate
      Plane&   ebPlane     // IN/OUT: object to translate
   )
   {
      KIJVector   translate
         (
            (i-0.5)*_grid.space.x+_grid.origin.x,
            (j-0.5)*_grid.space.y+_grid.origin.y
         ) ;

      eb += translate ;
      ebPlane.SetAnchor( ebPlane.anchor() + translate ) ;
   }

void EBSurfaceGenerator :: _transform
   (
      int i, int j, int k, // IN    : location of cell
      CVPolygon& eb,         // IN/OUT: object to translate
      Plane&   ebPlane     // IN/OUT: object to translate
   )
   {
      KIJVector   translate
         (
            (i-0.5)*_grid.space.x+_grid.origin.x,
            (j-0.5)*_grid.space.y+_grid.origin.y,
            (k-0.5)*_grid.space.z+_grid.origin.z
         ) ;

      eb += translate ;
      ebPlane.SetAnchor( ebPlane.anchor() + translate ) ;
   }

void EBSurfaceGenerator :: _pushOntoArray ( LineSegment& line, int id1, int id2 )
   {
      int i ;
      for( i=0; i<line.size(); i++ )
      {
         Point* pt ;
         pt = &(line[i]) ;
         _surface.push_back( pt->x() ) ;
         _surface.push_back( pt->y() ) ;
      }
      _cellIDs.push_back( id1 ) ;
      _cellIDs.push_back( id2 ) ;
   }

void EBSurfaceGenerator :: _pushOntoArray ( CVPolygon& poly, int id1, int id2 )
   {
      _startAddress.push_back( _surface.size() ) ;

      unsigned int i ;
      for( i=0; i<poly.size(); i++ )
      {
         Point* pt ;
         pt = &(poly[i]) ;
         _surface.push_back( pt->x() ) ;
         _surface.push_back( pt->y() ) ;
         _surface.push_back( pt->z() ) ;
      }

      _cellIDs.push_back( id1 ) ;
      _cellIDs.push_back( id2 ) ;
   }

void EBSurfaceGenerator :: _pushOntoArray ( LineArray& lines, int id1, int id2 )
   {
      unsigned int i ;
      for( i=0; i<lines.size(); i++ )
      {
         _pushOntoArray( lines[i], id1, id2 ) ;
      }
   }

void EBSurfaceGenerator :: _pushOntoArray ( PolyArray& polys, int id1, int id2 )
   {
      unsigned int i ;
      for( i=0; i<polys.size(); i++ )
      {
         _pushOntoArray( polys[i], id1, id2 ) ;
      }
   }

int EBSurfaceGenerator :: _IsValidCell ( double * norm, int n )
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

const char  I1 = 0x10 ;
const char  R1 = 0x20 ;
const char  C1 = 0x40 ;
const char  I2 = 0x01 ;
const char  R2 = 0x02 ;
const char  C2 = 0x04 ;
const char  mask2 = 0xf0 ;

template<class Real>
int EBSurfaceGenerator :: _computeSingleCell2D
   (
      Real * VF,
      Real * Dist,
      Real * AF,
      unsigned int VFsize,
      unsigned int DIsize,
      unsigned int AFsize,
      int i, int j,
      int cFromCache,
      IntArray&   faces,
      IntArray&   fromCache,
      EBCache&    cCache,
      EBCache&    nCache
   )
   {
      unsigned int c ;
      int dx,dy ;
      int ci,cj ;
      int hasPlane = 0, savedHasPlane = 0 ;
      Real * vf, * af ;
      LineSegment cellEB ;
      EBCacheItem   cellEBplane ;
      EBCacheItem   thisCellPlane ;
      PlaneArray  ebPlanes ;
      LineArray   caps ;
      CVPolygon& cell = _calculator.cell2D() ;
      char cappingCode = 0 ;

         // Identify first cell type
      vf = _getAddress( VF, VFsize, i,j,0 ) ;
      if( _IsRegular( *vf ) )
         cappingCode |= R1 ;
      else
      if( _IsCovered( *vf ) )
         cappingCode |= C1 ;
      else
         cappingCode |= I1 ;


      if( cFromCache )
      {
         if( cCache.getPlane( i,j,0, thisCellPlane ) )
         {
            hasPlane++ ;
         }
      }
      else
      {
         if( _calculator.constraintMode() == EBSINGLE::CONSTRAIN_DISTANCE )
            vf = _getAddress( Dist, DIsize, i,j,0 ) ;
         else
            vf = _getAddress( VF, VFsize, i,j,0 ) ;
         af = _getAddress( AF, AFsize, i,j,0 ) ;
         _setCellPtrs( vf, af ) ;
         if( _calculator.compute( _cvf, _caf, cellEB ) )
         {
            thisCellPlane.plane = _calculator.ebPlane() ;
            thisCellPlane.faces = _calculator.facesIntersected() ;
            _transform( i,j,0, cellEB, thisCellPlane.plane ) ;
            _pushOntoArray( cellEB, -1, index->index(i,j,0) ) ;
            cCache.setPlane( i,j,0, thisCellPlane ) ;
            hasPlane++ ;
         }
      }

      savedHasPlane = hasPlane ;

//cerr << "Current: " ;
//debug( i,j,2, NULL,NULL ) ;
//cerr << "planes: " << hasPlane << endl ;
      for( c=0; c<faces.size(); c++ )
      {
         hasPlane = savedHasPlane ;
         ebPlanes.clear() ;
         if( hasPlane > 0 )
            ebPlanes.push_back( thisCellPlane.plane ) ;

            // Get adjacent plane   
         _capper.adjCell( faces[c], dx,dy ) ;
         ci = i+dx ;
         cj = j+dy ;

            // Clear previous neighbor cell's info
         cappingCode &= mask2 ;
            // Identify second cell type
         vf = _getAddress( VF, VFsize, ci,cj,0 ) ;
         if( _IsRegular( *vf ) )
            cappingCode |= R2 ;
         else
         if( _IsCovered( *vf ) )
            cappingCode |= C2 ;
         else
            cappingCode |= I2 ;

//cerr << "Face " << faces[c] << " : " ;
//debug( ci,cj,2, NULL,NULL ) ;
         if( fromCache[c] )
         {     // Obtain from appropriate cache
            if( dx )
            {
               if( nCache.getPlane( ci,cj,0, cellEBplane ) )
               {
                  hasPlane++ ;
                  ebPlanes.push_back( cellEBplane.plane ) ;
               }
            }
            else
            {
               if( cCache.getPlane( ci,cj,0, cellEBplane ) )
               {
                  hasPlane++ ;
                  ebPlanes.push_back( cellEBplane.plane ) ;
               }
            }
         }
         else
         {     // Calculate the cell and
               //    store to appropriate cache
            if( _calculator.constraintMode() == EBSINGLE::CONSTRAIN_DISTANCE )
               vf = _getAddress( Dist, DIsize, ci,cj,0 ) ;
            else
               vf = _getAddress( VF, VFsize, ci,cj,0 ) ;
            af = _getAddress( AF, AFsize, ci,cj,0 ) ;
            _setCellPtrs( vf, af ) ;
            if( _calculator.compute( _cvf, _caf, cellEB ) )
            {
               cellEBplane.plane = _calculator.ebPlane() ;
               cellEBplane.faces = _calculator.facesIntersected() ;
               _transform( ci,cj,0, cellEB, cellEBplane.plane ) ;
               _pushOntoArray( cellEB, -1, index->index( ci,cj,0 ) ) ;
               ebPlanes.push_back( cellEBplane.plane ) ;
               hasPlane++ ;
               if( dx )
               {
                  nCache.setPlane( ci,cj,0, cellEBplane ) ;
               }
               else
               {
                  cCache.setPlane( ci,cj,0, cellEBplane ) ;
               }
            }
         }
//cerr << "planes: " << hasPlane << " ebPlanes size: " << ebPlanes.size() << endl ;
            // Cap if necessary
         if( _doCapping )
         {
            LineSegment face ;
            cell.Edge( faces[c], face ) ;
            _transform( i,j,0, face ) ;
            if(   cappingCode == ( I1 | R2 ) ||
                  cappingCode == ( R1 | I2 ) )
            {
                  // Add extra face as plane
// TODO find a way to get the plane!!!
               Point poly[3] =
                  {
                     face.left(), face.right(), face.right()+VZ
                  } ;
               CVPolygon plane( poly, 3 ) ;
               ebPlanes.push_back( Plane( plane ) ) ;
            }
            if( (cappingCode & I1) || (cappingCode & I2) )
            {
                  // Perform capping
               caps.clear() ;
               if( _capper.cap( face, ebPlanes, caps ) )
               {
                  _pushOntoArray(
                     caps,
                     index->index( i,j,0 ),
                     index->index( ci,cj,0 ) ) ;
               }
            }

/*
            if( hasPlane > 0 && cellIsCovered == 0 && firstCellIsCovered == 0 )
            {
               LineSegment face ;
               cell.Edge( faces[c], face ) ;
               _transform( i,j,0, face ) ;
               caps.clear() ;
               if( _capper.cap( face, ebPlanes, caps ) )
               {
                  _pushOntoArray(
                     caps,
                     index->index( i,j,0 ),
                     index->index( ci,cj,0 ) ) ;
               }
            }
*/
         }
      }
//cerr << endl ;

      return 1 ;
   }

// faces[0] and fromCache[0] always exist and refer to current cell
// others refer to neighbors
// codes indicate whether to get EB data from:
//    computation
//    cache
template<class Real>
int EBSurfaceGenerator :: _computeSingleCell3D
   (
      Real * VF,
      Real * Dist,
      Real * AF,
      unsigned int VFsize,
      unsigned int DIsize,
      unsigned int AFsize,
      int i, int j, int k,
      int cFromCache,
      IntArray&   faces,
      IntArray&   fromCache,
      EBCache&    cCache,
      EBCache&    nCache
   )
   {
      unsigned int c ;
      int dx,dy,dz ;
      int ci,cj,ck ;
      int hasPlane = 0, savedHasPlane = 0 ;
      //int firstCellIsRegular = 0, cellHasNoGeometry = 0 ;
      Real * vf, * af ;
      CVPolygon cellEB ;
      EBCacheItem   cellEBplane ;
      EBCacheItem   thisCellPlane ;
      PlaneArray  ebPlanes ;
      PolyArray   caps ;
      Hexahedron& cell = _calculator.cell3D() ;
      char cappingCode = 0 ;

         // Identify first cell type
      vf = _getAddress( VF, VFsize, i,j,k ) ;
      if( _IsRegular( *vf ) )
         cappingCode |= R1 ;
      else
      if( _IsCovered( *vf ) )
         cappingCode |= C1 ;
      else
         cappingCode |= I1 ;

      if( cFromCache )
      {
         if( cCache.getPlane( i,j,k, thisCellPlane ) )
         {
            hasPlane++ ;
         }
      }
      else
      {
         if( _calculator.constraintMode() == EBSINGLE::CONSTRAIN_DISTANCE )
            vf = _getAddress( Dist, DIsize, i,j,k ) ;
         else
            vf = _getAddress( VF, VFsize, i,j,k ) ;
         af = _getAddress( AF, AFsize, i,j,k ) ;
         _setCellPtrs( vf, af ) ;
         if( _calculator.compute( _cvf, _caf, cellEB ) )
         {
            thisCellPlane.plane = _calculator.ebPlane() ;
            thisCellPlane.faces = _calculator.facesIntersected() ;
            _transform( i,j,k, cellEB, thisCellPlane.plane ) ;
            _pushOntoArray( cellEB, -1, index->index( i,j,k ) ) ;
            cCache.setPlane( i,j,k, thisCellPlane ) ;
            hasPlane++ ;
         }
      }

      savedHasPlane = hasPlane ;

      for( c=0; c<faces.size(); c++ )
      {
         hasPlane = savedHasPlane ;
         ebPlanes.clear() ;
         if( hasPlane > 0 )
            ebPlanes.push_back( thisCellPlane.plane ) ;

            // Get adjacent plane   
         _capper.adjCell( faces[c], dx,dy,dz ) ;
         ci = i+dx ;
         cj = j+dy ;
         ck = k+dz ;

            // Clear previous neighbor cell's info
         cappingCode &= mask2 ;
            // Identify neighbor cell type
         vf = _getAddress( VF, VFsize, ci,cj,ck ) ;
         if( _IsRegular( *vf ) )
            cappingCode |= R2 ;
         else
         if( _IsCovered( *vf ) )
            cappingCode |= C2 ;
         else
            cappingCode |= I2 ;

         if( fromCache[c] )
         {     // Obtain from appropriate cache
            if( dx )
            {
               if( nCache.getPlane( ci,cj,ck, cellEBplane ) )
               {
                  hasPlane++ ;
                  ebPlanes.push_back( cellEBplane.plane ) ;
               }
            }
            else
            {
               if( cCache.getPlane( ci,cj,ck, cellEBplane ) )
               {
                  hasPlane++ ;
                  ebPlanes.push_back( cellEBplane.plane ) ;
               }
            }
         }
         else
         {     // Calculate the cell and
               //    store to appropriate cache
            if( _calculator.constraintMode() == EBSINGLE::CONSTRAIN_DISTANCE )
               vf = _getAddress( Dist, DIsize, ci,cj,ck ) ;
            else
               vf = _getAddress( VF, VFsize, ci,cj,ck ) ;
            af = _getAddress( AF, AFsize, ci,cj,ck ) ;
            _setCellPtrs( vf, af ) ;

            if( _calculator.compute( _cvf, _caf, cellEB ) )
            {
               cellEBplane.plane = _calculator.ebPlane() ;
               cellEBplane.faces = _calculator.facesIntersected() ;
               _transform( ci,cj,ck, cellEB, cellEBplane.plane ) ;
               _pushOntoArray( cellEB, -1, index->index( ci,cj,ck ) ) ;
               ebPlanes.push_back( cellEBplane.plane ) ;
               hasPlane++ ;
               if( dx )
               {
                  nCache.setPlane( ci,cj,ck, cellEBplane ) ;
               }
               else
               {
                  cCache.setPlane( ci,cj,ck, cellEBplane ) ;
               }
            }
         }
            // Cap if necessary
         if( _doCapping )
         {
            CVPolygon face ;
            cell.Face( faces[c], face ) ;
            _transform( i,j,k, face ) ;
            if(   cappingCode == ( I1 | R2 ) ||
                  cappingCode == ( R1 | I2 ) )
            {
                  // Add extra face as plane
               ebPlanes.push_back( -Plane( face ) ) ;
            }
            //if( (cappingCode & I1) || (cappingCode & I2) )
            {
                  // Perform capping
               caps.clear() ;
               if( _capper.cap( face, ebPlanes, caps ) )
               {
                  _pushOntoArray(
                     caps,
                     index->index( i,j,k ),
                     index->index( ci,cj,ck ) ) ;
               }
            }
         }
      } // for each face to cap

      return 1 ;
   }

template<class Real>
int EBSurfaceGenerator :: _compute2D
   (
      Real * VF,              // IN : VF data
      Real * Dist,            // IN : Dist data
      Real * AF,              // IN : AF data
      unsigned int   VFsize,  // IN : VF stride
      unsigned int   DIsize,  // IN : Dist stride
      unsigned int   AFsize   // IN : AF stride
   )
   {
      int i,j ;
      const int LASTX = _grid.dim.x - 1 ;
      const int LASTY = _grid.dim.y - 1 ;
      int curr, next ;

      typedef struct
      {
         IntArray faces2cap ;
         IntArray neighFromCache ;
         int      currFromCache ;
      } cellCapInfo ;

         // Bit masks for case table
      const int I_FIRST    = 001 ;
      const int I_MID      = 002 ;
      const int I_LAST     = 004 ;
      const int J_FIRST    = 010 ;
      const int J_MID      = 020 ;
      const int J_LAST     = 040 ;
         // Bit masks used for clearing the variable.
      //const int I_MASK     = 070 ;
      const int J_MASK     = 007 ;

      //const int F_LEFT     = 3 ;
      const int F_RIGHT    = 1 ;
      //const int F_BOTTOM   = 0 ;
      const int F_TOP      = 2 ;

         // The case table
         //    We use 6 bits to encode the info for each cell.
         //    We thus only need 2^6 = 64 entries of type cellCapInfo
      cellCapInfo capInfo[64] ;

      int index ;
      // i == 0      j == 0
      {
         index = I_FIRST | J_FIRST ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 0 ;
         capInfo[index].currFromCache = 0 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == 0      j == MID
      {
         index = I_FIRST | J_MID ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 0 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == 0      j == LAST
      {
         index = I_FIRST | J_LAST ;
         IntArray facesToCap(1) ;
         IntArray neighFromCache(1) ;
      
         facesToCap[0] = F_RIGHT ;
         neighFromCache[0] = 0 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == MID    j == 0
      {
         index = I_MID | J_FIRST ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == MID    j == MID
      {
         index = I_MID | J_MID ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == MID    j == LAST
      {
         index = I_MID | J_LAST ;
         IntArray facesToCap(1) ;
         IntArray neighFromCache(1) ;
      
         facesToCap[0] = F_RIGHT ;
         neighFromCache[0] = 0 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == LAST   j == 0
      {
         index = I_LAST | J_FIRST ;
         IntArray facesToCap(1) ;
         IntArray neighFromCache(1) ;
      
         facesToCap[0] = F_TOP ;
         neighFromCache[0] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == LAST   j == MID
      {
         index = I_LAST | J_MID ;
         IntArray facesToCap(1) ;
         IntArray neighFromCache(1) ;
      
         facesToCap[0] = F_TOP ;
         neighFromCache[0] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == LAST   j == LAST
      {
         index = I_LAST | J_LAST ;
         IntArray facesToCap(0) ;
         IntArray neighFromCache(0) ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }

      curr = 0 ;
      next = 1 ;
      _cache[curr].setDim( 1, _grid.dim.y, 1 ) ;
      _cache[next].setDim( 1, _grid.dim.y, 1 ) ;

      int code ;
      for( i=0; i<_grid.dim.x; i++ )
      {
         _cache[curr].setOrigin( i  , 0, 0 ) ;
         _cache[next].setOrigin( i+1, 0, 0 ) ;

         code = 0 ;
         if( i == 0 )            code |= I_FIRST ;
         else if( i == LASTX )   code |= I_LAST ;
         else                    code |= I_MID ;

         for( j=0; j<_grid.dim.y; j++ )
         {
            code &= J_MASK ;
            if( j == 0 )            code |= J_FIRST ;
            else if( j == LASTY )   code |= J_LAST ;
            else                    code |= J_MID ;

            IntArray facesToCap ;
            IntArray neighFromCache ;
            int currFromCache ;

            facesToCap     = capInfo[ code ].faces2cap ;
            neighFromCache = capInfo[ code ].neighFromCache ;
            currFromCache  = capInfo[ code ].currFromCache ;

            _computeSingleCell2D
               (
                  VF, Dist, AF, VFsize, DIsize, AFsize,
                  i,j, currFromCache,
                  facesToCap,
                  neighFromCache,
                  _cache[curr],
                  _cache[next]
               ) ;

         }

         curr = next ;
         next = (next+1)%2 ;
         _cache[next].reset() ;
      } // end for( i )

      return 1 ;
   }

template<class Real>
int EBSurfaceGenerator :: _compute3D
   (
      Real * VF,              // IN : VF data
      Real * Dist,            // IN : Dist data
      Real * AF,              // IN : AF data
      unsigned int   VFsize,  // IN : VF stride
      unsigned int   DIsize,  // IN : Dist stride
      unsigned int   AFsize   // IN : AF stride
   )
   {
      int i,j,k ;
      const int LASTX = _grid.dim.x - 1 ;
      const int LASTY = _grid.dim.y - 1 ;
      const int LASTZ = _grid.dim.z - 1 ;
      int curr, next ;

      typedef struct
      {
         IntArray faces2cap ;
         IntArray neighFromCache ;
         int      currFromCache ;
      } cellCapInfo ;

         // Bit masks for case table
      const int I_FIRST    = 0001 ;
      const int I_MID      = 0002 ;
      const int I_LAST     = 0004 ;
      const int J_FIRST    = 0010 ;
      const int J_MID      = 0020 ;
      const int J_LAST     = 0040 ;
      const int K_FIRST    = 0100 ;
      const int K_MID      = 0200 ;
      const int K_LAST     = 0400 ;
         // Bit masks used for clearing the variable.
      //const int I_MASK     = 0770 ;
      const int J_MASK     = 0707 ;
      const int K_MASK     = 0077 ;

      //const int F_LEFT     = 0 ;
      const int F_RIGHT    = 1 ;
      //const int F_BOTTOM   = 2 ;
      const int F_TOP      = 3 ;
      //const int F_BACK     = 4 ;
      const int F_FRONT    = 5 ;

         // The case table
         //    We use 9 bits to encode the info for each cell.
         //    We thus only need 2^9 = 512 entries of type cellCapInfo
      cellCapInfo capInfo[512] ;

         // Initialize the capInfo table
      int index ;
      // i == 0      j == 0      k == 0
      {
         index = I_FIRST | J_FIRST | K_FIRST ;
         IntArray facesToCap(3) ;
         IntArray neighFromCache(3) ;

         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         facesToCap[2] = F_FRONT ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 0 ;
         neighFromCache[2] = 0 ;
         capInfo[index].currFromCache = 0 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == 0      j == 0      k == MID
      {
         index = I_FIRST | J_FIRST | K_MID ;
         IntArray facesToCap(3) ;
         IntArray neighFromCache(3) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         facesToCap[2] = F_FRONT ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 0 ;
         neighFromCache[2] = 0 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == 0      j == 0      k == LAST
      {
         index = I_FIRST | J_FIRST | K_LAST ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 0 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == 0      j == MID    k == 0
      {
         index = I_FIRST | J_MID | K_FIRST ;
         IntArray facesToCap(3) ;
         IntArray neighFromCache(3) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         facesToCap[2] = F_FRONT ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 0 ;
         neighFromCache[2] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == 0      j == MID    k == MID
      {
         index = I_FIRST | J_MID | K_MID ;
         IntArray facesToCap(3) ;
         IntArray neighFromCache(3) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         facesToCap[2] = F_FRONT ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 0 ;
         neighFromCache[2] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == 0      j == MID    k == LAST
      {
         index = I_FIRST | J_MID | K_LAST ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 0 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == 0      j == LAST   k == 0
      {
         index = I_FIRST | J_LAST | K_FIRST ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_FRONT ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == 0      j == LAST   k == MID
      {
         index = I_FIRST | J_LAST | K_MID ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_FRONT ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == 0      j == LAST   k == LAST
      {
         index = I_FIRST | J_LAST | K_LAST ;
         IntArray facesToCap(1) ;
         IntArray neighFromCache(1) ;
      
         facesToCap[0] = F_RIGHT ;
         neighFromCache[0] = 0 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == MID    j == 0      k == 0
      {
         index = I_MID | J_FIRST | K_FIRST ;
         IntArray facesToCap(3) ;
         IntArray neighFromCache(3) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         facesToCap[2] = F_FRONT ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 1 ;
         neighFromCache[2] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == MID    j == 0      k == MID
      {
         index = I_MID | J_FIRST | K_MID ;
         IntArray facesToCap(3) ;
         IntArray neighFromCache(3) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         facesToCap[2] = F_FRONT ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 1 ;
         neighFromCache[2] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == MID    j == 0      k == LAST
      {
         index = I_MID | J_FIRST | K_LAST ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == MID    j == MID    k == 0
      {
         index = I_MID | J_MID | K_FIRST ;
         IntArray facesToCap(3) ;
         IntArray neighFromCache(3) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         facesToCap[2] = F_FRONT ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 1 ;
         neighFromCache[2] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == MID    j == MID    k == MID
      {
         index = I_MID | J_MID | K_MID ;
         IntArray facesToCap(3) ;
         IntArray neighFromCache(3) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         facesToCap[2] = F_FRONT ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 1 ;
         neighFromCache[2] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == MID    j == MID    k == LAST
      {
         index = I_MID | J_MID | K_LAST ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_TOP ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == MID    j == LAST   k == 0
      {
         index = I_MID | J_LAST | K_FIRST ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_FRONT ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == MID    j == LAST   k == MID
      {
         index = I_MID | J_LAST | K_MID ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_RIGHT ;
         facesToCap[1] = F_FRONT ;
         neighFromCache[0] = 0 ;
         neighFromCache[1] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == MID    j == LAST   k == LAST
      {
         index = I_MID | J_LAST | K_LAST ;
         IntArray facesToCap(1) ;
         IntArray neighFromCache(1) ;
      
         facesToCap[0] = F_RIGHT ;
         neighFromCache[0] = 0 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == LAST   j == 0      k == 0
      {
         index = I_LAST | J_FIRST | K_FIRST ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_TOP ;
         facesToCap[1] = F_FRONT ;
         neighFromCache[0] = 1 ;
         neighFromCache[1] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == LAST   j == 0      k == MID
      {
         index = I_LAST | J_FIRST | K_MID ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_TOP ;
         facesToCap[1] = F_FRONT ;
         neighFromCache[0] = 1 ;
         neighFromCache[1] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == LAST   j == 0      k == LAST
      {
         index = I_LAST | J_FIRST | K_LAST ;
         IntArray facesToCap(1) ;
         IntArray neighFromCache(1) ;
      
         facesToCap[0] = F_TOP ;
         neighFromCache[0] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == LAST   j == MID    k == 0
      {
         index = I_LAST | J_MID | K_FIRST ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_TOP ;
         facesToCap[1] = F_FRONT ;
         neighFromCache[0] = 1 ;
         neighFromCache[1] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == LAST   j == MID    k == MID
      {
         index = I_LAST | J_MID | K_MID ;
         IntArray facesToCap(2) ;
         IntArray neighFromCache(2) ;
      
         facesToCap[0] = F_TOP ;
         facesToCap[1] = F_FRONT ;
         neighFromCache[0] = 1 ;
         neighFromCache[1] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == LAST   j == MID    k == LAST
      {
         index = I_LAST | J_MID | K_LAST ;
         IntArray facesToCap(1) ;
         IntArray neighFromCache(1) ;
      
         facesToCap[0] = F_TOP ;
         neighFromCache[0] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == LAST   j == LAST   k == 0
      {
         index = I_LAST | J_LAST | K_FIRST ;
         IntArray facesToCap(1) ;
         IntArray neighFromCache(1) ;
      
         facesToCap[0] = F_FRONT ;
         neighFromCache[0] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == LAST   j == LAST   k == MID
      {
         index = I_LAST | J_LAST | K_MID ;
         IntArray facesToCap(1) ;
         IntArray neighFromCache(1) ;
      
         facesToCap[0] = F_FRONT ;
         neighFromCache[0] = 1 ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }
      // i == LAST   j == LAST   k == LAST
      {
         index = I_LAST | J_LAST | K_LAST ;
         IntArray facesToCap(0) ;
         IntArray neighFromCache(0) ;
         capInfo[index].currFromCache = 1 ;
         capInfo[index].faces2cap = facesToCap ;
         capInfo[index].neighFromCache = neighFromCache ;
      }

         // Initialize the cache
      curr = 0 ;
      next = 1 ;
      _cache[curr].setDim( 1, _grid.dim.y, _grid.dim.z ) ;
      _cache[next].setDim( 1, _grid.dim.y, _grid.dim.z ) ;

      int code ;
      for( i=0; i<_grid.dim.x; i++ )
      {
         _cache[curr].setOrigin( i  , 0, 0 ) ;
         _cache[next].setOrigin( i+1, 0, 0 ) ;

         code = 0 ;
         if( i == 0 )            code |= I_FIRST ;
         else if( i == LASTX )   code |= I_LAST ;
         else                    code |= I_MID ;

         for( j=0; j<_grid.dim.y; j++ )
         {
            code &= J_MASK ;
            if( j == 0 )            code |= J_FIRST ;
            else if( j == LASTY )   code |= J_LAST ;
            else                    code |= J_MID ;

            for( k=0; k<_grid.dim.z; k++ )
            {
               code &= K_MASK ;
               if( k == 0 )            code |= K_FIRST ;
               else if( k == LASTZ )   code |= K_LAST ;
               else                    code |= K_MID ;

               IntArray facesToCap ;
               IntArray neighFromCache ;
               int currFromCache ;

               facesToCap     = capInfo[ code ].faces2cap ;
               neighFromCache = capInfo[ code ].neighFromCache ;
               currFromCache  = capInfo[ code ].currFromCache ;

               _computeSingleCell3D
                  (
                     VF, Dist, AF, VFsize, DIsize, AFsize,
                     i,j,k, currFromCache,
                     facesToCap,
                     neighFromCache,
                     _cache[curr],
                     _cache[next]
                  ) ;
            }
         }

            // Update caches
         curr = next ;
         next = (next+1)%2 ;
         _cache[next].reset() ;
      } // end for( i )

      return 1 ;
   }
/*
template<class Real>
int EBSurfaceGenerator :: _compute3D
   (
      Real * VF,              // IN : VF data
      Real * Dist,            // IN : Dist data
      Real * AF,              // IN : AF data
      unsigned int   VFsize,  // IN : VF stride
      unsigned int   DIsize,  // IN : Dist stride
      unsigned int   AFsize   // IN : AF stride
   )
   {
      int i,j,k ;
      const int LASTX = _grid.dim.x - 1 ;
      const int LASTY = _grid.dim.y - 1 ;
      const int LASTZ = _grid.dim.z - 1 ;
      int curr, next ;

      //const int F_LEFT     = 0 ;
      const int F_RIGHT    = 1 ;
      //const int F_BOTTOM   = 2 ;
      const int F_TOP      = 3 ;
      //const int F_BACK     = 4 ;
      const int F_FRONT    = 5 ;

      curr = 0 ;
      next = 1 ;
      _cache[curr].setDim( 1, _grid.dim.y, _grid.dim.z ) ;
      _cache[next].setDim( 1, _grid.dim.y, _grid.dim.z ) ;

      for( i=0; i<_grid.dim.x; i++ )
      {
         _cache[curr].setOrigin( i  , 0, 0 ) ;
         _cache[next].setOrigin( i+1, 0, 0 ) ;

         // ========================================
         //    First Sheet
         // ========================================
         if( i == 0 )
         {  // First Sheet
            for( j=0; j<_grid.dim.y; j++ )
            {
               if( j == 0 )
               {     // First Row
                  for( k=0; k<_grid.dim.z; k++ )
                  {
                     if( k == 0 )
                     {
                        // 4-6
                        IntArray facesToCap(3) ;
                        IntArray neighFromCache(3) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_TOP ;
                        facesToCap[2] = F_FRONT ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 0 ;
                        neighFromCache[2] = 0 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 0,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     if( k == LASTZ )
                     {
                        // 2-5 (a)
                        IntArray facesToCap(2) ;
                        IntArray neighFromCache(2) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_TOP ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 0 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     {
                        // 3-5
                        IntArray facesToCap(3) ;
                        IntArray neighFromCache(3) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_TOP ;
                        facesToCap[2] = F_FRONT ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 0 ;
                        neighFromCache[2] = 0 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                  } // end for ( k )
               }
               else
               if( j == LASTY )
               {     // Last Row
                  for( k=0; k<_grid.dim.z; k++ )
                  {
                     if( k == 0 )
                     {
                        // 1-5
                        IntArray facesToCap(2) ;
                        IntArray neighFromCache(2) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_FRONT ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     if( k == LASTZ )
                     {
                        IntArray facesToCap(1) ;
                        IntArray neighFromCache(1) ;

                        facesToCap[0] = F_RIGHT ;
                        neighFromCache[0] = 0 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     {
                        // 1-4
                        IntArray facesToCap(2) ;
                        IntArray neighFromCache(2) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_FRONT ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                  } // end for ( k )
               }
               else
               {     // Middle Rows
                  for( k=0; k<_grid.dim.z; k++ )
                  {
                     if( k == 0 )
                     {
                        // 2-5 (b)
                        IntArray facesToCap(3) ;
                        IntArray neighFromCache(3) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_TOP ;
                        facesToCap[2] = F_FRONT ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 0 ;
                        neighFromCache[2] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     if( k == LASTZ )
                     {
                        IntArray facesToCap(2) ;
                        IntArray neighFromCache(2) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_TOP ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 0 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     {
                        // 2-4
                        IntArray facesToCap(3) ;
                        IntArray neighFromCache(3) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_TOP ;
                        facesToCap[2] = F_FRONT ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 0 ;
                        neighFromCache[2] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                  } // end for ( k )
               }
            } // end for ( j )
         }
         else
         // ========================================
         //    Last Sheet
         // ========================================
         if( i == LASTX )
         {  // Last Sheet
            for( j=0; j<_grid.dim.y; j++ )
            {
               if( j == 0 )
               {     // First Row
                  for( k=0; k<_grid.dim.z; k++ )
                  {
                     if( k == 0 )
                     {
                        // 0-5
                        IntArray facesToCap(2) ;
                        IntArray neighFromCache(2) ;

                        facesToCap[0] = F_TOP ;
                        facesToCap[1] = F_FRONT ;
                        neighFromCache[0] = 1 ;
                        neighFromCache[1] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     if( k == LASTZ )
                     {
                        IntArray facesToCap(1) ;
                        IntArray neighFromCache(1) ;

                        facesToCap[0] = F_TOP ;
                        neighFromCache[0] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     {
                        // 0-4 (a)
                        IntArray facesToCap(2) ;
                        IntArray neighFromCache(2) ;

                        facesToCap[0] = F_TOP ;
                        facesToCap[1] = F_FRONT ;
                        neighFromCache[0] = 1 ;
                        neighFromCache[1] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                  } // end for ( k )
               }
               else
               if( j == LASTY )
               {     // Last Row
                  for( k=0; k<_grid.dim.z; k++ )
                  {
                     if( k == 0 )
                     {
                        IntArray facesToCap(1) ;
                        IntArray neighFromCache(1) ;

                        facesToCap[0] = F_FRONT ;
                        neighFromCache[0] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     if( k == LASTZ )
                     {
                        IntArray facesToCap(0) ;
                        IntArray neighFromCache(0) ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     {
                        IntArray facesToCap(1) ;
                        IntArray neighFromCache(1) ;

                        facesToCap[0] = F_FRONT ;
                        neighFromCache[0] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                  } // end for ( k )
               }
               else
               {     // Middle Rows
                  for( k=0; k<_grid.dim.z; k++ )
                  {
                     if( k == 0 )
                     {
                        // 0-4 (b)
                        IntArray facesToCap(2) ;
                        IntArray neighFromCache(2) ;

                        facesToCap[0] = F_TOP ;
                        facesToCap[1] = F_FRONT ;
                        neighFromCache[0] = 1 ;
                        neighFromCache[1] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     if( k == LASTZ )
                     {
                        IntArray facesToCap(1) ;
                        IntArray neighFromCache(1) ;

                        facesToCap[0] = F_TOP ;
                        neighFromCache[0] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     {
                        // 0-3
                        IntArray facesToCap(2) ;
                        IntArray neighFromCache(2) ;

                        facesToCap[0] = F_TOP ;
                        facesToCap[1] = F_FRONT ;
                        neighFromCache[0] = 1 ;
                        neighFromCache[1] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                  } // end for ( k )
               }
            } // end for ( j )
         }
         else
         // ========================================
         //    Middle Sheets
         // ========================================
         {  // Middle Sheets
            for( j=0; j<_grid.dim.y; j++ )
            {
               if( j == 0 )
               {     // First Row
                  for( k=0; k<_grid.dim.z; k++ )
                  {
                     if( k == 0 )
                     {
                        // 1-5
                        IntArray facesToCap(3) ;
                        IntArray neighFromCache(3) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_TOP ;
                        facesToCap[2] = F_FRONT ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 1 ;
                        neighFromCache[2] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     if( k == LASTZ )
                     {
                        IntArray facesToCap(2) ;
                        IntArray neighFromCache(2) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_TOP ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     {
                        // 1-4 (a)
                        IntArray facesToCap(3) ;
                        IntArray neighFromCache(3) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_TOP ;
                        facesToCap[2] = F_FRONT ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 1 ;
                        neighFromCache[2] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                  } // end for( k )
               }
               else
               if( j == LASTY )
               {     // Last Row
                  for( k=0; k<_grid.dim.z; k++ )
                  {
                     if( k == 0 )
                     {
                        // 1-4 (b)
                        IntArray facesToCap(2) ;
                        IntArray neighFromCache(2) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_FRONT ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     if( k == LASTZ )
                     {
                        IntArray facesToCap(1) ;
                        IntArray neighFromCache(1) ;

                        facesToCap[0] = F_RIGHT ;
                        neighFromCache[0] = 0 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     {
                        // 1-3
                        IntArray facesToCap(2) ;
                        IntArray neighFromCache(2) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_FRONT ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                  } // end for( k )
               }
               else
               {     // Middle Rows
                  for( k=0; k<_grid.dim.z; k++ )
                  {
                     if( k == 0 )
                     {
                        // 1-4 (b)
                        IntArray facesToCap(3) ;
                        IntArray neighFromCache(3) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_TOP ;
                        facesToCap[2] = F_FRONT ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 1 ;
                        neighFromCache[2] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     if( k == LASTZ )
                     {
                        IntArray facesToCap(2) ;
                        IntArray neighFromCache(2) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_TOP ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                     else
                     {
                        // 1-3
                        IntArray facesToCap(3) ;
                        IntArray neighFromCache(3) ;

                        facesToCap[0] = F_RIGHT ;
                        facesToCap[1] = F_TOP ;
                        facesToCap[2] = F_FRONT ;
                        neighFromCache[0] = 0 ;
                        neighFromCache[1] = 1 ;
                        neighFromCache[2] = 1 ;

                        _computeSingleCell3D
                           (
                              VF, Dist, AF, VFsize, DIsize, AFsize,
                              i,j,k, 1,
                              facesToCap,
                              neighFromCache,
                              _cache[curr],
                              _cache[next]
                           ) ;
                     }
                  } // end for( k )
               }
            } // end for( j )
         }
            // Update caches
         curr = next ;
         next = (next+1)%2 ;
         _cache[next].reset() ;
      } // end for( i )

      return 1 ;
   }

*/
// ============================================================
//    Operations
// ============================================================

template<class Real>
int EBSurfaceGenerator :: compute
   (
      Real * VF,              // IN : VF data
      Real * Dist,            // IN : Dist data
      Real * AF,              // IN : AF data
      unsigned int   VFsize,  // IN : VF stride
      unsigned int   DIsize,  // IN : Dist stride
      unsigned int   AFsize   // IN : AF stride
   )
   {
      if( VF == NULL || AF == NULL )
      {
         cerr  << "EBSurfaceGenerator::compute(): "
               << "fraction data pointer NULL" << endl ;
         return 0 ;
      }

      if( index == NULL )
      {
         cerr  << "EBSurfaceGenerator::compute(): "
               << "indexing routine not defined" << endl ;
         return 0 ;
      }

         // (Re)-initialize variables
      int result ;
      _surface.clear() ;
      _cellIDs.clear() ;
      if( _calculator.gridMode() == EBSINGLE::GRID2D )
      {
         result = _compute2D( VF, Dist, AF, VFsize, DIsize, AFsize ) ;
         return result ;
      }

      _startAddress.clear() ;
      result = _compute3D( VF, Dist, AF, VFsize, DIsize, AFsize ) ;
      _startAddress.push_back( _surface.size() ) ;
      return result ;
   }

// ============================================================
//    Set / Access Routines
// ============================================================

unsigned int EBSurfaceGenerator :: size ()
   {
      switch( _calculator.gridMode() )
      {
         case EBSINGLE::GRID2D :
            return _surface.size()/4 ;
         case EBSINGLE::GRID3D :
            return _startAddress.size()-1 ;
      }
      cerr  << "EBSurfaceGenerator::size(): "
            << "invalid grid mode" << endl ;
      return 0 ;
   }

double * EBSurfaceGenerator :: line ( int i )
   {
      return &(_surface[i*4]) ;
   }

double * EBSurfaceGenerator :: poly ( int i, int& size )
   {
      size = ( _startAddress[i+1] - _startAddress[i] ) / 3 ;
      return &(_surface[ _startAddress[i] ]) ;
   }

int * EBSurfaceGenerator :: cellId ( int i )
   {
      return &(_cellIDs[i*2]) ;
   }

void EBSurfaceGenerator :: setGrid ( GridInfo& g )
   {
      _grid = g ;
      _calculator.setCellDimensions( g.space.x, g.space.y, g.space.z ) ;
   }
