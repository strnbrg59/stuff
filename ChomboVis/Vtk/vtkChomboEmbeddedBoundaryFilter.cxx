/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** vtkChomboEmbeddedBoundaryFilter.cxx 
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

/* provided as a modification from source material derived from
   the VTK distribution.  In accordance with VTK software agreement
   the following also holds:
=========================================================================



Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#define INTERP_DATA     0

#define MYDEBUG_0       0
#define MYDEBUG_1       0
#define MYDEBUG_2       0

#include "vtkChomboEmbeddedBoundaryFilter.h"
#include "../VtkImpl/vtkArray.h"
#include "vtkObjectFactory.h"
#include "vtkDataArray.h"
#include "vtkStructuredPoints.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"

#include "../EBLib/EBSurfaceLib.h"

#include <fcntl.h>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkChomboEmbeddedBoundaryFilter);

vtkChomboEmbeddedBoundaryFilter::vtkChomboEmbeddedBoundaryFilter ()
{
  nDim = 0;
  Plane = 0;
  NormalDir = 1;
  Capping = 1;
}

vtkChomboEmbeddedBoundaryFilter::~vtkChomboEmbeddedBoundaryFilter ()
{
}

void vtkChomboEmbeddedBoundaryFilter::Execute()
{
  this->GetOutput(); // Do we need this for some side effect?
  
  vtkDebugMacro(<< "Creating dataset outline");

#if MYDEBUG_0
fprintf(stderr,"Entering Execute...\n");
#endif
  //
  // Let OutlineSource do all the work
  //
  this->ExecuteEB();
}


/** This function, and the corresponding trilinear one, are not used.
 *  But they look interesting enough, so I'll just comment them out (TDS)
template <class T>
static T vtkChomboEmbeddedBoundaryFilterBilinearInterpolate
(
  const double px, const double py,
  const int i, const int j,
  T *data,
  const int dim[2],
  const float space[2]
)
{
    //
    //  NOTE: We assume that the point data was generated in a cell
    //    rooted at the origin
    //

    //
    //  The corners of the quadrant to use for the bilinear interpolation
    //
    //          1 ---- 3
    //          |      |       y
    //          |      |       |
    //          |      |       *-- x
    //          0 ---- 2
    //
  T corners[4] ;
  int indices[4] ;
  int lx, ly, hx, hy ;
  int iter ;
  double xw, yw ;

    //  The weight for linear interpolation is obtained by
    //    scaling the point in each dimension
  xw = px / space[0] ;
  yw = py / space[1] ;

    //  Determine which 8 cell points to use
    //    The weights must be adjusted by (+/-)0.5.
  if( xw > 0.5 )
  {
    lx = i ;
    hx = ( (i+1) >= dim[0] ? (dim[0]-1) : (i+1) ) ;
    xw -= 0.5 ;
  }
  else
  {
    lx = ( (i-1) < 0 ? 0 : (i-1) ) ;
    hx = i ;
    xw += 0.5 ;
  }

  if( yw > 0.5 )
  {
    ly = j ;
    hy = ( (j+1) >= dim[1] ? (dim[1]-1) : (j+1) ) ;
    yw -= 0.5 ;
  }
  else
  {
    ly = ( (j-1) < 0 ? 0 : (j-1) ) ;
    hy = j ;
    yw += 0.5 ;
  }

    //  Assume the index is generated by iterating
    //    for( j )
    //      for( i )
  #define getBiIndex( jj, ii ) ( (jj)*dim[0] + (ii) )
  indices[0] = getBiIndex(ly, lx) ;
  indices[1] = getBiIndex(hy, lx) ;
  indices[2] = getBiIndex(ly, hx) ;
  indices[3] = getBiIndex(hy, hx) ;

  for( iter=0; iter<4; iter++ )
  {
    corners[iter] = data[indices[iter]] ;
  }

    //  Perform standard bilinear interpolation

    // Reduce 4 points to 2 points that form a Y line.
    // Linearly interpolate 2 points to 1 point.
  
    // Let DA = Density average, T=top, B=bottom
  T x_B_DA = ( 1 - xw ) * corners[0] + xw * corners[2] ;
  T x_T_DA = ( 1 - xw ) * corners[1] + xw * corners[3] ;
  
    // Let DA = Density average
  T y_DA = ( 1 - yw ) * x_B_DA + yw * x_T_DA ;
  
  return y_DA ;
}

template <class T>
static T vtkChomboEmbeddedBoundaryFilterTrilinearInterpolate
(
  const double px, const double py, const double pz,
  const int i, const int j, const int k,
  T *data,
  const int dim[3],
  const float space[3]
)
{
    //
    //  NOTE: We assume that the point data was generated in a cell
    //    rooted at the origin
    //

    //
    //  The corners of the octant to use for the trilinear interpolation
    //
    //          2 ---- 6
    //         /|     /|       y
    //        3 ---- 7 |       |
    //        | |    | |       *-- x
    //        | 0 ---| 4      /
    //        |/     |/      z
    //        1 ---- 5
    //
  T corners[8] ;
  int indices[8] ;
  int lx, ly, lz, hx, hy, hz ;
  int iter ;
  double xw, yw, zw ;

    //  The weight for linear interpolation is obtained by
    //    scaling the point in each dimension
  xw = px / space[0] ;
  yw = py / space[1] ;
  zw = pz / space[2] ;

    //  Determine which 8 cell points to use
    //    The weights must be adjusted by (+/-)0.5.
  if( xw > 0.5 )
  {
    lx = i ;
    hx = ( (i+1) >= dim[0] ? (dim[0]-1) : (i+1) ) ;
    xw -= 0.5 ;
  }
  else
  {
    lx = ( (i-1) < 0 ? 0 : (i-1) ) ;
    hx = i ;
    xw += 0.5 ;
  }

  if( yw > 0.5 )
  {
    ly = j ;
    hy = ( (j+1) >= dim[1] ? (dim[1]-1) : (j+1) ) ;
    yw -= 0.5 ;
  }
  else
  {
    ly = ( (j-1) < 0 ? 0 : (j-1) ) ;
    hy = j ;
    yw += 0.5 ;
  }

  if( zw > 0.5 )
  {
    lz = k ;
    hz = ( (k+1) >= dim[2] ? (dim[2]-1) : (k+1) ) ;
    zw -= 0.5 ;
  }
  else
  {
    lz = ( (k-1) < 0 ? 0 : (k-1) ) ;
    hz = k ;
    zw += 0.5 ;
  }

    //  Assume the index is generated by iterating
    //  for( k )
    //    for( j )
    //      for( i )
  #define getTriIndex( kk, jj, ii ) ( ( (kk)*dim[1] + (jj) )*dim[0] + (ii) )
  indices[0] = getTriIndex(lz, ly, lx) ;
  indices[1] = getTriIndex(hz, ly, lx) ;
  indices[2] = getTriIndex(lz, hy, lx) ;
  indices[3] = getTriIndex(hz, hy, lx) ;
  indices[4] = getTriIndex(lz, ly, hx) ;
  indices[5] = getTriIndex(hz, ly, hx) ;
  indices[6] = getTriIndex(lz, hy, hx) ;
  indices[7] = getTriIndex(hz, hy, hx) ;

  for( iter=0; iter<8; iter++ )
  {
    corners[iter] = data[indices[iter]] ;
  }

    //  Perform standard trilinear interpolation

    // Reduce 8 points to 4 points that form a YZ square.
    // Reduce 4 points to 2 points that form a Z line.
    // Linearly interpolate 2 points to 1 point.
  
    // Let DA = Density average, 1: T=top, B=bottom, 2 : B=back, F=front
  T x_BB_DA = ( 1 - xw ) * corners[0] + xw * corners[4] ;
  T x_BF_DA = ( 1 - xw ) * corners[1] + xw * corners[5] ;
  T x_TB_DA = ( 1 - xw ) * corners[2] + xw * corners[6] ;
  T x_TF_DA = ( 1 - xw ) * corners[3] + xw * corners[7] ;
  
    // Let DA = Density average, B=back, T=Front
  T y_B_DA = ( 1 - yw ) * x_BB_DA + yw * x_TB_DA ;
  T y_T_DA = ( 1 - yw ) * x_BF_DA + yw * x_TF_DA ;
  
  T z_DA = ( 1 - zw ) * y_B_DA + zw * y_T_DA ;

  return z_DA ;
}
*/


