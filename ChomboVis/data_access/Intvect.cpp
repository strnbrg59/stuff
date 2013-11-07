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
// Author: Ted Sternberg

#include "../utils/ch_config.h"
#include "Intvect.h"
#include "../utils/Trace.h"
#include "../utils/Consts.h"
#include <typeinfo>
#include <string>
#include <cmath>

using std::string;


/** Only needed for Intvect, and always rounds down. */
template<> void
Triple<int>::MultiplyAndRoundDown( double x )
{
    int temp;
    for( int i=0;i<THREE;++i )
    {
        temp = int(floor(m_data[i] * x + 1E-10));
        // We want to round down, but we need the 1E-10 to prevent 5.0,
        // represented internally as 4.99999999, from being "rounded" all the
        // way down to 4.
        m_data[i] = temp;
    }
}


TEMPLATE_ANGLE_ANGLE hid_t Triple<int>::s_HDF5Datatype(0);
TEMPLATE_ANGLE_ANGLE hid_t Triple<float>::s_HDF5Datatype(0);
TEMPLATE_ANGLE_ANGLE hid_t Triple<double>::s_HDF5Datatype(0);


/** Imax, Imin, Jmax, Jmin: return one of two Intvect args, according to which
 *  one of them has the larger i or j coordinate.
*/
/*static*/ TEMPLATE_ANGLE_ANGLE Intvect
Intvect::Imax( Intvect const & iv1, Intvect const & iv2 )
{
    return (iv1.i() > iv2.i()) ? iv1:iv2;
}
/*static*/ TEMPLATE_ANGLE_ANGLE Intvect
Intvect::Imin( Intvect const & iv1, Intvect const & iv2 )
{
    return (iv1.i() > iv2.i()) ? iv2:iv1;
}
/*static*/ TEMPLATE_ANGLE_ANGLE Intvect
Intvect::Jmax( Intvect const & iv1, Intvect const & iv2 )
{
    return (iv1.j() > iv2.j()) ? iv1:iv2;
}
/*static*/ TEMPLATE_ANGLE_ANGLE Intvect
Intvect::Jmin( Intvect const & iv1, Intvect const & iv2 )
{
    return (iv1.j() > iv2.j()) ? iv2:iv1;
}


template<> Triple<int>
MultiplyAndRoundDown( Triple<int> const & iv1, double x )
{
    Triple<int> result( iv1 );
    result.MultiplyAndRoundDown( x );
    return result;
}


/** Tell HDF5 what it needs to know in order to load compound data from disk to
 *  memory.
 *  Arg dimensionality is the official dimensionality of the hdf5 file.
 *  We note that only because it affects how many fields (i,j,k)
 *  appear in the file.  But we always construct, and use, 3-long Intvects
 *  (as well as Box objects made from 3-long Intvects).
*/
/*static*/ template<typename T> void
Triple<T>::InitializeHDF5Datatype( int dimensionality )
{
    Trace t("Triple::InitializeHDF5Datatype()");

    s_HDF5Datatype = H5Tcreate( H5T_COMPOUND, sizeof(Triple<T>) );
    Triple<T> iv;
    int offset = (char *)(&(iv.m_data)) - (char *)&iv;

    // Figure out what type T is.  Unfortunately, we need to know that.
    hid_t hdf5Type(0);
    if( typeid(iv.m_data[0]) == typeid(int) )
    {
        hdf5Type = H5T_NATIVE_INT;
    } else
    if( typeid(iv.m_data[0]) == typeid(float) )
    {
        hdf5Type = H5T_NATIVE_FLOAT;
    } else
    if( typeid(iv.m_data[0]) == typeid(double) )
    {
        hdf5Type = H5T_NATIVE_DOUBLE;
    } else
    {
        t.FatalError( "Unsupported type for Triple<T> -- neither int, nor "
            "float nor double." );
    }

    string intvectComponentNames[3] =
        {Consts::intvecti, Consts::intvectj, Consts::intvectk };
    string realvectComponentNames[3] =
        {Consts::realvectx, Consts::realvecty, Consts::realvectz };
    // Note the components Consts::intvecti, etc are only labels in the Chombo
    // HDF5 format.  They are not the names of data
    // members of our struct Intvect.

    for( int i=0; i<dimensionality; ++i )
    {
        if( hdf5Type == H5T_NATIVE_INT )
        {
            H5Tinsert( s_HDF5Datatype, intvectComponentNames[i].c_str(),
                       offset + i*sizeof(T), hdf5Type );
        } else
        {
            H5Tinsert( s_HDF5Datatype, realvectComponentNames[i].c_str(),
                       offset + i*sizeof(T), hdf5Type );
        }
    }

    // When dimensionality==2, the Triple ctor sees to filling the last two
    // elements with zero.
}

