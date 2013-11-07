// ============================================================
//    Class EBCapper
//
//       This class encapsulates code to stitch together
//       disjoint surfaces created for regular grid data.
//
//       Cells' boundaries are clipped against the EB
//       produced by EBSingleCell on consecutive cells
//       in a grid to produce the caps.
//
//       This code uses a caching facility for efficiency.
//
//    (c) Christopher S. Co
// ============================================================

#include "EBCapper.h"

// ============================================================
//    Copy Routine
// ============================================================

void EBCapper :: _Copy ( const EBCapper& e )
   {
   }

// ============================================================
//    Constructors, Destructor
// ============================================================

EBCapper :: EBCapper ()
   {
   }

EBCapper :: EBCapper ( const EBCapper& e )
   {
      _Copy( e ) ;
   }

EBCapper :: ~EBCapper ()
   {
   }

// ============================================================
//    Internal geometry helper
// ============================================================

int EBCapper :: _exor
   (
      PlaneArray& cutPlanes,  // IN : cutting planes at edge of clips
      PolyArray&  clips,      // IN : polygons to exor
      PolyArray&  result      // OUT: resulting poly(s) from exor
   )
   {
// TODO case when _exor result is empty (should not happen... but...)
      int success = 0 ;
      CVPolygon piece ;
      CVPolygon& a = clips[0] ;
      CVPolygon& b = clips[1] ;
      Plane& ap = cutPlanes[0] ;
      Plane& bp = cutPlanes[1] ;
      if( a.ClipOutEx( bp, piece ) )
      {
         result.push_back( piece ) ;
         success = 1 ;
      }
      if( b.ClipOutEx( ap, piece ) )
      {
         result.push_back( piece ) ;
         success = 1 ;
      }
      return success ;
   }

int EBCapper :: _exor
   (
      PlaneArray& cutPlanes,  // IN : cutting planes at edge of clips
      LineArray&  clips,      // IN : lines to exor
      LineArray&  result      // OUT: resulting line(s) from exor
   )
   {
// TODO case when _exor result is empty (should not happen... but...)
      int success = 0 ;
      LineSegment piece ;
      LineSegment& a = clips[0] ;
      LineSegment& b = clips[1] ;
      Plane& ap = cutPlanes[0] ;
      Plane& bp = cutPlanes[1] ;
      if( a.clipOutEx( bp, piece ) )
      {
         result.push_back( piece ) ;
         success = 1 ;
      }
      if( b.clipOutEx( ap, piece ) )
      {
         result.push_back( piece ) ;
         success = 1 ;
      }
      return success ;
   }

// ============================================================
//    Operations
// ============================================================

int EBCapper :: cap
   (
      CVPolygon&    face,       // IN : face to be used as base for capping
      PlaneArray& EBs,        // IN : EBs affecting the shape of the cap
      PolyArray&  capping     // OUT: the polygon(s) sealing the hole
   )
   {
      unsigned int i ;
      CVPolygon tmp ;
      PolyArray clips ;

      if( EBs.size() > 2 )
      {
         return 0 ;
      }
// TODO: Is this right? 
      if( EBs.size() == 0 )
      {
         capping.push_back( face ) ;
         return 0 ;
      }
/*
// TODO: Or is this right? 
      if( EBs.size() == 0 )
      {
         capping.push_back( face ) ;
         return 1 ;
      }
*/

         // Clip "face" against each EB
      for( i=0; i<EBs.size(); i++ )
      {
         if( face.ClipIn( EBs[i], tmp ) )
         {
            clips.push_back( tmp ) ;
         }
      }

      if( clips.size() == 2 )
      {     // Take the exor of clipped polys
         _exor( EBs, clips, capping ) ;
      }
      else
      {
         capping = clips ;
      }

      if( capping.size() == 0 )
      {
         return 0 ;
      }

         // Fix orientation (if necessary)
// TODO figure out how to fix the orientation
// - Orientation of single cap should report YES to IsIn() of VolumeCentroid
// - Two caps can be associated with two planes as follows:
//    - a plane belongs to a cap if it reports YES to IsIn() of cap centroid
// This requires that the volume of intersection centroid be calculated
//    and passed in.
      Plane capPlane( capping[0] ) ;
      if( EBs.size() == 2 )
      {
         Point capCentroid ;
         capCentroid = capping[0].Centroid() ;
         if( EBs[0].IsIn( capCentroid ) )
         {     // Cap 0 goes to Plane 0
               // Cap 1 goes to Plane 1
            if( capPlane.IsOut( EBs[0].anchor() ) )
            {
               capping[0].Reverse() ;
            }
            else
            if( capPlane.IsOn( EBs[0].anchor() ) )
            {
               capping[0].Reverse() ;
            }
            else
            if( capPlane.IsOn( EBs[1].anchor() ) )
            {
               capping[0].Reverse() ;
            }
            else
            if( capping.size() == 2 )
            {
               capping[1].Reverse() ;
            }
         }
         else
         {     // Cap 0 goes to Plane 1
               // Cap 1 goes to Plane 0
            if( capPlane.IsOut( EBs[1].anchor() ) )
            {
               capping[0].Reverse() ;
            }
            else
            if( capPlane.IsOn( EBs[0].anchor() ) )
            {
               capping[0].Reverse() ;
            }
            else
            if( capPlane.IsOn( EBs[1].anchor() ) )
            {
               capping[0].Reverse() ;
            }
            else
            if( capping.size() == 2 )
            {
               capping[1].Reverse() ;
            }
         }
      }
      else
      {     // EBs.size() == 1
         if( capPlane.IsOut( EBs[0].anchor() ) )
         {
            capping[0].Reverse() ;
         }
      }

      return 1 ;
   }

