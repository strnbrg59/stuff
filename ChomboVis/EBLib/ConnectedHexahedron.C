// ============================================================
//                      ConnectedHexahedron.C
// ============================================================

#include <cstdlib>
#include <cmath>
#include <strings.h>
#include <stream.h>

#ifdef VIS
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif

#include "ConnectedHexahedron.h"
#include "Tetrahedron.h"

#define     TRUE     1 
#define     FALSE    0

// ==========================================================
//               CONNECTIVITY INFORMATION
//
//   The following tables encapsulate the connection
//   between edges and vertices in the hexahedron. With
//   this information, we should be able to traverse the
//   hexahedron from vertex to edge and from edge to vertex.
//
// ==========================================================

static const int _VertexToVertex[8][3] =
   {
      { 1, 4, 2 },   // Indices of hexahedron verts connected to 0-th vertex
      { 0, 3, 5 },
      { 0, 6, 3 },   // Indices of hexahedron verts connected to 2-nd vertex
      { 1, 2, 7 },
      { 0, 5, 6 },   // Indices of hexahedron verts connected to 4-th vertex
      { 1, 7, 4 },
      { 2, 4, 7 },   // Indices of hexahedron verts connected to 6-th vertex
      { 3, 6, 5 }
   } ;

static const int _VertexToEdge[8][3] =
   {
      { 'a', 'c', 'b' },   // Indices of edges connected to 0-th vertex
      { 'a', 'd', 'e' },
      { 'b', 'g', 'f' },   // Indices of edges connected to 2-nd vertex
      { 'd', 'f', 'h' },
      { 'c', 'j', 'i' },   // Indices of edges connected to 4-th vertex
      { 'e', 'k', 'j' },
      { 'g', 'i', 'l' },   // Indices of edges connected to 6-th vertex
      { 'h', 'l', 'k' }
   } ;

static const int _VertexToFace[8][3] =
   {
      { 'A', 'C', 'E' },   // Indices of faces connected to 0-th vertex
      { 'A', 'F', 'C' },
      { 'A', 'E', 'D' },   // Indices of faces connected to 2-nd vertex
      { 'A', 'D', 'F' },
      { 'B', 'E', 'C' },   // Indices of faces connected to 4-th vertex
      { 'B', 'C', 'F' },
      { 'B', 'D', 'E' },   // Indices of faces connected to 6-th vertex
      { 'B', 'F', 'D' }
   } ;

static const int _EdgeToVertex[12][8] =
   {
      // 0   1   2   3   4   5   6   7
      {  1,  0, -1, -1, -1, -1, -1, -1 }, // a
      {  2, -1,  0, -1, -1, -1, -1, -1 }, // b
      {  4, -1, -1, -1,  0, -1, -1, -1 }, // c
      { -1,  3, -1,  1, -1, -1, -1, -1 }, // d
      { -1,  5, -1, -1, -1,  1, -1, -1 }, // e
      { -1, -1,  3,  2, -1, -1, -1, -1 }, // f
      { -1, -1,  6, -1, -1, -1,  2, -1 }, // g
      { -1, -1, -1,  7, -1, -1, -1,  3 }, // h
      { -1, -1, -1, -1,  6, -1,  4, -1 }, // i
      { -1, -1, -1, -1,  5,  4, -1, -1 }, // j
      { -1, -1, -1, -1, -1,  7, -1,  5 }, // k
      { -1, -1, -1, -1, -1, -1,  7,  6 }  // l
   } ;

