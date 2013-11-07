#ifndef INCLUDED_CHOMBOHDF5_SUBCLASSES_H
#define INCLUDED_CHOMBOHDF5_SUBCLASSES_H

#include "ChomboHDF5.h"

template <typename T> class OldChomboHDF5 : public ChomboHDF5<T>
{
  public:
    OldChomboHDF5( char const * infileName, Status * status );

  private:
    virtual void GatherComponentNamesAndCenterings( HeteroMap<string> const & );
    virtual void ScanForGhostStatus( HeteroMap<string> * );
};

template <typename T> class EBChomboHDF5 : public ChomboHDF5<T>
{
  public:
    EBChomboHDF5( char const * infileName, Status * status );

  private:
    virtual void GatherComponentNamesAndCenterings( HeteroMap<string> const & );
    virtual void ScanForGhostStatus( HeteroMap<string> * );
};

#endif // INCLUDED_CHOMBOHDF5_SUBCLASSES_H
