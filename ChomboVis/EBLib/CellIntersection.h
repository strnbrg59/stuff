#ifndef _CellIntersection_H_
#define _CellIntersection_H_

// ============================================================
//    Class CellIntersection
//
//       Small library for calculating the intersection
//       between a hexahedron and a plane. Can return
//       other information to help with volume calculations
//       and intersection connectivity info.
//
//       Orientation for all geometry is based on
//       the direction of the cutting plane's normal.
//
//       All routines return 1 on success and 0 on failure.
//
//       Written specifically for EB calculations.
//
//       The user must set the mode before calling
//       IntersectCellWithPlane(). Furthermore, calls to
//       SplitVolumeIntoTets() is only valid if the mode
//       include COMPUTE_INTERSECTING_VOLUME at the time
//       that IntersectCellWithPlane() was called.
//
//    TODO :
//       - allow access to faces of polyhedron (surface area calc)
//       - add COMPUTE_INTERSECTING_VOLUME_CENTROID feature
//
//    (c) Christopher S. Co
// ============================================================

#include <vector>
#include <list>
#include "CVPolygon.h"
#include "Plane.h"
#include "Hexahedron.h"
#include "Tetrahedron.h"

class CellIntersection
{
   public :

         // Data types (for readability)
      typedef std::vector<int>        IntArray ;
      typedef std::vector<CVPolygon>    PolyArray ;
      typedef std::list<Tetrahedron>  TetList ;

   private :

      int         _mode ;              // Determines how much to compute
      Point       _centroid ;          // Centroid of volume of intersection
      CVPolygon     _Volume2D ;          // Poly of intersection in 2D
      IntArray    _TetFaces ;          // Polys of VolumeFaces that form tets
      IntArray    _FacesIntersected ;  // Cell faces intersected
      PolyArray   _VolumeFaces ;       // Polys of volume of intersection

   public :

         // Computation modes (how much work to do)
      static const int COMPUTE_NONE                         = 0x00 ;
      static const int COMPUTE_INTERSECTING_VOLUME          = 0x01 ;
      static const int COMPUTE_INTERSECTED_FACES            = 0x02 ;
      static const int COMPUTE_INTERSECTING_VOLUME_CENTROID = 0x04 ;

         // Constructors, Destructor
      CellIntersection () ;
      ~CellIntersection () ;

         // Intersection Routines
      int IntersectCellWithPlane
         (
            const Plane&,        // IN : the plane slicing the volume
            const Hexahedron&,   // IN : the hexahedron to intersect
            CVPolygon&             // OUT: the intersecting polygon
         ) ;

      int IntersectCellWithPlane
         (
            const Plane &,       // IN : the plane slicing the volume
            const CVPolygon &,     // IN : the quad to intersect
            Point &,             // OUT: the 'left' point of intersection
            Point &              // OUT: the 'right' point of intersection
         ) ;

      int IntersectLineWithPlane
         (
            const Point&,        // IN : the 1st point defining line
            const Point&,        // IN : the 2nd point defining line
            const Plane&,        // IN : the plane to intersect
            Point&               // OUT: the intersection point
         ) ;

         // Additional helper routines
      int SplitVolumeIntoTets
         (
            TetList&             // OUT: vol. of intersection split into tets
         ) ;
      double VolumeOfIntersection3D () ;
      double VolumeOfIntersection2D () ;

         // Set / Access Routines
      void mode ( int ) ;
      IntArray& facesIntersected () ;
      Point& centroid () ;
      //PolyArray& volumeOfIntersection () ;
} ; // end class CellIntersection

#endif
