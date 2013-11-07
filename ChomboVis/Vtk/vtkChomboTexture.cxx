/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** This software is copyright (C) by the Lawrence Berkeley
** National Laboratory.  Permission is granted to reproduce
** this software for non-commercial purposes provided that
** this notice is left intact.
** 
** It is acknowledged that the U.S. Government has rights to
** this software under Contract DE-AC03-765F00098 between
** the U.S. Department of Energy and the University of
** California.
**
** This software is provided as a professional and academic
** contribution for joint exchange.  Thus it is experimental,
** is provided ``as is'', with no warranties of any kind
** whatsoever, no support, no promise of updates, or printed
** documentation.  By using this software, you acknowledge
** that the Lawrence Berkeley National Laboratory and
** Regents of the University of California shall have no
** liability with respect to the infringement of other
** copyrights by any part of this software.
**
*/

#include <cassert>
#include <iostream>
#include <vector>
#include <sstream>

#include "vtkStructuredPoints.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkDataArray.h"
#include "vtkPointData.h"

#include "vtkChomboReader.h"
#include "../utils/Trace.h"
#include "../utils/RectanglePacker.h"
#include "vtkChomboTexture.h"
#include "vtkThresholdPoints.h"
#include "vtkObjectFactory.h"

using std::vector;
using std::pair;
using std::ostringstream;


template<class T> void _PermuteCoords( T * point, char direction );
template<class T> void _InversePermuteCoords( T * point, char direction );

vtkStandardNewMacro(vtkChomboTexture);

vtkChomboTexture::vtkChomboTexture()
  : m_reader(0),
    m_textureMemory(0),
    m_rectanglePacker(0)
{
    m_dblBuf = new double[8];
}


vtkChomboTexture::~vtkChomboTexture()
{
    delete [] m_dblBuf;
}


void
vtkChomboTexture::SetChomboReader( vtkChomboReader * reader )
{
    Trace t("vtkChomboTexture::vtkChomboTexture()"); t.NoOp();
    assert( reader->GetNumDims() != 0 ); // Would be a sign no hdf5 is loaded.

    m_reader = reader;
}


/** The (texture) memory partition defines how we pack all the visible boxes
 *  into the (texture) memory space.
 *
 *  Arg direction is 'x', 'y' or 'z' -- the slice's normal.
*/
void
vtkChomboTexture::UpdateMemoryPartition( char direction )
{
    Trace t("vtkChomboTexture::UpdateMemoryPartition()");
        // Assemble the collection of rectangles (i.e. boxes) we're going to
        // pack.  We call them rectangles because we have a class CVRectangle
        // in utils/RectPacking.h.  It's like a lightweight Box, but in any case
        // we didn't want to use the actual Box class from utils/ChomboHDF5.h,
        // because this way RectPacking stays nice and independent.
        vector<CVRectangle> rects;
        assert( m_reader );
        int nPieces = m_reader->GetNumPieces();
        rects.reserve( nPieces );
        for( int b=0; b<nPieces; ++b )
        {
            int level  = m_reader->GetVisibleBoxLevel( b );
            int boxNum = m_reader->GetVisibleBoxNum( b );

            int * temp = m_reader->GetBoxOrigin( level, boxNum );
            int boxOrigin[3];
            memcpy( boxOrigin, temp, 3*sizeof(int) );
            _PermuteCoords<int>(boxOrigin, direction);

            temp = m_reader->GetBoxDims( level, boxNum );
            int boxDims[3];
            memcpy( boxDims, temp, 3*sizeof(int) );
            _PermuteCoords<int>( boxDims, direction );

            Corner c1(boxOrigin[0],              boxOrigin[1]);
            Corner c2(boxOrigin[0]+boxDims[0]-1, boxOrigin[1]+boxDims[1]-1);
            CVRectangle rect( c1,c2, b );
            rects.push_back( rect );
        }

        // Gotta cast, because VTK Python wrapper chokes on the mere forward
        // declaration of class RectanglePacker.
        RectanglePacker * packer = (RectanglePacker *)m_rectanglePacker;

        delete packer; // Initialized to 0, so OK.
        packer = new OKRectanglePacker( rects );
        packer->Pack();

        /*
        ostringstream ost;
        ost << "packing." << direction << ".ps";
        packer->PackedToPostscript( ost.str() );
        PackStats packStats;
        packer->Stats( packStats );
        cout << packStats << endl;
        */

        m_rectanglePacker = (int *)packer;
}


