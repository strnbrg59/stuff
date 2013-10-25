#include "astro.hpp"
#include "cmdline.hpp"
#include "trace.hpp"
#include "utils.hpp"
#include "file_collector.hpp"
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>

using std::cerr;
using std::cout;
using std::endl;

int
main( int argc, char * argv[] )
{
    Trace t("main()");
    Cmdline cfg( argc, argv );
    if( cfg.DebugLevel() > 3 )
    {
        cfg.PrintCmdlineParams( std::cerr );
    }

    // Assemble the list of ppm[.gz] files to align.
    FileCollector file_collector( cfg.DataDir(), cfg.DataSuffix() );
    vector<string> infile_names( file_collector.GetFiles() );
    assert( infile_names.size() > 0 );
    t.Info( "infile_names.size() = %d.", infile_names.size() );

    // Model -- picture we will try to align all others to.
    if( cfg.ModelFile() != string("") )
    {
        vector<string>::iterator mi(
            find(infile_names.begin(), infile_names.end(), cfg.ModelFile() ));
        assert( mi != infile_names.end() );
        std::iter_swap( mi, infile_names.begin() );
    }
    char const * filename = infile_names[0].c_str();
    t.Info( "Loading model file: %s.", filename );
    RawData model( (cfg.DataDir() + string(filename)).c_str() );

    // Dark frame, if any, to subtract from everything else.
    RawData dark_frame(0,1,0,1);
    if( cfg.DarkFrame() != string("") )
    {
        // Make sure dark frame is not among the pictures we're going to try to
        // align.
        vector<string>::iterator mi(
            find(infile_names.begin(), infile_names.end(), cfg.DarkFrame() ));
        if( mi == infile_names.begin() )
        {
            t.FatalError( "Dark frame same as model.  Restart program, and "
                "this time specify model_file on the command line." );
        } else
        if( mi != infile_names.end() )
        {
            infile_names.erase( mi );
        }

        dark_frame.Copy( cfg.DarkFrame() );
        model -= dark_frame;
    }

    Sample model_sample( model, cfg );
    t.Info() << "Model_sample: (size=" << model_sample.Size() << ")\n"
         << model_sample << '\n';

    RawData image_sum(0,1,0,1);
    image_sum.Copy( model );

    int n_files_aligned = 1;
    for( unsigned int iFile=1; iFile<infile_names.size(); ++iFile )
    {
        filename = infile_names[iFile].c_str();
        t.Info( "Loading alignee: %s.", filename );
        RawData alignee( (cfg.DataDir() + string(filename)).c_str());
        if( cfg.DarkFrame() != string("") )
        {
            if( !alignee.SameSize(dark_frame) )
            {
                t.FatalError("dark_frame feature can be used only if all\n"
                       "frames (including the dark frame) are the same size.");
            }
            alignee -= dark_frame;
        }

        Sample alignee_sample( alignee, cfg );
        t.Info() << "Alignee sample: (size=" << alignee_sample.Size() << ")\n"
             << alignee_sample << '\n';

        StretchParams transforms;

        if( 0 == alignee_sample.AlignWith( &transforms, model_sample ) )
        {
            t.Warning( "Could not align %s, moving to the next file...",
                       filename );
            if( cfg.QuickFinish() ) // Always false, except some debugging
            {
                return 0;
            }
        } else
        {
            n_files_aligned ++;
            t.Info( "Transforming Rawsample by..." );
            t.Info() << transforms << '\n';
            alignee.Transform( transforms );
            t.Info( "Done transforming Rawsample..." );
            image_sum += alignee;
        }
    }

    if( cfg.QuickFinish() ) // Always false, except some debugging
    {
        return 0;
    }

    t.Info( "Succeeded in registering %d of %d files against the model file.",
            n_files_aligned-1, infile_names.size()-1 );
    if( n_files_aligned > 1 )
    {
        // Rescale and save the image_sum to a file.  It's going to be a ppm
        // file but we give it a suffix "ppm.bak" so if we rerun the program it
        // doesn't accidentally pick up that file (if .ppm is the suffix the
        // program is looking for).
        image_sum.Rescale( trim, n_files_aligned );
        char buf[100];
        sprintf(buf, "trim_scaling_%d.ppm.bak", n_files_aligned );
        std::ofstream grandtotal( buf );
        t.Info( "Writing out trim_scaling..." );
        grandtotal << image_sum;
    }

    return 0;
}

