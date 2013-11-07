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

#include <string.h>
#include "EBCache.h"

// ============================================================
//    Constructors, Destructor
// ============================================================

EBCache :: EBCache ()
   {
      _origin.x = _origin.y = _origin.z = 0 ;
      _dim.x = _dim.y = _dim.z = 0 ;
      _validBits = NULL ;
      _ebplanes = NULL ;
   }

EBCache :: ~EBCache ()
   {
      if( _validBits )  delete [] _validBits ;
      if( _ebplanes )   delete [] _ebplanes ;
   }

// ============================================================
//    Operations
// ============================================================

void EBCache :: reset ()
   {
      int size ;

      size = _dim.x*_dim.y*_dim.z ;
      memset( _validBits, 0, size ) ;
   }

void EBCache :: setOrigin ( int x, int y, int z )
   {
      _origin.x = x ;
      _origin.y = y ;
      _origin.z = z ;
   }

void EBCache :: getOrigin ( int& x, int& y, int& z )
   {
      x = _origin.x ;
      y = _origin.y ;
      z = _origin.z ;
   }

void EBCache :: setDim ( int x, int y, int z )
   {
      int size ;

      size = x*y*z ;
      if( _ebplanes )
      {
         int csize ;

         csize = _dim.x * _dim.y * _dim.z ;
         if( size > csize )
         {
            delete [] _validBits ;
            _validBits = new char [ size ] ;
            delete [] _ebplanes ;
            _ebplanes = new EBCacheItem [ size ] ;
         }
      }
      else
      {
         _validBits = new char [ size ] ;
         _ebplanes = new EBCacheItem [ size ] ;
      }
      _dim.x = x ;
      _dim.y = y ;
      _dim.z = z ;
      reset() ;
   }

void EBCache :: getDim ( int& x, int& y, int& z )
   {
      x = _dim.x ;
      y = _dim.y ;
      z = _dim.z ;
   }

int EBCache :: setPlane ( int i, int j, int k, const EBCacheItem& p )
   {
      int dx, dy, dz ;

      dx = i - _origin.x ;
      dy = j - _origin.y ;
      dz = k - _origin.z ;

      if( dx < _dim.x && dy < _dim.y && dz < _dim.z )
      {
         if( dx >= 0 && dy >= 0 && dz >= 0 )
         {
            _element( dx,dy,dz ) = p ;
            _validBits[ _index(dx,dy,dz) ] = 1 ;
            return 1 ;
         }
      }

      cerr  << "EBCache::setPlane(): "
            << "cell (" << i <<","<< j <<","<< k << ") not in cache" << endl ;

      return 0 ;
   }

int EBCache :: getPlane ( int i, int j, int k, EBCacheItem& p )
   {
      int dx, dy, dz ;

      dx = i - _origin.x ;
      dy = j - _origin.y ;
      dz = k - _origin.z ;

      if( dx < _dim.x && dy < _dim.y && dz < _dim.z )
      {
         if( dx >= 0 && dy >= 0 && dz >= 0 )
         {
            if( _validBits[ _index(dx,dy,dz) ] )
            {
               p = _element( dx,dy,dz ) ;
               return 1 ;
            }
            else
            {
               return 0 ;
            }
         }
      }

      cerr  << "EBCache::getPlane(): "
            << "cell (" << i <<","<< j <<","<< k << ") not in cache" << endl ;

      return 0 ;
   }
