#include <string>
using std::string;
#include <iostream>
using std::cin; using std::cout; using std::cerr; using std::endl;

void StrTrim( string & str )
{
    unsigned pos;
    pos = str.find_first_not_of( " \t\r\n" );
    str.erase( 0, pos );

    pos = str.find_last_not_of( " \t\r\n" );
    str.erase( pos+1 );
}

void StrNormalize( string & str )
{
    // capitalize.
    std::transform( str.begin(), str.end(), str.begin(), toupper );

    // remove non-letters-not-blanks.
    static char lettersAndBlank[] = {'A','B','C','D','E','F','G','H','I','J',
                                     'K','L','M','N','O','P','Q','R','S','T',
                                     'U','V','W','X','Y','Z',' '};
    unsigned pos = str.find_first_not_of( lettersAndBlank );
    while( pos != str.npos )
    {
        cerr << "Removing nonletter-nonblank at pos" << pos << endl;
        str.erase( pos,1 );
        pos = str.find_first_not_of( lettersAndBlank );
    }

    // Remove multiple blanks
    pos = str.find("  ");
    while( pos != str.npos )
    {
        cerr << "Removing multiple blank at pos" << pos << endl;
        str.erase( pos,1 );
        pos = str.find("  ");
    }

    // trim leading and trailing white space.
    StrTrim( str );
}

int main()
{
    string words[6];
    words[0] = " AE1 B AH0 K AH0 S";
    words[1] = "fone    ";
    words[2] = "  ftwo";
    words[3] = "@zerob";
    words[4] = "@ oneb    ";
    words[5] = " @ twob  ";
/*
    cerr << words[0] << endl;
    words[0].erase(3,1);
    cerr << words[0] << endl;
    exit(0);
*/

    for( int i=0;i<6;++i )
    {
        cerr << "|" << words[i] << "|" << " ";
        StrNormalize(words[i]);
        cerr << "|" << words[i] << "|" << endl;
    }

    return 0;
}