class EBIndexer : public Indexer
   {
      public :
         GridInfo * grid ;
         int index ( int i, int j, int k )
            {
               if ( grid )
               {
                  int x,y ;
                  x = grid->dim.x ;
                  y = grid->dim.y ;

                  return ( (k*y + j)*x + i ) ;
               }
               else
               {
                  return 0 ;
               }
            }
         void reverse ( int id, int &i, int &j, int &k )
            {
               if ( grid )
               {
                  int x,y,xy,r ;
                  x = grid->dim.x ;
                  y = grid->dim.y ;

                  if ( x <= 0 || y <= 0 )
                  {
                    i = 0 ;
                    j = 0 ;
                    k = 0 ;
                  }
                  else
                  {
                    xy = x*y;

                    k = id / xy;
                    r = id % xy;

                    j = r / x;
                    i = r % x;
                  }
               }
               else
               {
                  i = 0 ;
                  j = 0 ;
                  k = 0 ;
               }
            }
   } ;

template <class T>
static void vtkChomboEmbeddedBoundaryFilterEB
(
  vtkStructuredPoints *input,
  T * ebComps[5], // vof, dist, nx, ny, nz
  T *data,
  vtkPolyData *output,
  int nDim,
  int Plane,
  int NormalDir,
  int Capping
)
{
  T * vof( ebComps[0] );
  T * dist( ebComps[1] );
  T * nx( ebComps[2] );
  T * ny( ebComps[3] );
  T * nz( ebComps[4] );

/* TDS debugging stuff -- for gcc3.4.2 segfault problem.
  cerr << "Plane=" << Plane << endl;
  cerr << "NormalDir=" << NormalDir << endl;
  cerr << "Capping=" << Capping << endl;
*/

  vtkPoints *newPts =NULL ;
  vtkCellData *outCellData =NULL ;
  vtkDataArray *newScalars =NULL ;

  int index;
  int dim[3] ;
  double * ptorig = new double[3];
  double * ptspace = new double[3];
  
  input->GetOrigin( ptorig[0], ptorig[1], ptorig[2] );
  input->GetSpacing(ptspace[0], ptspace[1], ptspace[2]);
  input->GetDimensions(dim);
  outCellData = output->GetCellData() ;


  /* TDS debugging stuff.
  // Dump EB component data to file.
  //
  int fd = open( "ebcomps.dat", O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR );
  cerr << "fd=" << fd << endl;
  assert( fd != -1 );    
  int n = dim[0]*dim[1]*dim[2];
  write( fd, vof, n*sizeof(T) );
  write( fd, dist, n*sizeof(T) );
  write( fd, nx, n*sizeof(T) );
  write( fd, ny, n*sizeof(T) );
  write( fd, nz, n*sizeof(T) );
  close( fd );
  */

      // Configure EB surface generation object
   GridInfo           grid ;
   EBIndexer          indexer ;
   EBSurfaceGenerator ebgen ;
   {
      int i ;
      for( i=0; i<3; i++ )
      {
         /* TDS debugging stuff
         cerr << "dim[" << i << "]=" << dim[i] << endl;
         cerr << "ptorig[" << i << "]=" << ptorig[i] << endl;
         cerr << "ptspace[" << i << "]=" << ptspace[i] << endl;
         */
         grid.dim[i] = dim[i] ;
         grid.space[i] = ptspace[i] ;
         grid.origin[i] = ptorig[i] ;
      }
   }
   indexer.grid = &grid ;
   ebgen.index  = &indexer ;
   ebgen.capping( Capping ) ;
   if( nDim == 2 )
   {
      ebgen.gridMode( EBSINGLE::GRID2D ) ;
   }
   else
   if( nDim == 3 )
   {
      ebgen.gridMode( EBSINGLE::GRID3D ) ;
   }
   else
   {
      cerr << "vtkChomboEmbeddedBoundaryFilter::ExecuteEB: " << endl ;
      cerr << "\tCannot calculate embedded boundary for data with " ;
      cerr << nDim << " dimensions" << endl ;
      return ;
   }
   ebgen.setGrid( grid ) ;

   if( Plane == 1 )
   {
      ebgen.constraintMode( EBSINGLE::CONSTRAIN_DISTANCE ) ;
   }
   else
   {
      ebgen.constraintMode( EBSINGLE::CONSTRAIN_VF_NORMAL ) ;
   }

#if MYDEBUG_0
fprintf(stderr,"    Origin:  %g %g %g...\n",ptorig[0],ptorig[1],ptorig[2]);
fprintf(stderr,"    Spacing: %g %g %g...\n",ptspace[0],ptspace[1],ptspace[2]);
fprintf(stderr,"    Dim: %d %d %d...\n",dim[0],dim[1],dim[2]);
#endif

   if( nDim == 2 )
   {
#if MYDEBUG_0
cerr << "generating 2D EB" << endl ;
#endif
      T * normals = NULL ;
      vtkCellArray *newLines;

      int i,j ;

      //  Allocate space for data
      newPts = vtkPoints::New() ;
      newLines = vtkCellArray::New() ;
      newScalars = vtkArray<T>::New();

      // Create a single array for normal data.
      //    Normals must be flipped for Greg Miller's data.
      normals = new T [ 2*dim[1]*dim[0] ] ;
      index = 0 ;
      for( j=0; j<dim[1]; j++ )
      {
         for( i=0; i<dim[0]; i++ )
         {
            if (NormalDir == 0) {
               normals[index*2  ] =  nx[index] ;
               normals[index*2+1] =  ny[index] ;
            } else {
               normals[index*2  ] = -nx[index] ;
               normals[index*2+1] = -ny[index] ;
            }
            index++ ;
         }
      }

// TODO : worry about scalar mapping
         // Compute the EB over the entire block
      ebgen.compute( vof, dist, normals, sizeof(T), sizeof(T), 2*sizeof(T) ) ;

      delete [] normals ;

         // Pass the EB geometry to VTK
      unsigned int c ;
      int ptId[2];
      double vtkSegData[2*3*1];
      double * line ;
      for( c=0; c<ebgen.size(); c++ )
      {
         int *indices;
         int sid;
         T value;

         indices = ebgen.cellId(c);

         if (indices[0] < 0)
         {
           value = data[indices[1]];
         }
         else
         {
           value = 0.5*(data[indices[0]] + data[indices[1]]);
         }

         line = ebgen.line( c ) ;
         vtkSegData[0] = line[0] ;
         vtkSegData[1] = line[1] ;
         vtkSegData[2] = 0.0 ;
         vtkSegData[3] = line[2] ;
         vtkSegData[4] = line[3] ;
         vtkSegData[5] = 0.0 ;

         ptId[0] = newPts->InsertNextPoint(&(vtkSegData[0])) ;
         ptId[1] = newPts->InsertNextPoint(&(vtkSegData[3])) ;

         newLines->InsertNextCell( 2, ptId ) ;
         sid = ((vtkArray<T>*)newScalars)->InsertNextValue(value) ;
      }

#if MYDEBUG_0
fprintf(stderr,"    Set points...\n");
#endif

      output->SetPoints(newPts);
      newPts->Delete();

#if MYDEBUG_0
fprintf(stderr,"    Set lines...\n");
#endif
      output->SetLines(newLines);
      newLines->Delete();

#if MYDEBUG_0
fprintf(stderr,"    Set scalars...\n");
#endif
      outCellData->SetScalars(newScalars);
      newScalars->Delete();
#if MYDEBUG_0
fprintf(stderr,"  Done...\n");
#endif
   }
   else
   if( nDim == 3 )
   {
#if MYDEBUG_0
cerr << "generating 3D EB" << endl ;
#endif

      T * normals = NULL ;
      vtkCellArray *newPolys;

      int i,j,k ;

         //  Allocate space for data
      newPts = vtkPoints::New() ;
      newPolys = vtkCellArray::New() ;
      newScalars = vtkArray<T>::New();

         // Create a single array for normal data.
         //    Normals must be flipped for Greg Miller's data.
      normals = new T [ 3*dim[2]*dim[1]*dim[0] ] ;
      index = 0 ;
      for( k=0; k<dim[2]; k++ )
      {
         for( j=0; j<dim[1]; j++ )
         {
            for( i=0; i<dim[0]; i++ )
            {
               if (NormalDir == 0) {
                  normals[index*3  ] =  nx[index] ;
                  normals[index*3+1] =  ny[index] ;
                  normals[index*3+2] =  nz[index] ;
               } else {
                  normals[index*3  ] = -nx[index] ;
                  normals[index*3+1] = -ny[index] ;
                  normals[index*3+2] = -nz[index] ;
               }
               index++ ;
            }
         }
      }

         // Compute the EB over the entire block
      /* TDS: here's where the segfault occurs, under gcc3.4.2:*/
      //cerr << "Entering ebgen.compute()..." << endl;
      ebgen.compute( vof, dist, normals, sizeof(T), sizeof(T), 3*sizeof(T) ) ;
      //cerr << "Exited ebgen.compute()" << endl;
      /**/

      delete [] normals ;

         // Pass the EB geometry to VTK
      unsigned int c;
      int *ptId;
      double *poly;
      for( c=0; c<ebgen.size(); c++ )
      {
         int polySize;
         int *indices;
         int sid;
         T value;

         indices = ebgen.cellId(c);

         if (indices[0] < 0)
         {
           value = data[indices[1]];
         }
         else
         {
           value = 0.5*(data[indices[0]] + data[indices[1]]);
         }

         poly = ebgen.poly( c, polySize );

         ptId = new int [polySize];
         for (int pi = 0; pi < polySize; pi++)
         {
            ptId[pi] = newPts->InsertNextPoint(&(poly[pi*3]) ) ;
         }

         newPolys->InsertNextCell( polySize, ptId ) ;
         sid = ((vtkArray<T>*)newScalars)->InsertNextValue(value) ;

         delete [] ptId;
      }

#if MYDEBUG_0
fprintf(stderr,"    Set points...\n");
#endif
      output->SetPoints(newPts);
      newPts->Delete();

#if MYDEBUG_0
fprintf(stderr,"    Set polys...\n");
#endif
      output->SetPolys(newPolys);
      newPolys->Delete();

#if MYDEBUG_0
fprintf(stderr,"    Set scalars...\n");
#endif
      outCellData->SetScalars(newScalars);
      newScalars->Delete();

#if MYDEBUG_0
fprintf(stderr,"  Done...\n");
#endif
   }

  delete [] ptorig;
  delete [] ptspace;
}

