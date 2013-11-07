#ifndef _EBCache_H_
#define _EBCache_H_

// ============================================================
//    Class EBCache
//
//       Simple front-end to array (2D or 3D) for caching
//       EB data on a per cell basis.
//
//       Helps cache EB data computed for previous cells
//       to aid in capping regions between cells.
//
//    (c) Christopher S. Co, June 2002
// ============================================================

#include <vector>
#include "Plane.h"

using namespace std ;

typedef struct
{
   Plane          plane ;
   std::vector<int>    faces ;
} EBCacheItem ;

class EBCache
{
   public :

      typedef struct
      {
         int x, y, z ;
      } trivar ;

   private :

      trivar         _origin ;
      trivar         _dim ;
      char *         _validBits ;
      EBCacheItem *  _ebplanes ;

      inline int _index ( int i, int j, int k )
         {
            return ( i*_dim.y*_dim.z + j*_dim.z + k ) ;
         }
      inline EBCacheItem& _element ( int i, int j, int k )
         {
            return _ebplanes[ _index( i,j,k ) ] ;
         }

   public :

         // Constructors, Destructor
      EBCache () ;
      ~EBCache () ;

         // Operations
      void reset () ;
      void setOrigin ( int, int, int ) ;
      void setDim ( int, int, int ) ;
      void getOrigin ( int&, int&, int& ) ;
      void getDim ( int&, int&, int& ) ;
      int setPlane ( int, int, int, const EBCacheItem& ) ;
      int getPlane ( int, int, int, EBCacheItem& ) ;

} ;   // end class EBCache

#endif
