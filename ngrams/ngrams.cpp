/*
 * Random language generator.
 * By Theodore Sternberg strnbrg59@gmail.com
 * This is free software.  You can do whatever you like with it, as long
 * as you mention my name.
 */

#include <algorithm>
#include <cassert>
#include <unistd.h>  // getopt()
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <time.h>
#include <map>
#include <string>
#include <vector>
using std::vector;
using std::string;
using std::map;
using std::pair;
using std::cerr;
using std::cout;
using std::endl;

#include "timer.hpp"

typedef map< char, double > NthGramMap; // prob(n-th gram | n-1 preceding grams)
typedef map< string, NthGramMap > ProbData; // All the prob dsns.

void Counts2Probs( ProbData* data );
void DisplayData( ProbData const & data );
void Generate( ProbData const & data, int output_length, int seed );
void ProcessCmdLine( int argc,
                     char * argv[],
                     vector<FILE *> * infiles,
                     int * ngram_size,
                     int * output_length,
                     int * rand_seed );
void ProcessQueryString( int argc,
                         char * argv[],
                         vector<FILE *> * infiles,
                         int * ngram_size,
                         int * output_length,
                         int * rand_seed );
void Train( ProbData *,
            int ngram_size,
            FILE * infile,
            int num_chars_to_read );
void Usage( char* argv[] );


void Usage( char* argv[] )
{
    cerr << "Usage: " << argv[0] << " "
         << "-f training_file1 [-f training_file2 [...]] "
         << "-n ngram_size "
         << "-N output_length "
         << "[-s rand_seed] "
         << endl;
}

void ProcessCmdLine( int argc,
                     char * argv[],
                     vector<FILE *> * infiles,
                     int * ngram_size,
                     int * output_length,
                     int * rand_seed )
{
    int c;
    bool f_is_set, n_is_set, N_is_set;
    f_is_set = n_is_set = N_is_set = false;
    while ( ( c = getopt( argc, argv, "f:n:N:s:" ) )  != EOF )
    {
        switch ( c )
        {
            case 'f' : // training file
            {
                FILE * infile = fopen( optarg, "r" );
                if ( ! infile )
                {
                    cerr << "Can't open infile " << optarg << endl;
                    exit(1);
                }
                infiles->push_back( infile );
                f_is_set = true;
                break;
            }

            case 'n' : // ngram size
                *ngram_size = atoi( optarg );
                n_is_set = true;
                break;

            case 'N' : // output length
                *output_length = atoi( optarg );
                N_is_set = true;
                break;

            case 's' : // seed for random number generation
                *rand_seed = atoi( optarg );
                break;

            case '?' :
            {
                Usage( argv );
                exit(1);
            }
        }
    }

    if( ! (f_is_set && n_is_set && N_is_set ) )
    {
        Usage( argv );
        exit(2);
    }

    if( optind == 1 ) // no cmd-line args
    {
        Usage( argv );
        exit(1);
    }
}

void ProcessQueryString( int argc,
                         char * argv[],
                         vector<FILE *> * infiles,
                         int * ngram_size,
                         int * output_length,
                         int * rand_seed )
{
    string query_string = getenv("QUERY_STRING");
    //cout << "QUERY_STRING = " << query_string << endl;
    char* qs = new char[ query_string.size() + 1 ];
    strcpy( qs, query_string.c_str() );

    // Grab the kv pairs between the & delimitters.
    vector<string> option_pairs;
    char* tok = strtok( qs, "&\0\n" );
    while( tok )
    {
        option_pairs.push_back( tok );
        tok = strtok( NULL, "&\0\n" );
    }

    // Break up the kv pairs.
    map<string,string> options;
    for( vector<string>::const_iterator i = option_pairs.begin();
         i != option_pairs.end();
         ++ i )
    {
        strcpy( qs, i->c_str() );
        tok = strtok( qs, "=" );
        char* val = strtok( NULL, "\0\n" );
        options[tok] = val;
    }

    string infilenameprefix = "/var/www/ngrams/";

    // Go through the QUERY_STRING items, looking for keys that begin with
    // "training_" and whose values are "on".
    for( map<string,string>::const_iterator i = options.begin();
         i != options.end();
         ++ i )
    {
        if( i->first.substr(0,strlen("training_")) == "training_" )
        {
            string infilenamebase = (i->first.c_str()) + strlen("training_");
            string infilename = 
              infilenameprefix + infilenamebase;

            FILE * infile = fopen( infilename.c_str(), "r" );
            if ( ! infile )
            {
                cout << "Can't open " << infilename << '\n';
                exit(1);
            }
           infiles->push_back( infile );
        }
    }
    if( infiles->size() == 0 )
    {
        cout << "Error: you must select at least one training text!\n";
        exit(0);
    }

    *ngram_size = atoi( options["depth"].c_str() );
    *output_length = 4000/(*ngram_size);

    delete [] qs;
}

int main( int argc, char* argv[] )
{
    int ngram_size; // order of markov model -- the 'n' in n-gram.
    int output_length; // how much text to generate
    int rand_seed=0;  // arg to srand() in Generate().

    // The number of chars to read from each training file.  If file is
    // longer than this, stop at this point.  If file is shorter, rewind
    // and read it again.  The goal is to give an equal weight to every
    // training file indicated, without regard to the file's actual length.
    int const num_chars_to_read = 40000;

    vector<FILE *> infiles;

    if( getenv("QUERY_STRING") )
    {
        ProcessQueryString( argc, argv,
                            &infiles, &ngram_size, &output_length, &rand_seed );
    } else
    {
        ProcessCmdLine( argc, argv,
                        &infiles, &ngram_size, &output_length, &rand_seed );
    }

    //
    // Training
    //
    ProbData data;
    Timer timer;
    timer.SetSilent(true);
    timer.Start();
    for( vector<FILE *>::iterator i = infiles.begin();
         i != infiles.end();
         ++ i )
    {
        Train( &data, ngram_size, *i, num_chars_to_read/infiles.size() );
    }
    timer.Stop("Train()");

    timer.Start();
    Counts2Probs( &data );
    timer.Stop("Counts2Probs()");

//  DisplayData( data );
    timer.Start();
    Generate( data, output_length, rand_seed );
    timer.Stop("Generate()");

    //
    // Cleanup
    //
    for_each( infiles.begin(), infiles.end(), std::ptr_fun( fclose ) );
}