/*static*/ template<typename T> void
Triple<T>::ReleaseHDF5Datatype()
{
    H5Tclose( s_HDF5Datatype );
}


/*static*/ template<typename T> Triple<T> 
Triple<T>::WackyTriple()
{
    return Triple<T>( T(9090.9), T(-8080.8), T(-7070.7) );
}


/** Arg data points to THREE ints.  We make a copy of its contents, so the
 *  caller can delete them.
 *  We use this function to construct from the hyperslab read in as dataset
 *  "boxes".
*/
/* explicit */
template<typename T>
Triple<T>::Triple( T const * data )
{
    memcpy( m_data, data, THREE * sizeof(T) );
}

template<typename T>
Triple<T>::Triple( T i, T j, T k )
{
    m_data[0] = i;
    m_data[1] = j;
    m_data[2] = k;
}

/** Construct from i component of arg iComp, j of jComp and k of kComp. */
template<typename T>
Triple<T>::Triple( Triple<T> const & iComp,
                   Triple<T> const & jComp,
                   Triple<T> const & kComp )
{
    m_data[0] = iComp.m_data[0];
    m_data[1] = jComp.m_data[1];
    m_data[2] = kComp.m_data[2];
}
    
template<typename T>
Triple<T>::Triple( Triple<T> const & that )
{
    memcpy( m_data, that.m_data, THREE * sizeof(T) );
}

template<typename T> Triple<T> &
Triple<T>::operator=( Triple<T> const & that )
{
    if( this != &that )
    {
        memcpy( m_data, that.m_data, THREE * sizeof(T) );
    }
    return *this;
}


template<typename T>
Triple<T>::operator std::vector<T>()
{
    std::vector<T> result(THREE);
    for( int i=0;i<THREE;++i ) result[i] = m_data[i];
    return result;
}
    

template<typename T> void
Triple<T>::operator-=( Triple<T> const & that )
{
    for( int i=0;i<THREE;++i )
    {
        m_data[i] -= that.m_data[i];
    }
}

template<typename T> void
Triple<T>::operator+=( Triple<T> const & that )
{
    for( int i=0;i<THREE;++i )
    {
        m_data[i] += that.m_data[i];
    }
}

template<typename T> Triple<T>
operator-( Triple<T> const & iv1, Triple<T> const & iv2 )
{
    Triple<T> result( iv1 );
    result -= iv2;
    return result;
}

template<typename T> void
Triple<T>::operator*=( T x )
{
    for( int i=0;i<THREE;++i )
    {
        m_data[i] *= x;
    }
}


template<typename T> Triple<T>
operator+( Triple<T> const & iv1, Triple<T> const & iv2 )
{
    Triple<T> result( iv1 );
    result += iv2;
    return result;
}


template<typename T> Triple<T>
operator*( Triple<T> const & iv1, T x )
{
    Triple<T> result( iv1 );
    result *= x;
    return result;
}


template<typename REAL_T> bool
Triple<REAL_T>::operator==( Triple<REAL_T> const & that ) const
{
    if( (m_data[0] == that.m_data[0])
    &&  (m_data[1] == that.m_data[1])
    &&  (m_data[2] == that.m_data[2]) )
    {
        return true;
    } else
    {
        return false;
    }
}


template<typename REAL_T> bool
Triple<REAL_T>::operator!=( Triple<REAL_T> const & that ) const
{
    return ! operator==(that);
}

//
// Explicit template instantiations
//
template struct Triple<float>;
template struct Triple<double>;
template struct Triple<int>;

template Triple<int> operator-( Triple<int> const & iv1, Triple<int> const & iv2 );
template Triple<int>  operator+( Triple<int> const & iv1, Triple<int> const & iv2 );
template Triple<int> operator*( Triple<int> const & iv1, int x );

template Triple<float> operator-( Triple<float> const & iv1, Triple<float> const & iv2 );
template Triple<float>  operator+( Triple<float> const & iv1, Triple<float> const & iv2 );
template Triple<float> operator*( Triple<float> const & iv1, float x );

template Triple<double> operator-( Triple<double> const & iv1, Triple<double> const & iv2 );
template Triple<double>  operator+( Triple<double> const & iv1, Triple<double> const & iv2 );
template Triple<double> operator*( Triple<double> const & iv1, double x );
