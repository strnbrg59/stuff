#ifndef _INCLUDED_RCM_TEMPLATE_FUNCTIONS_H_
#define _INCLUDED_RCM_TEMPLATE_FUNCTIONS_H_

template<class T> ostream& operator<<( ostream&, const M<T>& );

template<class T> M<T> transp( const M<T>& );
template<class T> V<T> transp( const V<T>& );
template<class T> V<T> diag( const M<T>& );

template<class T,class U> M<U> apply( U (*func)(T), const M<T>& );
template<class T,class U> M<U> apply( U (*func)(T,T), const M<T>&, const M<T>& );

template<class T> M<T> innerProduct( T (*func)(T,T), T (*func)(T,T), const M<T>&, const M<T>& );

template<class T> M<T> laminate( T (*func)(T,T), const M<T>& );
template<class T> V<T> scan( T (*func)(T,T), const M<T>& );

/** We need all these variants because implicit conversions don't seem
 *  to take place between template classes.
*/
template<class T> M<T> catcols( const M<T>&, const M<T>& );
template<class T> M<T> catcols( const V<T>&, const M<T>& );
template<class T> M<T> catcols( const M<T>&, const V<T>& );
template<class T> M<T> catcols( const V<T>&, const V<T>& );
template<class T> M<T> catrows( const M<T>&, const M<T>& );
template<class T> M<T> catrows( const V<T>&, const M<T>& );
template<class T> M<T> catrows( const M<T>&, const V<T>& );
template<class T> M<T> catrows( const V<T>&, const V<T>& );

template<class T> M<T> ident( int rows, int cols );
template<class T> M<T> drho( int rows, int cols, const M<T>& x );
template<class T> V<T> ravel( const M<T>& x );

template<class T> M<T> take( const M<T>&, const V<int>& );
template<class FROM, class TO> M<TO> convertM( const M<FROM>& mat );
template<class FROM, class TO> V<TO> convertV( const V<FROM>& vect );

#include "rcm-template-functions.hImpl"

#endif
