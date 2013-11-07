#ifndef _ConnectedHexahedron_h_
#define _ConnectedHexahedron_h_

//  ============================================================
//                    ConnectedHexahedron.h
//
//     Extends the hexadedron class by storing explicit
//     connectivity to allow specific types of calculations
//
//  ============================================================

#include <iostream>
#include "Hexahedron.h"

class HexahedronVertexIterator ;
class HexahedronEdgeIterator ;
class HexahedronFaceIterator ;

//  ============================================================
//                  Connectivity Encodings
//
//                           y
//                           |
//                           *-- x
//                          /
//                         z
//
//          2 ----- 6         * --g-- *         * ----- *
//         /|      /|        /|      /|        /|      /|
//        / |     / |       f |     l |       / | D   / |
//       /  |    /  |      /  b    /  i      /  |   E/  |
//      3 ----- 7   |     * --h-- *   |     * ----- *   |
//      |   |   |   |     |   |   |   |     | A |   | B |
//      |   0 --|-- 4     |   * -c|-- *     |   * --|-- *
//      |  /    |  /      d  /    k  /      |  /F   |  /
//      | /     | /       | a     | j       | /   C | /
//      |/      |/        |/      |/        |/      |/
//      1 ----- 5         * --e-- *         * ----- *
//
//                            * --6-- *
//                           /|      /|      A = left    (X-low)
//                          5 |     11|      B = right   (X-high)
//                         /  1    /  8      C = bottom  (Y-low)
//                        * --7-- *   |      D = top     (Y-high)
//                        |   |   |   |      E = back    (Z-low)
//                        |   * -2|-- *      F = front   (Z-high)
//                        3  /    10 /
//                        | 0     | 9
//                        |/      |/
//                        * --4-- *
//
//  ============================================================

//  ============================================================
//                  HexahedronVertexIterator
//  ============================================================

class HexahedronVertexIterator
{
   protected :
   
         //  The "base" vertex in the hexahedron whose
         //    3 edges we will iterate around
      int   _base ;

         //  The current neighbor vertex to the _base.
      int   _curr ;

         //  The neighbor vertex we started looking at first
      int   _begin ;

      void _Copy( const HexahedronVertexIterator& ) ;
      void _IsValidIterator() const ;

   public :

         //  Constructors, Destructor
      HexahedronVertexIterator () ;
      HexahedronVertexIterator ( const int ) ;
      HexahedronVertexIterator
         ( const HexahedronVertexIterator& ) ;
      virtual ~HexahedronVertexIterator () ;

         //  Assignment
      HexahedronVertexIterator& operator=
         ( const HexahedronVertexIterator& ) ;

         //  Output
      friend ostream& operator<<
         ( ostream&, const HexahedronVertexIterator& ) ;

         //  Comparison
      friend int operator==
         ( const HexahedronVertexIterator&, const HexahedronVertexIterator& ) ;
      friend int operator!=
         ( const HexahedronVertexIterator&, const HexahedronVertexIterator& ) ;

         //  Access functions for point iteration

         //  Calculations
      int   Next() ;
      int   Current() const ;
      int   End() const ;
      void  Reset() ;
} ;

//  ============================================================
//                  HexahedronEdgeIterator
//  ============================================================

class HexahedronEdgeIterator
{
   protected :
   
         //  The "base" vertex in the hexahedron whose
         //    3 edges we will iterate around
      int   _base ;

         //  The current edge connected to the _base.
      int   _curr ;

         //  The edge that we started looking at first
      int   _begin ;

      void _Copy( const HexahedronEdgeIterator& ) ;
      void _IsValidIterator() const ;

   public :

         //  Constructors, Destructor
      HexahedronEdgeIterator () ;
      HexahedronEdgeIterator ( const int ) ;
      HexahedronEdgeIterator ( const int, const int ) ;
      HexahedronEdgeIterator
         ( const HexahedronEdgeIterator& ) ;
      virtual ~HexahedronEdgeIterator () ;

         //  Assignment
      HexahedronEdgeIterator& operator=
         ( const HexahedronEdgeIterator& ) ;

         //  Output

      friend ostream& operator<<
         ( ostream&, const HexahedronEdgeIterator& ) ;

         //  Comparison
      friend int operator==
         ( const HexahedronEdgeIterator&, const HexahedronEdgeIterator& ) ;
      friend int operator!=
         ( const HexahedronEdgeIterator&, const HexahedronEdgeIterator& ) ;

         //  Access functions for point iteration

         //  Calculations
      int   Next() ;
      int   Current() const ;
      int   GetBaseVertex() const ;
      int   GetOtherVertex() const ;
      int   End() const ;
      void  Reset() ;

      friend int  GetOtherVertexConnectedToEdge( const int, const int ) ;
      friend int  GetFaceConnectedToEdges( const int, const int ) ;

} ;

int  GetOtherVertexConnectedToEdge( const int, const int ) ;
int  GetFaceConnectedToEdges( const int, const int ) ;

//  ============================================================
//                  HexahedronFaceIterator
//  ============================================================

class HexahedronFaceIterator
{
   protected :
   
         //  The "base" vertex in the hexahedron whose
         //    3 faces we will iterate around
      int   _base ;

         //  The current face connected to the _base.
      int   _curr ;

         //  The face that we started looking at first
      int   _begin ;

      void _Copy( const HexahedronFaceIterator& ) ;
      void _IsValidIterator() const ;

   public :

         //  Constructors, Destructor
      HexahedronFaceIterator () ;
      HexahedronFaceIterator ( const int ) ;
      HexahedronFaceIterator ( const int, const int ) ;
      HexahedronFaceIterator
         ( const HexahedronFaceIterator& ) ;
      virtual ~HexahedronFaceIterator () ;

         //  Assignment
      HexahedronFaceIterator& operator=
         ( const HexahedronFaceIterator& ) ;

         //  Output
      friend ostream& operator<<
         ( ostream&, const HexahedronFaceIterator& ) ;

         //  Comparison
      friend int operator==
         ( const HexahedronFaceIterator&, const HexahedronFaceIterator& ) ;
      friend int operator!=
         ( const HexahedronFaceIterator&, const HexahedronFaceIterator& ) ;

         //  Access functions for point iteration

         //  Calculations
      int   Next() ;
      int   Current() const ;
      int   GetBaseVertex() const ;
      int   End() const ;
      void  Reset() ;
} ;

/*
class HexahedronEdge
{

   protected :
   
      int   _edgePts[2] ;

      void _Copy( const ConnectedHexahedron& ) ;

   public :

         //  Constructors

      HexahedronEdge () ;

      HexahedronEdge ( const int, const int ) ;

         //  Copy Constructor

      HexahedronEdge ( const HexahedronEdge& ) ;

         //  Destructor

      virtual ~HexahedronEdge () ;

         //  Assignment

      HexahedronEdge& operator= ( const HexahedronEdge& ) ;

         //  Output

      friend ostream& operator<< ( ostream&, const HexahedronEdge& ) ;

         //  Comparison

      friend int operator==
         ( const HexahedronEdge&, const HexahedronEdge& ) ;
      friend int operator!=
         ( const HexahedronEdge&, const HexahedronEdge& ) ;

         //  Calculations

         //  Draw


} ;
*/

#endif
