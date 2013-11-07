#ifndef INCLUDED_VISUALIZABLEDATASETINTERFACEFORPYTHON_H
#define INCLUDED_VISUALIZABLEDATASETINTERFACEFORPYTHON_H

#include <string>
#include <vector>
#include <map>
#include "../Box.h"
#include "../Intvect.h"
#include "BoxLayoutDataInterfaceForPython.h"
#include "boost/shared_ptr.hpp"
#include "../../utils/StatusCodes.h"
class VectorFunctorBase;

/** Abstract base class for template class VisualizableDataset.
 *  Serves two purposes:
 *    1. Lets us call the visdat pointer polymorphically, in 
 *       python/visdatmodule.cpp.
 *    2. Defines, explicitly, the precise subset of VisualizableDataset
 *       functionality that we want to expose at the Python layer (and that is
 *       a fairly restricted subset).
*/
class VisualizableDatasetInterfaceForPython
{
  public:
    virtual boost::shared_ptr<VisualizableDatasetInterfaceForPython>
        Slice( char axis, double axisPos ) const = 0;

    virtual boost::shared_ptr<VisualizableDatasetInterfaceForPython>
        CropToGeneralBox(
            Box const &,
            std::vector<int> const * croppingLevels=0,
            std::vector<string> const * croppedComponents=0 ) const = 0;
    virtual 
        Status SaveToHDF5( std::string outfilename, bool ascii=false ) const =0;
    virtual int 
        DefineNewComponent(
            std::string name, VectorFunctorBase * callable,
            std::vector<std::string> const & argNames ) = 0;
    virtual int
        DefineNewComponent(
            std::string name, VectorFunctorBase * callable,
            std::vector<int> const & argNames ) = 0;
    virtual int
        RedefineNewComponent(
            std::string name, VectorFunctorBase * callable,
            std::vector<std::string> const & argNames ) = 0;
    virtual int
        RedefineNewComponent(
            std::string name, VectorFunctorBase * callable,
            std::vector<int> const & argNames ) = 0;

    virtual int
        SetDebugLevel( int d ) const = 0;

    virtual int
        GetDimensionality() const = 0;
    virtual std::vector<string>
        GetComponentNames() const = 0;
    virtual int
        GetPrecision() const = 0;
    virtual int
        GetNumLevels() const = 0;
    virtual Intvect const &
        GetOutputGhost() const = 0;
    virtual Intvect const &
        GetDataCentering() const = 0;
    virtual std::vector<double>
        GetOriginAsVector() const = 0;
    virtual std::vector<double>
        GetDxAsVector( int level ) const = 0;
    virtual double
        GetDt( int level ) const = 0;
    virtual double
        GetTime( int level ) const = 0;
    virtual Box const &
        GetProbDomain( int level ) const = 0;
    virtual bool
        FabIsEmpty( int level, int boxNum, int component, bool padded )
            const = 0;
    virtual bool
        RealFabIsEmpty( int level, int boxNum, int component, bool padded )
            const = 0;

    virtual std::vector<std::vector<vector<int> > >
        GetBoxLayoutAsNestedVectors( int level, bool real=true ) const=0;
    virtual boost::shared_ptr< BoxLayoutDataInterfaceForPython >
        GetBoxLayoutData( int level, int component,
                          bool real=true, bool contrapad=false ) const = 0;
    virtual std::vector< std::pair<double,double> >
        GetLinePlot( Triple<double> const & p0, Triple<double> const & p1,
                     int n, int component, int finestLevel ) const = 0;

  protected:
    virtual ~VisualizableDatasetInterfaceForPython() { }
    VisualizableDatasetInterfaceForPython() { };

  private:
    VisualizableDatasetInterfaceForPython(
        VisualizableDatasetInterfaceForPython const & );
    VisualizableDatasetInterfaceForPython & operator=(
        VisualizableDatasetInterfaceForPython & );
};

#endif // INCLUDED_VISUALIZABLEDATASETINTERFACEFORPYTHON_H