// assumes that their anchors are not in each other's planes
int shouldReverse ( Plane& cap, Plane& eb )
   {
      if( cap.IsIn( eb.anchor() ) && eb.IsIn( cap.anchor() ) )
         return 0 ;
      return 1 ;
   }

int EBCapper :: cap
   (
      LineSegment&   face,       // IN : face to be used as base for capping
      PlaneArray&    EBs,        // IN : EBs affecting the shape of the cap
      LineArray&     capping     // OUT: the line(s) sealing the hole
   )
   {
      unsigned int i ;
      LineSegment tmp ;
      LineArray clips ;

      if( EBs.size() > 2 )
      {
         return 0 ;
      }
      if( EBs.size() == 0 )
      {
         capping.push_back( face ) ;
         return 1 ;
      }

         // Clip "face" against each EB
      for( i=0; i<EBs.size(); i++ )
      {
         if( face.clipIn( EBs[i], tmp ) )
         {
            clips.push_back( tmp ) ;
         }
      }

      if( clips.size() == 2 )
      {     // Take the exor of clipped polys
         _exor( EBs, clips, capping ) ;
      }
      else
      {
         capping = clips ;
      }

      if( capping.size() == 0 )
      {
         return 0 ;
      }

         // Fix orientation (if necessary)
// TODO figure out how to fix the orientation
      Plane capPlane ;
      capPlane.SetAnchor( capping[0].left() ) ;
      capPlane.SetDirection
         ( cross( VZ, capping[0].right()-capping[0].left() ) ) ;
      if( EBs.size() == 2 )
      {
         Point capCentroid ;
         capCentroid = capping[0].centroid() ;
         if( EBs[0].IsIn( capCentroid ) )
         {     // Cap 0 goes to Plane 0
               // Cap 1 goes to Plane 1
            if( capPlane.IsOut( EBs[0].anchor() ) )
            {
               capping[0].reverse() ;
            }
            else
            if( capping.size() == 2 )
            {
               capping[1].reverse() ;
            }
         }
         else
         if( EBs[0].IsOn( capCentroid ) )
         {
         }
         else
         if( EBs[1].IsOn( capCentroid ) )
         {
         }
         else
         {     // Cap 0 goes to Plane 1
               // Cap 1 goes to Plane 0
            if( capPlane.IsOut( EBs[1].anchor() ) )
            {
               capping[0].reverse() ;
            }
            else
            if( capping.size() == 2 )
            {
               capping[1].reverse() ;
            }
         }
      }
      else
      {     // EBs.size() == 1
         if( capPlane.IsOut( EBs[0].anchor() ) )
         {
            capping[0].reverse() ;
         }
      }

      return 1 ;
   }

// ============================================================
//    Adjacent Cell
//
//    Given a single face, it returns relative grid
//    coordinates identifying the location of the cell
//    that shares this face.
// ============================================================

void EBCapper :: adjCell ( int face, int& i, int& j, int& k )
   {
      if( face < 0 || face > 5 )
      {
         cerr  << "EBCapper::adjCell(): "
               << "invalid face" << endl ;
         return ;
      }

      static   int connectivity[][3] =
         {
            { -1,  0,  0 },   // LEFT
            {  1,  0,  0 },   // RIGHT
            {  0, -1,  0 },   // BOTTOM
            {  0,  1,  0 },   // TOP
            {  0,  0, -1 },   // BACK
            {  0,  0,  1 }    // FRONT
         } ;
      i = connectivity[face][0] ;
      j = connectivity[face][1] ;
      k = connectivity[face][2] ;
   }

void EBCapper :: adjCell ( int face, int& i, int& j )
   {
      if( face < 0 || face > 3 )
      {
         cerr  << "EBCapper::adjCell(): "
               << "invalid face" << endl ;
         return ;
      }

      static   int connectivity[][2] =
         {
            {  0, -1 },
            {  1,  0 },
            {  0,  1 },
            { -1,  0 }
         } ;
      i = connectivity[face][0] ;
      j = connectivity[face][1] ;
   }

// ============================================================
//    Access Routines
// ============================================================

// ============================================================
//    Set Routines
// ============================================================