void Train( ProbData * data,
            int ngram_size,
            FILE * infile,
            int num_chars_to_read )
{
    
    char ngram[ ngram_size ];
    int ngram_index=0;
    int chars_read=0;

    for(;;) // exit when reach end of file
    {        
        //
        // Accumulate an n-gram
        //
        while( ngram_index < ngram_size )
        {
            //int gram = tolower(fgetc( infile ));
            int gram = fgetc( infile );

            chars_read ++;
            if( chars_read > num_chars_to_read )
            {
                return;
            }

            if( feof(infile) )
            {
                rewind(infile);
            }

            bool gram_is_usable = true;
            if( gram=='\n' || gram=='\r' )
            {
                gram_is_usable = false;
            }

            if ( gram_is_usable )
            {
                ngram[ ngram_index ] = gram;
                ngram_index ++;
            }
        }

        //
        // Store this n-gram
        //

        // Find its element in data, if any.  Create if none.
        // In that element of data, accumulate the count for the nth gram.
        // We'll convert counts to probabilities later.
        string all_but_last_gram = string(ngram).substr(0,ngram_size-1);
        int gram = ngram[ ngram_size - 1 ];
        ProbData::iterator i = data->find( all_but_last_gram );
        if( i == data->end() )
        {  // insert a dummy
            NthGramMap dummy;
            (*data)[ all_but_last_gram ] = dummy;
            i = data->find( all_but_last_gram );            
        }
        NthGramMap* nth_gram_map = &(i->second);
        if ( nth_gram_map->find( gram ) == nth_gram_map->end() )
        {
            (*nth_gram_map)[ gram ] = 1.0;
        } else
        {
            (*nth_gram_map)[ gram ] += 1.0;
        }

        //
        // Shift the n-gram and go accumulate a new one.
        //
        for ( int i=0;i<ngram_size-1;i++ )
        {
            ngram[i] = ngram[i+1];
        }
        ngram_index --;
    }
}

/** Convert counts (ProbData.second.second) to probabilities. */
void Counts2Probs( ProbData* data )
{
    for( ProbData::iterator i = data->begin();
         i != data->end();
         ++ i )
    {
        double total = 0;

        for( NthGramMap::iterator j = i->second.begin();
             j != i->second.end();
             ++ j )
        {
            total += j->second;
        }        

        double cum_total = 0;

        for( NthGramMap::iterator j = i->second.begin();
             j != i->second.end();
             ++ j )
        {
            cum_total += j->second;
            j->second = cum_total / total;
        }        
        
    }
}

/** For debugging */
void DisplayData( ProbData const & data )
{
    for( ProbData::const_iterator i = data.begin();
         i != data.end();
         ++ i )
    {
        cout << "key = " << i->first << endl;
        for( NthGramMap::const_iterator j = i->second.begin();
             j != i->second.end();
             ++ j )
        {
            cout << i->first << " :: " 
                 << j->first << " : " << j->second
                 << endl;
        }
        cout << "=======" << endl;
    }
}


void Generate( ProbData const & data, int output_length, int rand_seed )
{
    cout << "Content-type: text/html; charset=utf-8\n\n\n";
    cout << "<HTML><BODY BGCOLOR=\"AAAAAA\">";
    //DisplayData( data );
    if( rand_seed != 0 )
    {
        srand( rand_seed );
    } else
    {
        time_t t;
        srand( time(&t) );
    }
    const int max_line_length = 70;
    int line_length = 0;

    int ngram_length = data.begin()->first.size() + 1;

    // Pick a random initial n-1-gram.
    unsigned data_size( data.size() );
    double size_frac( (rand()+0.0)/RAND_MAX );
    ProbData::const_iterator data_iter = data.begin();
    for( int k=0; k<int(data_size*size_frac); ++k )
    {
        ++data_iter;
    }
    cout << data_iter->first;

    // Generate as much text as specified in the -N option on
    // the command line.
    char gram_2='x';
    char gram_1='x';
    for( int ngrams_printed = 0;
             ngrams_printed < output_length;
             ++ ngrams_printed )
    {
        // pick a random nth gram.
        double u = (rand()+0.0)/RAND_MAX;
        NthGramMap::const_iterator j = data_iter->second.begin();
        while( j->second < u )  // Use sorted vector and go straight to element.
        {
            ++j;
            assert( j != data_iter->second.end() );
        }

        // print it (unless there've already been two spaces in a row).
        if( gram_1!=' ' || gram_2!=' ' || j->first!=' ' )
        {
            gram_2 = gram_1;
            gram_1 = j->first;
            cout << j->first;
            line_length ++;
        }
        if( line_length > max_line_length && j->first == ' ' )
        {
            cout << endl;
            line_length = 0;
        }
         
        // find the next n-1-gram
        string last_ngram( data_iter->first + j->first );
        string last_ngram_substr( last_ngram.substr(1,ngram_length-1) );
        data_iter = data.find( last_ngram_substr );
    }

    cout << "</HTML>" << endl;
}
