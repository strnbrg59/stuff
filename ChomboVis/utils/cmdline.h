
    #ifndef INCLUDED_CMDLINE_HPP
    #define INCLUDED_CMDLINE_HPP
    #include "cmdline_base.h"
    /*******************************************************************
     * This is a generated file.  Do not hand-edit it.
    *******************************************************************/
     
    class Cmdline : public CmdlineBase
    { 
      public: 
         
        Cmdline( int argc, char *argv[] );
        virtual ~Cmdline() {}
    
        string Infile() const { return m_infile; }
        void Infile( string x ) {m_infile = x; }

        string Outfile() const { return m_outfile; }
        void Outfile( string x ) {m_outfile = x; }

        bool Ascii2hdf5Inverse() const { return m_ascii2hdf5_inverse; }
        void Ascii2hdf5Inverse( bool x ) {m_ascii2hdf5_inverse = x; }

        bool Ascii2hdf5RawMode() const { return m_ascii2hdf5_raw_mode; }
        void Ascii2hdf5RawMode( bool x ) {m_ascii2hdf5_raw_mode = x; }

        bool MemoryLeakTest() const { return m_memory_leak_test; }
        void MemoryLeakTest( bool x ) {m_memory_leak_test = x; }

      private:
        string m_infile; // "impt that default be empty string!"
        string m_outfile; // "output"
        bool m_ascii2hdf5_inverse; // "for hdf52ascii"
        bool m_ascii2hdf5_raw_mode; // "field data comes in raw -- for use by VisDat::SaveToHDF5()"
        bool m_memory_leak_test; // "around & around"

    }; 
    #endif // INCLUDED_CMDLINE_HPP    
    
