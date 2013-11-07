#ifndef INCLUDED_VECTORFUNCTOR_H
#define INCLUDED_VECTORFUNCTOR_H

#include <boost/shared_array.hpp>

/** Abstract base class used as a trick to pass VectorFunctor pointers around
 *  in nontemplatized code.
 *  Usually can't be used without a downcast to one of the templatized flavors of
 *  VectorFunctor.
*/
class VectorFunctorBase
{
  public:
    virtual ~VectorFunctorBase() { }
};


/* Abstract class.  operator()() maps a vector<REAL_T const *> to a REAL_T*,
 * using a transformation that has to be defined on a subclass.
 * Example: see NewComponentGenerator in ../data_access/python/visdatmodule.cpp.
*/
template<class REAL_T> class VectorFunctor : public VectorFunctorBase
{
  public:
    VectorFunctor() { }
    virtual ~VectorFunctor() { }
    virtual boost::shared_array<REAL_T>
        operator()( vector<boost::shared_array<REAL_T> > args, int length )
            const = 0;
  private:
    VectorFunctor( VectorFunctor const & );
    VectorFunctor & operator=( VectorFunctor const & );
};

#endif // INCLUDED_VECTORFUNCTOR_H
