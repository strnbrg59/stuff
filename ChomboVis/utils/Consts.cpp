#include "Consts.h"

char const * const Consts::BoxCorners     = "BoxCorners";
char const * const Consts::Chombo_global  = "Chombo_global";
char const * const Consts::SpaceDim       = "SpaceDim";
char const * const Consts::ascii2hdf5     = "ascii2hdf5";
char const * const Consts::chombovis_home = "CHOMBOVIS_HOME";
char const * const Consts::component_     = "component_";
char const * const Consts::component_names = "component_names";
char const * const Consts::comps          = "comps";
char const * const Consts::data           = "data";
char const * const Consts::data_attributes= "data_attributes";
char const * const Consts::data_datatype0 = "data:datatype=0";
char const * const Consts::data_centering = "data_centering";
char const * const Consts::intvecti       = "intvecti";
char const * const Consts::intvectj       = "intvectj";
char const * const Consts::intvectk       = "intvectk";
char const * const Consts::iteration      = "iteration";
char const * const Consts::name           = "name";
char const * const Consts::num_components = "num_components";
char const * const Consts::num_particles  = "num_particles";
char const * const Consts::objectType     = "objectType";
char const * const Consts::origin         = "origin";
char const * const Consts::particles      = "particles";
char const * const Consts::position_x     = "position_x";
char const * const Consts::position_y     = "position_y";
char const * const Consts::position_z     = "position_z";
char const * const Consts::precision      = "precision";
char const * const Consts::realvectx      = "x";
char const * const Consts::realvecty      = "y";
char const * const Consts::realvectz      = "z";
char const * const Consts::ref_ratio      = "ref_ratio";
char const * const Consts::space_dim      = "space_dim";
char const * const Consts::testReal       = "testReal";
char const * const Consts::time           = "time";
char const * const Consts::unlikely_name  = "d&$FF012faQZe**$#|<?I#@123$F";
char const * const Consts::values         = "values";

//
// Constants meaningful for only one hdf5 format.
//
char const * const EBConsts::CenteredComponents = "CenteredComponents";
char const * const EBConsts::centering_prefixes[8] =
    { "Cell",  // See DataCenteringInt2Intvect() in ChomboHDF5.cpp.
      "XFace",
      "YFace",
      "ZFace",
      "XEdge",
      "YEdge",
      "ZEdge",
      "Node" };
char const * const EBConsts::short_centering_prefixes[8] =
    { "C",
      "X",
      "Y",
      "Z",
      "XE",  // Edge centerings are not yet supported in Chombo.
      "YE",
      "ZE",
      "N" };