static const int _EdgeToFace[12][12] =
   {
      //  a    b    c    d    e    f    g    h    i    j    k    l
      {  -1, 'A', 'C', 'A', 'C', 'A',  -1,  -1,  -1, 'C',  -1,  -1 }, // a
      { 'A',  -1, 'E', 'A',  -1, 'A', 'E',  -1, 'E',  -1,  -1,  -1 }, // b
      { 'C', 'E',  -1,  -1, 'C',  -1, 'E',  -1, 'E', 'C',  -1,  -1 }, // c
      { 'A', 'A',  -1,  -1, 'F', 'A',  -1, 'F',  -1,  -1, 'F',  -1 }, // d
      { 'C',  -1, 'C', 'F',  -1,  -1,  -1, 'F',  -1, 'C', 'F',  -1 }, // e
      { 'A', 'A',  -1, 'A',  -1,  -1, 'D', 'D',  -1,  -1,  -1, 'D' }, // f
      {  -1, 'E', 'E',  -1,  -1, 'D',  -1, 'D', 'E',  -1,  -1, 'D' }, // g
      {  -1,  -1,  -1, 'F', 'F', 'D', 'D',  -1,  -1,  -1, 'F', 'D' }, // h
      {  -1, 'E', 'E',  -1,  -1,  -1, 'E',  -1,  -1, 'B', 'B', 'B' }, // i
      { 'C',  -1, 'C',  -1, 'C',  -1,  -1,  -1, 'B',  -1, 'B', 'B' }, // j
      {  -1,  -1,  -1, 'F', 'F',  -1,  -1, 'F', 'B', 'B',  -1, 'B' }, // k
      {  -1,  -1,  -1,  -1,  -1, 'D', 'D', 'D', 'B', 'B', 'B',  -1 }  // l
   } ;
/*
// I may choose to use this someday.... CSC 08/22/2001
static const int _VertexToSingleEdge[8][8] =
   {
      // 0    1    2    3    4    5    6    7
      { -1 , 'a', 'b', -1 , 'c', -1 , -1 , -1  }, // 0
      { 'a', -1 , -1 , 'd', -1 , 'e', -1 , -1  }, // 1
      { 'b', -1 , -1 , 'f', -1 , -1 , 'g', -1  }, // 2
      { -1 , 'd', 'f',  1 , -1 , -1 , -1 , 'h' }, // 3
      { 'c', -1 , -1 , -1 , -1 , 'j', 'i', -1  }, // 4
      { -1 , 'e', -1 , -1 , 'j', -1 , -1 , 'k' }, // 5
      { -1 , -1 , 'g', -1 , 'i', -1 , -1 , 'l' }, // 6
      { -1 , -1 , -1 , 'h', -1 , 'k', 'l', -1  }  // 7
   } ;
*/

// ============================================================
//            HexahedronVertexIterator
// ============================================================

HexahedronVertexIterator :: HexahedronVertexIterator ()
   {
      _base = -1 ;
      _curr = -1 ;
      _begin = -1 ;
   }

HexahedronVertexIterator :: HexahedronVertexIterator ( const int base )
   {
      if( base > 7 || base < 0 )
      {
         cerr  << "HexahedronVertexIterator:: Invalid hexahedron base index"
               << endl ;
         _base = -1 ;
         _curr = -1 ;
         _begin = -1 ;
         return ;
      }

      _base = base ;
      _curr = 0 ;
      _begin = 0 ;
   }

HexahedronVertexIterator :: HexahedronVertexIterator
   ( const HexahedronVertexIterator& p ) 
   {
      _Copy( p ) ;
   }

HexahedronVertexIterator :: ~HexahedronVertexIterator ()
   {
   }
      
void HexahedronVertexIterator :: _Copy ( const HexahedronVertexIterator& p )
   {
      _base = p._base ;
      _curr = p._curr ;
      _begin = p._begin ;
   }

void HexahedronVertexIterator :: _IsValidIterator() const
   {
      if(   _base > 7 || _base < 0 
            || _curr < 0 || _curr > 2
         )
      {
         cerr  << "HexahedronVertexIterator:: Uninitialized or invalid iterator"
               << endl ;
         exit( 200 ) ;
      }
   }

// ==========================================================
//      OPERATOR=
// ==========================================================

HexahedronVertexIterator& HexahedronVertexIterator :: operator=
   ( const HexahedronVertexIterator& p ) 
   {
      if ( this == &p ) return ( *this ) ;

      _Copy( p ) ;

      return ( *this ) ;
   }

// ==========================================================
//      OUTPUT
// ==========================================================