void vtkChomboEmbeddedBoundaryFilter::ExecuteEB()
{
  vtkStructuredPoints *input = (vtkStructuredPoints*) this->Inputs[0];
  vtkPolyData *output = (vtkPolyData*) this->Outputs[0];
  vtkFieldData *field = input->GetFieldData();
  vtkPointData *pointdata = input->GetPointData();
  vtkDataArray *inScalars = pointdata->GetScalars();

  if (field->GetNumberOfArrays() == 0) return;

#if MYDEBUG_0
fprintf(stderr,"  Entering ExecuteEB...\n");
#endif
//  vtkDebugMacro(<< "Generating embedded boundary");

  switch (field->GetArray(0)->GetDataType()) {
    case VTK_FLOAT:
    {
      float * ebComps[5];
      ebComps[4] = NULL;
      for( int i=0;i<2+this->nDim;++i )
      {
        ebComps[i] = ((vtkFloatArray *)(field->GetArray(i)))->GetPointer(0);
      }

      float *data = ((vtkFloatArray *)inScalars)->GetPointer(0);
#if MYDEBUG_0
fprintf(stderr,"    Calling float function (%dD)...\n", this->nDim);
#endif
      vtkChomboEmbeddedBoundaryFilterEB
        (input,ebComps,data,output,
         this->nDim,this->Plane,this->NormalDir,this->Capping);
    }
    break;

    case VTK_DOUBLE:
    {
      double * ebComps[5];
      ebComps[4] = NULL;
      for( int i=0;i<2+this->nDim;++i )
      {
        ebComps[i] = ((vtkDoubleArray *)(field->GetArray(i)))->GetPointer(0);
      }

      double *data = ((vtkDoubleArray *)inScalars)->GetPointer(0);
#if MYDEBUG_0
fprintf(stderr,"    Calling double function (%dD)...\n", this->nDim);
#endif
      vtkChomboEmbeddedBoundaryFilterEB
        (input,ebComps,data,output,
         this->nDim,this->Plane,this->NormalDir,this->Capping);
    }
    break;
  }
#if MYDEBUG_0
fprintf(stderr,"  Leaving ExecuteEB...\n");
#endif
}

void vtkChomboEmbeddedBoundaryFilter::ExecuteInformation()
{
  this->GetOutput(); // Do we need this for some side effect?
  
  vtkDebugMacro(<< "Creating dataset embedded boundary");

  //
  // Let OutlineSource do all the work
  //
  
  this->vtkSource::ExecuteInformation();
}
