
   #include "cmdline_base.h"
   #include "cmdline.h"
   
   /*******************************************************************
    * This is a generated file.  Do not hand-edit it.
    *******************************************************************/
   
   Cmdline::Cmdline( int argc, char *argv[] )
   { 
   
        m_argmap["infile"] = new SetString(&m_infile, "impt that default be empty string!");
        m_infile="";

        m_argmap["outfile"] = new SetString(&m_outfile, "output");
        m_outfile="ascii2hdf5-output.hdf5";

        m_argmap["ascii2hdf5_inverse"] = new SetBool(&m_ascii2hdf5_inverse, "for hdf52ascii");
        m_ascii2hdf5_inverse=false;

        m_argmap["ascii2hdf5_raw_mode"] = new SetBool(&m_ascii2hdf5_raw_mode, "field data comes in raw -- for use by VisDat::SaveToHDF5()");
        m_ascii2hdf5_raw_mode=false;

        m_argmap["memory_leak_test"] = new SetBool(&m_memory_leak_test, "around & around");
        m_memory_leak_test=false;


        CheckForCmdlineFile( string(
             string(getenv("HOME")) + string("/") + string(
                    ".pychombovisrc")));
        SaveCmdlineFileTimestamp();
        LoadDefaultsFromFile();
        ParseCommandLine( argc, argv );
    }
    