ostream& operator<< ( ostream& co, const HexahedronVertexIterator& p )
   {
      p._IsValidIterator() ;

      co << "{ " ;
      co << "base index: " << p._base ;
      co << " " ;
      co << "curr vertex: "
         << _VertexToVertex[ p._base ][ p._curr%3 ] ;
      co << " }" ;

      return co ;
   }

// ==========================================================
//      COMPARISON OPERATIONS
// ==========================================================

int operator!=
   ( const HexahedronVertexIterator& p1, const HexahedronVertexIterator& p2 ) 
   {
      return !(p1 == p2) ;
   }

int operator==
   ( const HexahedronVertexIterator& p1, const HexahedronVertexIterator& p2 ) 
   {
      if(   ( p1._base == p2._base )
            && ( p1._curr == p2._curr )
            && ( p1._begin == p2._begin )
         )
      {
         return TRUE ;
      }

      return FALSE ;
   }

// ==========================================================
//      CURRENT AND NEXT OPERATIONS
// ==========================================================

int HexahedronVertexIterator :: Next ()
   {
      _curr = ( _curr + 1 ) % 3 ;

      _IsValidIterator() ;

      return _VertexToVertex[ _base ][ _curr ] ;
   }

int HexahedronVertexIterator :: Current () const
   {
      _IsValidIterator() ;

      return _VertexToVertex[ _base ][ _curr ] ;
   }

int HexahedronVertexIterator :: End () const
   {
      if( _curr == _begin )
         return TRUE ;

      return FALSE ;
   }

void HexahedronVertexIterator :: Reset ()
   {
      _curr = _begin ;
   }

// ============================================================
//            HexahedronEdgeIterator
// ============================================================

HexahedronEdgeIterator :: HexahedronEdgeIterator ()
   {
      _base = -1 ;
      _curr = -1 ;
      _begin = -1 ;
   }

HexahedronEdgeIterator :: HexahedronEdgeIterator ( const int base )
   {
      if( base > 7 || base < 0 )
      {
         cerr  << "HexahedronEdgeIterator:: Invalid hexahedron base index"
               << endl ;
         _base = -1 ;
         _curr = -1 ;
         _begin = -1 ;
         return ;
      }

      _base = base ;
      _curr = 0 ;
      _begin = 0 ;
   }

HexahedronEdgeIterator :: HexahedronEdgeIterator
   ( const int base, const int startEdge )
   {
         //  "base" is the index [0,7] indicating which of the 8
         //  hexahedron vertices is the center of our iteration
         //
         //  "startEdge" is the index [0,11] indicating which of
         //  the 12 edges we wish to begin with.

      if( base > 7 || base < 0
         || startEdge > 11 || startEdge < 0
         )
      {
         cerr  << "HexahedronEdgeIterator:: Invalid hexahedron base index: "
               << *this
               << endl ;
         _base = -1 ;
         _curr = -1 ;
         _begin = -1 ;
         return ;
      }

      _base = base ;

      int found = 0 ;

      for( _curr=0; _curr<3; _curr++ )
      {
         if( ( startEdge + 'a' ) == _VertexToEdge[_base][_curr] )
         {
            found = 1 ;
            break ;
         }
      }
      _begin = _curr ;

      if( !found )
      {
         cerr  << "HexahedronEdgeIterator:: Invalid hexahedron starting edge: "
               << *this
               << endl ;
         _base = -1 ;
         _curr = -1 ;
         _begin = -1 ;
         return ;
      }
   }

HexahedronEdgeIterator :: HexahedronEdgeIterator
   ( const HexahedronEdgeIterator& p ) 
   {
      _Copy( p ) ;
   }

HexahedronEdgeIterator :: ~HexahedronEdgeIterator ()
   {
   }
      
void HexahedronEdgeIterator :: _Copy ( const HexahedronEdgeIterator& p )
   {
      _base = p._base ;
      _curr = p._curr ;
      _begin = p._begin ;
   }

