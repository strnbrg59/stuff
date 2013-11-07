/**
    Notice this class, ChomboBridge, lives in the data_access directory, but
    we have a whole directory called chombo_bridge.  Hence a few words about
    the design.

    Directory chombo_bridge builds a dynamically loadable module (actually,
    four versions, one for each combination of [float,double] x [2d,3d])
    that's linked to Chombo (.so) libraries and therefore can do Chombo things.

    Source files in chombo_bridge can #include anything in Chombo, but should
    never #include anything in ChomboVis.  In contrast, nothing in the
    data_access directory should know anything about Chombo.  The closest that
    anything in data_access comes to knowing about Chombo is this class,
    ChomboBridge, which loads the appropriate module from the chombo_bridge
    directory and calls its functions.

    All functions in chombo_bridge (that are meant to be exported) have the same
    signature: they return void and take one argument of pointer type.  The
    common signature is so we can dispatch on the module with a minimum of
    complexity.  What the pointer points to, however, is part of the interface
    between the module and class ChomboBridge.
*/

class LtdlModule;

class ChomboBridge
{
  public:
    ChomboBridge( char const * hdf5filename );
    ChomboBridge( int dim, int precision );
    ~ChomboBridge();

    int IntVectTest( int );
    int amrtools_ReadAMRHierarchyHDF5( char const * hdf5filename );
  private:
    LtdlModule * m_module;
    int m_dim;
    int m_precision;
    static int s_count;

    // Deliberately unimplemented:
    ChomboBridge( ChomboBridge const & );
    ChomboBridge & operator=( ChomboBridge const & );
};
