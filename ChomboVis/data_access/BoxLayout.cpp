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

#include "../utils/Trace.h"
#include "BoxLayout.h"
#include <cassert>
#include <vector>
#include <list>
using std::vector;
using std::list;


template<class REAL_T>
BoxLayout<REAL_T>::BoxLayout()
{
}


template<class REAL_T>
BoxLayout<REAL_T>::BoxLayout( BoxLayout<REAL_T> const & that )
  : m_rep( that.m_rep ),
    m_dx( that.m_dx ),
    m_origin( that.m_origin )
{
}


template<class REAL_T> BoxLayout<REAL_T> &
BoxLayout<REAL_T>::operator=( BoxLayout<REAL_T> const & that )
{
    if( this != &that )
    {
        this->m_rep = that.m_rep;
        this->m_dx = that.m_dx;
        this->m_origin = that.m_origin;
    }
    return *this;
}


template<class REAL_T>
BoxLayout<REAL_T>::BoxLayout( vector<Box> vb, 
                              Triple<REAL_T> dx, Triple<REAL_T> origin )
  : m_rep( vb ),
    m_dx( dx ),
    m_origin( origin )
{
    // Assign consecutive box ID numbers.
    for( unsigned i=0;i<vb.size();++i )
    {
        m_rep[i].SetIdNum(i);
    }
}


template<class REAL_T> void
BoxLayout<REAL_T>::PrintBoxIDs( std::ostream & ost ) const
{
    Trace t("BoxLayout::PrintBoxIDs()");
    ost << "BoxLayout box IDs: ";
    for( typename std::vector<Box>::const_iterator i=m_rep.begin();
         i!=m_rep.end();
         ++i )
    {
        ost << i->GetIdNum() << " ";
    }
    ost << std::endl;
}
        


template<class REAL_T> unsigned int
BoxLayout<REAL_T>::size() const
{
    return m_rep.size();
}


template<class REAL_T> Box
BoxLayout<REAL_T>::operator[]( int i ) const
{
    return m_rep[i];
}

template<class REAL_T> Box &
BoxLayout<REAL_T>::operator[]( int i )
{
    return m_rep[i];
}


template<class REAL_T> vector<Box>::iterator
BoxLayout<REAL_T>::begin()
{
    return m_rep.begin();
}


template<class REAL_T> vector<Box>::iterator
BoxLayout<REAL_T>::end()
{
    return m_rep.end();
}


template<class REAL_T> void
BoxLayout<REAL_T>::push_back( Box const & b )
{
    m_rep.push_back( b );
}


template<class REAL_T> void
BoxLayout<REAL_T>::assign( list<Box>::iterator iBegin,
                           list<Box>::iterator iEnd,
                           Triple<REAL_T> dx, Triple<REAL_T> origin )
{
    m_rep.assign( iBegin, iEnd );
    m_dx = dx;
    m_origin = origin;
}


/** 2 or 3 */
template<class REAL_T> int
BoxLayout<REAL_T>::GetDimensionality() const
{
    assert( ! m_rep.empty() );
    Box box( m_rep[0] );
    if( box.GetLoCorner(2) == box.GetHiCorner(2) )
    {
        return 2;
    } else
    {
        return 3;
    }
}


/*  Nice for printing, also, used from Python (via BoxLayoutData).
 *  Dimensionality assumed to be 2, if locorner[k]==hicorner[k].
 *  The nested vectors represent...
 *  ...                layout     lo&hi corners    corner (i,j,[k])      */
template<class REAL_T> vector<    vector<          vector<int> > >
BoxLayout<REAL_T>::GetAsNestedVectors() const
{
    vector<vector<vector<int> > > result;
    result.reserve( m_rep.size() );
    for( unsigned b=0;b<m_rep.size();++b )
    {
        Box const & box( m_rep[b] );
        vector<int> loCorner( box.GetLoCorner() );
        vector<int> hiCorner( box.GetHiCorner() );
        if( GetDimensionality() == 2 )
        {
            loCorner.pop_back();
            hiCorner.pop_back();
        }
        vector<vector<int> > corners(2);
        corners[0] = loCorner;
        corners[1] = hiCorner;
        result.push_back( corners );
    }
    return result;
}


//
// Explicit template instantiations
//
template class BoxLayout<float>;
template class BoxLayout<double>;