void HexahedronEdgeIterator :: _IsValidIterator() const
   {
      if(   _base > 7 || _base < 0 
            || _curr < 0 || _curr > 11
         )
      {
         cerr  << "HexahedronEdgeIterator:: Uninitialized or invalid iterator"
               << endl ;
         exit( 200 ) ;
      }
   }

// ==========================================================
//      OPERATOR=
// ==========================================================

HexahedronEdgeIterator& HexahedronEdgeIterator :: operator=
   ( const HexahedronEdgeIterator& p ) 
   {
      if ( this == &p ) return ( *this ) ;

      _Copy( p ) ;

      return ( *this ) ;
   }

// ==========================================================
//      OUTPUT
// ==========================================================

ostream& operator<< ( ostream& co, const HexahedronEdgeIterator& p )
   {
      p._IsValidIterator() ;

      co << "{ " ;
      co << "base index: " << p._base ;
      co << " " ;
      co << "curr edge: "
         << (char) _VertexToEdge[ p._base ][ p._curr%3 ] ;
      co << " }" ;

      return co ;
   }

// ==========================================================
//      COMPARISON OPERATIONS
// ==========================================================

int operator!=
   ( const HexahedronEdgeIterator& p1, const HexahedronEdgeIterator& p2 ) 
   {
      return !(p1 == p2) ;
   }

int operator==
   ( const HexahedronEdgeIterator& p1, const HexahedronEdgeIterator& p2 ) 
   {
      if(   ( p1._base == p2._base )
            && ( p1._curr == p2._curr )
            && ( p1._begin == p2._begin )
         )
      {
         return TRUE ;
      }

      return FALSE ;
   }

// ==========================================================
//      CURRENT AND NEXT OPERATIONS
// ==========================================================

int HexahedronEdgeIterator :: Next ()
   {
      _curr = ( _curr + 1 ) % 3 ;

      _IsValidIterator() ;

      return _VertexToEdge[ _base ][ _curr ] - 'a' ;
   }

int HexahedronEdgeIterator :: Current () const
   {
      _IsValidIterator() ;

      return _VertexToEdge[ _base ][ _curr ] - 'a' ;
   }

int HexahedronEdgeIterator :: GetBaseVertex () const
   {
      return _base ;
   }

int HexahedronEdgeIterator :: GetOtherVertex () const
   {
      _IsValidIterator() ;

      return _EdgeToVertex[ Current() ][ _base ] ;
   }

int HexahedronEdgeIterator :: End () const
   {
      if( _curr == _begin )
         return TRUE ;

      return FALSE ;
   }

void HexahedronEdgeIterator :: Reset ()
   {
      _curr = _begin ;
   }

int GetOtherVertexConnectedToEdge ( const int edge, const int vertex )
   {
      if(   edge < 0 || edge > 11
            || vertex < 0 || vertex > 7
         )
      {
         cerr  << "GetOtherVertexConnectedToEdge: "
               << "invalid edge and/or vertex indices"
               << endl ;
         exit( 200 ) ;
      }

      return _EdgeToVertex[ edge ][ vertex ] ;
   }

int GetFaceConnectedToEdges ( const int e1, const int e2 )
   {
      if(   e1 < 0 || e1 > 11
            || e2 < 0 || e2 > 11
         )
      {
         cerr  << "GetFaceConnectedToEdges: "
               << "invalid edge indices"
               << endl ;
         exit( 200 ) ;
      }

      return _EdgeToFace[ e1 ][ e2 ] - 'A' ;
   }

// ============================================================
//            HexahedronFaceIterator
// ============================================================

HexahedronFaceIterator :: HexahedronFaceIterator ()
   {
      _base = -1 ;
      _curr = -1 ;
      _begin = -1 ;
   }

HexahedronFaceIterator :: HexahedronFaceIterator
   ( const int base )
   {
      if( base > 7 || base < 0 )
      {
         cerr  << "HexahedronFaceIterator:: Invalid hexahedron base index"
               << endl ;
         _base = -1 ;
         _curr = -1 ;
         _begin = -1 ;
         return ;
      }

      _base = base ;
      _curr = 0 ;
      _begin = 0 ;
   }

