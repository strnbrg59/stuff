/** Generates the "say it" sequence.
 *  Usage: "a.out n [init=1]"
 *  where n is the number of sequence elements desired, and init is the
 *  initial value of the sequence (which defaults to '1').
*/


#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


/** Arg digits is, for example, "111221".  Break it up into "111","22","1".
*/
string Next( string current )
{
    assert( current.size() > 0 );

    unsigned start=0;
    unsigned stop = 0;
    string result;
    result.reserve( current.size()*2 );
    unsigned largest_number = 0;
    char buf[2] = {0,0};

    while( stop < current.size() )
    {
        while( (stop < current.size())
        &&     (current[stop] == current[start]) )
        {
            ++stop;
        }

        ostringstream ost;
        ost << (stop - start) << current[start];
        result += ost.str();

        // Find largest number in the sequence.
        if( ((stop-start) > largest_number) )
        {
            largest_number = stop-start;
        }
        buf[0] = current[start];
        if( unsigned(atoi(buf)) > largest_number )
        {
            largest_number = atoi(buf);
        }

        start = stop;
    }

    // cerr << "(largest number = " << largest_number << ") ";
    return result;
}

int
main( int argc, char * argv[] )
{
    int n;
    string initial_element = "1";

    n = atoi( argv[1] );
    if( argc == 3 )
    {
        initial_element = argv[2];
    }

    cout << initial_element << endl;
    string curr_element = initial_element;
    for( int i=0; i<n; ++i )
    {
        curr_element = Next( curr_element );
//      cout << curr_element.size() << endl;
        cout << curr_element << endl;
    }

    return 0;
}







