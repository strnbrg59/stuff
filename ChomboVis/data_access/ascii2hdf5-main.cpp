// Putting these up here to avoid using a template specialization before its
// definition.
#include "Intvect.h"
#include "Box.h"
#include "FAB.h"
#include "BoxLayout.h"
#include "BoxLayoutData.h"
#include "Ascii2HDF5.h"
#include "VisualizableDataset.h"
#include "../utils/cmdline.h"
#include <hdf5.h>
#include <iterator>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

using std::stringstream;
using std::ofstream;
using std::cerr;

void doit();
void readstdin();

/****************************************************************************
 * Converts an ascii format to ChomboVis-legal hdf5.  Does reverse conversion
 * if Cmdline::ascii2hdf5_inverse == true.
 ****************************************************************************
*/

int main( int argc, char * argv[] )
{
    Trace t("ascii2hdf5 main()");

    stringstream ost;
    ost
     << "*******************************************************************\n"
     << "Usage: cat foo.dat | ascii2hdf5.sh outfile=foo.hdf5                \n"
     << "See doc/Other_features.html for more on ascii2hdf5.                \n"
     << "*******************************************************************\n";

    Cmdline cmdline( argc, argv );

    if( cmdline.Ascii2hdf5Inverse() == 0 )
    {
        if( cmdline.Infile() == "" )
        {
            t.Info() << "Reading from stdin...\n";
            Ascii2HDF5::Convert( cmdline.Outfile(), std::cin,
                                 cmdline.Ascii2hdf5RawMode() );
        }
        else
        {
            t.Info() << "Reading from " << cmdline.Infile() << '\n';
            std::ifstream infile( cmdline.Infile().c_str() );
            Ascii2HDF5::Convert( cmdline.Outfile(), infile,
                                 cmdline.Ascii2hdf5RawMode() );
        }
        t.Info() << "hdf5 output is now ready in " << cmdline.Outfile() << '\n';
    } else
    // Inverse operation: hdf5 to ascii.  This part isn't documented, but it's
    // useful for the unit test -- ../ascii2hdf5_test.sh.
    {
        ofstream outfile( cmdline.Outfile().c_str() );
        int dimensionality, precision;
        ChomboHDF5FileType filetype;
        ChomboHDF5DiscoverMetaparameters(
            cmdline.Infile().c_str(), &dimensionality, &precision, &filetype );

        boost::shared_ptr<VisualizableDataset<float> > visdatF;
        boost::shared_ptr<VisualizableDataset<double> > visdatD;
        if( precision == 1 )
        {
            visdatF.reset( new VisualizableDataset<float>( cmdline.Infile() ));
            operator<< <float,ostream>(outfile, (*visdatF));
        } else
        {
            visdatD.reset( new VisualizableDataset<double>( cmdline.Infile() ));
            operator<< <double,ostream>(outfile, (*visdatD));
        }

        cerr << "ascii output is now ready in " << cmdline.Outfile() << '\n';
    }

    return 0;
}