HexahedronFaceIterator :: HexahedronFaceIterator
   ( const int base, const int startEdge )
   {
         //  "base" is the index [0,7] indicating which of the 8
         //  hexahedron vertices is the center of our iteration
         //
         //  "startEdge" is the index [0,11] indicating which of
         //  the 12 edges we wish to begin with.

      if( base > 7 || base < 0
         || startEdge > 11 || startEdge < 0
         )
      {
         cerr  << "HexahedronFaceIterator:: Invalid hexahedron base index: "
               << *this
               << endl ;
         _base = -1 ;
         _curr = -1 ;
         _begin = -1 ;
         return ;
      }

      _base = base ;

      int found = 0 ;

      for( _curr=0; _curr<3; _curr++ )
      {
         if( ( startEdge + 'A' ) == _VertexToFace[_base][_curr] )
         {
            found = 1 ;
            break ;
         }
      }
      _begin = _curr ;

      if( !found )
      {
         cerr  << "HexahedronFaceIterator:: Invalid hexahedron starting edge: "
               << *this
               << endl ;
         _base = -1 ;
         _curr = -1 ;
         _begin = -1 ;
         return ;
      }
   }

HexahedronFaceIterator :: HexahedronFaceIterator
   ( const HexahedronFaceIterator& p ) 
   {
      _Copy( p ) ;
   }

HexahedronFaceIterator :: ~HexahedronFaceIterator ()
   {
   }
      
void HexahedronFaceIterator :: _Copy ( const HexahedronFaceIterator& p )
   {
      _base = p._base ;
      _curr = p._curr ;
      _begin = p._begin ;
   }

void HexahedronFaceIterator :: _IsValidIterator() const
   {
      if(   _base > 7 || _base < 0 
            || _curr < 0 || _curr > 11
         )
      {
         cerr  << "HexahedronFaceIterator:: Uninitialized or invalid iterator"
               << endl ;
         exit( 200 ) ;
      }
   }

// ==========================================================
//      OPERATOR=
// ==========================================================

HexahedronFaceIterator& HexahedronFaceIterator :: operator=
   ( const HexahedronFaceIterator& p ) 
   {
      if ( this == &p ) return ( *this ) ;

      _Copy( p ) ;

      return ( *this ) ;
   }

// ==========================================================
//      OUTPUT
// ==========================================================

ostream& operator<< ( ostream& co, const HexahedronFaceIterator& p )
   {
      p._IsValidIterator() ;

      co << "{ " ;
      co << "base index: " << p._base ;
      co << " " ;
      co << "curr face: "
         << (char) _VertexToFace[ p._base ][ p._curr%3 ] ;
      co << " }" ;

      return co ;
   }

// ==========================================================
//      COMPARISON OPERATIONS
// ==========================================================

int operator!=
   ( const HexahedronFaceIterator& p1, const HexahedronFaceIterator& p2 ) 
   {
      return !(p1 == p2) ;
   }

int operator==
   ( const HexahedronFaceIterator& p1, const HexahedronFaceIterator& p2 ) 
   {
      if(   ( p1._base == p2._base )
            && ( p1._curr == p2._curr )
            && ( p1._begin == p2._begin )
         )
      {
         return TRUE ;
      }

      return FALSE ;
   }

// ==========================================================
//      CURRENT AND NEXT OPERATIONS
// ==========================================================

int HexahedronFaceIterator :: Next ()
   {
      _curr = ( _curr + 1 ) % 3 ;

      _IsValidIterator() ;

      return _VertexToFace[ _base ][ _curr ] - 'A' ;
   }

int HexahedronFaceIterator :: Current () const
   {
      _IsValidIterator() ;

      return _VertexToFace[ _base ][ _curr ] - 'A' ;
   }

int HexahedronFaceIterator :: GetBaseVertex () const
   {
      return _base ;
   }

int HexahedronFaceIterator :: End () const
   {
      if( _curr == _begin )
         return TRUE ;

      return FALSE ;
   }

void HexahedronFaceIterator :: Reset ()
   {
      _curr = _begin ;
   }
