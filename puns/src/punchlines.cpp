#include "punchlines.hpp"
#include "punutils.hpp"
#include "trace.hpp"
#include "cmdline.hpp"
#include <cassert>
#include <cstring>

Punchlines::Punchlines( char const * infilename, Phonics const & phonics )
  : m_phonics( phonics )
{
    Trace t("LoadPunchlines()");
    Cmdline const& cmdline(CmdlineFactory::TheCmdline());

    FILE * punchlineFile = fopen( infilename, "r" );
    assert( punchlineFile );

    m_punchlines.reserve(100000);

    unsigned int const maxLineLength(4096);
    char line[maxLineLength+1];
    int linesRead(0);
    while( 1 )
    {
        // Read in a line.
        char* pc = fgets(line, maxLineLength, punchlineFile);
        ++ linesRead;
        //t.Info( "linesRead=%d", linesRead );
        if( !pc ) break;
        assert( strlen(line) < maxLineLength );
        assert( line[strlen(line)-1] == '\n' );
        if( feof( punchlineFile ) )
        {
            break;
        }
        if( (line[0] == '#') || (line[0]=='\n') )
        {
            continue;
        }


        // Put english into vector of Punchlines.
        Punchline nextPunchline;
        string punchline( line );
        StrTrim( punchline );
        nextPunchline.english = punchline;

        // Normalize.
        StrNormalize( punchline );
        if( punchline.empty() )
        {
            continue;
        }
        nextPunchline.normalizedEnglish = punchline;
        //t.Info( "normalized:|%s|", punchline.c_str() );

        // Translate into phonetic.
        string tPhonetic;
        BytePhoneticString tBytePhonetic;
        int status = m_phonics.NormalizedEnglish2BytePhonetic(
            nextPunchline.normalizedEnglish,
            cmdline.Latin() ? Phonics::latin_eccl : Phonics::phon_dict,
            &tPhonetic,
            &tBytePhonetic, 0 );
        if( status == 0 )
        {
            nextPunchline.phonetic = tPhonetic;
            nextPunchline.bytePhonetic = tBytePhonetic;
            m_punchlines.push_back( nextPunchline );
        }
    }
}

vector<Punchline>::const_iterator
Punchlines::begin() const
{
    return m_punchlines.begin();
}

vector<Punchline>::const_iterator
Punchlines::end() const
{
    return m_punchlines.end();
}