/** Allocate and fill the texture memory, then return it.  The texture memory
 *  contains the values we want the texture to represent, packed according to
 *  the partition calculated in this->UpdateMemoryPartition().
 *
 *  Arg direction is 'x', 'y' or 'z' -- the slice's normal.
*/
vtkStructuredPoints *
vtkChomboTexture::MakeTextureMemory( char direction, double position )
{
    Trace t("vtkChomboTexture::MakeTextureMemory()");
    assert( m_reader );
    assert( m_rectanglePacker );

    //
    // Define size, and allocate memory.
    //
    m_textureMemory = vtkStructuredPoints::New();
    CVRectangle packedDomain =
        ((RectanglePacker *)m_rectanglePacker)->GetPackedDomain();
    m_textureMemory->SetDimensions(
        std::max(1 + packedDomain.m_hi.first  - packedDomain.m_lo.first,2),
        1 + packedDomain.m_hi.second - packedDomain.m_lo.second,
        1 );
    // Gotta make sure texture memory is at least 2 wide, or we get candy-cane
    // effect.
    
    int n = m_textureMemory->GetNumberOfPoints();
    vtkDataArray * memoryScalars;
    if( m_reader->GetPrecision() == 2 )
    {
        memoryScalars = vtkFloatArray::New();
        ((vtkFloatArray*)memoryScalars)->SetNumberOfValues( n );
    } else
    {
        memoryScalars = vtkDoubleArray::New();
        ((vtkDoubleArray*)memoryScalars)->SetNumberOfValues( n );
    }

    int axisNum = 1 * (direction=='y')
                + 2 * (direction=='z');

    //
    // Fill it up.
    //
    int component = m_reader->GetCurrentComponentNum();
    for( int b=0;b<m_reader->GetNumPieces();++b )
    {
        int level  = m_reader->GetVisibleBoxLevel( b );
        int boxNum = m_reader->GetVisibleBoxNum( b );
        if( ! m_reader->PlaneIntersectsBox( level, boxNum,
                                           direction,
                                           position ) )
        {
            continue;
        }

        int * temp = m_reader->GetBoxDims( level, boxNum );
        int boxDims[3];
        memcpy( boxDims, temp, 3*sizeof(int) );

        CVRectangle rect(
          ((RectanglePacker *)m_rectanglePacker)->GetPackedRectangle(b) );
        pair<int,int> rectDims =
            pair<int,int>( 1 + rect.m_hi.first - rect.m_lo.first,
                           1 + rect.m_hi.second - rect.m_lo.second );

        int ii, jj, kk; // Indices into GetDatum().

        double coordinateOrigin[3];
        memcpy( coordinateOrigin, m_reader->GetCoordinateOrigin(),
                3*sizeof(double) );

        for( int i=0; i<rectDims.first; ++i )
        {
            for( int j=0;j<rectDims.second; ++j )
            {
                int indexIntoTextureMemory =
                   std::max(
                       (1 + packedDomain.m_hi.first - packedDomain.m_lo.first),
                        2)
                   * (j + rect.m_lo.second)  +  (i + rect.m_lo.first);

                int origin = m_reader->GetBoxOrigin( level, boxNum )[axisNum];
                    
                int slice_position_ijk = int(
                    (position - coordinateOrigin[axisNum]) /
                    m_reader->GetLevelDx(level)[axisNum] - origin );
                int coords[3] = {i,j,slice_position_ijk};
                if( m_reader->Is3DMode() || m_reader->IsSlicingMode() )
                {
                    _InversePermuteCoords( coords, direction );
                }
                
                ii = std::max(0, std::min(coords[0], boxDims[0]-1));
                jj = std::max(0, std::min(coords[1], boxDims[1]-1));
                kk = std::max(0, std::min(coords[2], boxDims[2]-1));

                if( m_reader->GetPrecision() == 2 )
                {
                    ((vtkFloatArray*)memoryScalars)->SetValue(
                        indexIntoTextureMemory,
                        m_reader->GetDatum( level, boxNum, component,
                                            ii,jj,kk ));
                } else
                {
                    ((vtkDoubleArray*)memoryScalars)->SetValue(
                        indexIntoTextureMemory,
                        m_reader->GetDatum( level, boxNum, component,
                                            ii,jj,kk ));
                }
            }
        }
    }

    m_textureMemory->GetPointData()->SetScalars( memoryScalars );
    memoryScalars->Delete();

    return m_textureMemory;
}


/** Return the four corners of the boxNum-th box of the "visible pieces",
 *  as fractions of the packed domain's width and height.
*/
double *
vtkChomboTexture::GetMemoryPartition( int boxNum ) const
{
    CVRectangle rect(
        ((RectanglePacker *)m_rectanglePacker)->GetPackedRectangle( boxNum ));
    int * memDims = m_textureMemory->GetDimensions();
    double const fudge = 0.001;  // prevents stray pixels from tiny "gaps"

    m_dblBuf[0] = (rect.m_lo.first + fudge)/double(memDims[0]);
    m_dblBuf[1] = (rect.m_lo.second + fudge)/double(memDims[1]);

    m_dblBuf[2] = (1+rect.m_hi.first - fudge)/double(memDims[0]);
    m_dblBuf[3] = (rect.m_lo.second + fudge)/double(memDims[1]);

    m_dblBuf[4] = (1+rect.m_hi.first - fudge)/double(memDims[0]);
    m_dblBuf[5] = (1+rect.m_hi.second - fudge)/double(memDims[1]);

    m_dblBuf[6] = (rect.m_lo.first + fudge)/double(memDims[0]);
    m_dblBuf[7] = (1+rect.m_hi.second - fudge)/double(memDims[1]);

    return m_dblBuf;
}

/**  Arg point is a triple.  We permute its three coordinates to what
     they would be if arg direction were the axis pointing out of the screen,
     rather than (by default) the z axis.
*/
template<class T> void
_PermuteCoords( T * point, char direction )
{
    assert( direction=='x' || direction=='y' || direction=='z' );
    static int const permutations[3][3] = {{1,2,0},{2,0,1},{0,1,2}};
    int direction_num = 1 * (direction=='y')  +  2 * (direction=='z');
    T temp[3];

    for( int i=0;i<3;++i )
    {
        temp[i] = point[permutations[direction_num][i]];
    }
    memcpy( point, temp, 3*sizeof(T) );
}


/** Inverse to _PermuteCoords */
template<class T> void
_InversePermuteCoords( T * point, char direction )
{
    assert( direction=='x' || direction=='y' || direction=='z' );
    static int const permutations[3][3] = {{2,0,1},{1,2,0},{0,1,2}};
    int direction_num = 1 * (direction=='y')  +  2 * (direction=='z');
    T temp[3];

    for( int i=0;i<3;++i )
    {
        temp[i] = point[permutations[direction_num][i]];
    }
    memcpy( point, temp, 3*sizeof(T) );
}
