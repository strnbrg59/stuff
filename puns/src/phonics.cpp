#include "cmdline.hpp"
#include "punutils.hpp"
#include "phonics.hpp"
#include "trace.hpp"
#include <iostream>
#include <cassert>
#include <climits>
#include <cstring>
#include <algorithm>
using std::make_pair;

Phonics::Phonics( string datadir, Cmdline const & cmdline )
  : m_cmdline( cmdline )
{
    LoadBytePhonDict( (datadir + "phonemes.txt").c_str() );
    LoadPhoneticDictionary( (datadir + "cmudict.0.6.txt").c_str());
    CheckForUnknownPhonemes();
    LoadPhonemeDistances( (datadir + "phoneme_distances.txt").c_str());
}


void
Phonics::LoadPhoneticDictionary( char const * infilename )
{
    FILE * phonDictFile = fopen( infilename, "r" );
    assert( phonDictFile );

    unsigned int const maxLineLength(128);
    char line[maxLineLength+1];
    while( 1 )
    {
        // Read in a line.
        char * pc;
        pc = fgets( line, maxLineLength, phonDictFile );
        if( !pc ) break;
        assert( strlen(line) < maxLineLength );
        assert( line[strlen(line)-1] == '\n' );
        if( feof( phonDictFile ) )
        {
            break;
        }
        if( (line[0] == '#') || (line[0]=='\n') )
        {
            continue;
        }

        // Tokenize: first string is English, the rest is phonetic.
        // When there are variant pronunciations, the first string will appear
        // multiple times, with (2), (3), etc appended.  It's because of those
        // variant pronunciations that we're using a multimap.
        char * lastTok;
        char * english;
        char * phonetic;
        english = strtok_r( line, " ", &lastTok );
        assert( english );
        char * c;
        if( c = strchr( english, '(' ) )
        {   // variant pronunciation
            assert( strchr( english, ')' ) == c+2 );
            assert( strpbrk( c, "123456789" ) );
            *c = 0;
        }
        phonetic = strtok_r( 0, "\n", &lastTok );
        assert( phonetic );
        string strPhonetic( phonetic );
        StrNormalize( strPhonetic );
        m_phonDict.insert( std::make_pair(english, strPhonetic) );
    }
}


BytePhoneticString
Phonics::Phonetic2Bytephonetic( string phonetic ) const
{
    BytePhoneticString result;
    char * from = new char[ phonetic.size() + 1];
    strcpy( from, phonetic.c_str() );

    char * tok;
    char * last;

    tok = strtok_r( from, " ", &last );
    while( tok )
    {
        // This assertion should never be triggered; errors like this should
        // have been caught by CheckForUnknownPhonemes().
        assert( m_bytePhonDict.Contains( tok ) );

        result.push_back( m_bytePhonDict.Find( tok ) );
        tok = strtok_r( 0, " ", &last );
    }

    delete [] from;
    return result;
}


void
Phonics::LoadBytePhonDict( char const * infilename )
{
    FILE * phonemeFile = fopen( infilename, "r" );
    assert( phonemeFile );

    unsigned int const maxLineLength(5);
    char line[maxLineLength+1];
    unsigned char byteCode = 0;
    while( 1 )
    {
        // Read in a line.
        char * pc;
        pc = fgets( line, maxLineLength, phonemeFile );
        if( !pc ) break;
        assert( strlen(line) < maxLineLength );
        assert( line[strlen(line)-1] == '\n' );
        if( feof( phonemeFile ) )
        {
            break;
        }
        if( (line[0] == '#') || (line[0]=='\n') )
        {
            continue;
        }
        string strLine( line );
        StrTrim( strLine );
        m_bytePhonDict.Insert( strLine, byteCode );
        ++byteCode;
    }
}


void
Phonics::LoadPhonemeDistances( char const * infilename )
{
    // Initialize to a diapositive identity matrix.
    int nPhonemes = m_bytePhonDict.Size();
    m_phonemeDistances.resize( nPhonemes );
    for( unsigned int i=0;i<nPhonemes;++i )
    {
        m_phonemeDistances[i].resize( nPhonemes );
        m_phonemeDistances[i].insert( m_phonemeDistances[i].begin(),
                                      nPhonemes, 10 );
        m_phonemeDistances[i][i] = 0;
    }

    FILE * distanceFile = fopen( infilename, "r" );
    assert( distanceFile );

    unsigned int const maxLineLength(12);
    char line[maxLineLength+1];
    char byteCode = 0;
    while( 1 )
    {
        // Read in a line.
        char * pc;
        pc = fgets( line, maxLineLength, distanceFile );
        if( !pc ) break;
        assert( strlen(line) < maxLineLength );
        assert( line[strlen(line)-1] == '\n' );
        if( feof( distanceFile ) )
        {
            break;
        }
        if( (line[0] == '#') || (line[0]=='\n') )
        {
            continue;
        }

        char * last;
        char * phonetic1 = strtok_r( line, " ", &last );
        assert( phonetic1 );
        string strPhonetic1( phonetic1 );

        char * phonetic2 = strtok_r( 0, " ", &last );
        assert( phonetic2 );
        string strPhonetic2( phonetic2 );

        char * chDist = strtok_r( 0, "\n", &last );
        assert( chDist );
        int dist = atoi(chDist);

        int ndx1 = int( m_bytePhonDict.Find( strPhonetic1 ) );
        int ndx2 = int( m_bytePhonDict.Find( strPhonetic2 ) );
        m_phonemeDistances[ndx1][ndx2] = dist;
        m_phonemeDistances[ndx2][ndx1] = dist;
    }
}


/** Returns status: 0=normal, 1=word_not_found
*   If arg wordPositions!=0, then fills it with an array that gives, for every
*   element of bytePhonetic, the word (0,1,2,...) in arg normalizedEnglish in
*   which that byte is a sound.
*/
int 
Phonics::NormalizedEnglish2BytePhonetic(
    string normalizedEnglish,
    XlationMethod xlationMethod,
    string * phoneticPhrase,
    BytePhoneticString * bytePhonetic,
    vector<int> * wordPositions ) const
{
    Trace t("Phonics::NormalizedEnglish2BytePhonetic()");
    char * tokbuf = new char[ normalizedEnglish.size()+1 ];
    strcpy( tokbuf, normalizedEnglish.c_str() );

    char * last;
    char * tok = strtok_r( tokbuf, " ", &last );
    *phoneticPhrase = "";
    PhonDict::const_iterator pPhonDict;
    string onePhoneticWord;
    int iWord(0);
    while( tok )
    {
        if(xlationMethod == phon_dict)
        {
            pPhonDict = m_phonDict.find( string(tok) );
            if(pPhonDict == m_phonDict.end())
            {
                //t.Info("Word not found:|%s|", tok );
                delete [] tokbuf;
                return 1;
            } else
            {
                onePhoneticWord = pPhonDict->second;
            }
        } else if(xlationMethod == latin_eccl)
        {
            onePhoneticWord = m_phoneticComputer.Convert(tok);
        }

        (*phoneticPhrase) += onePhoneticWord + " ";
        if( wordPositions )
        {
            BytePhoneticString bytePhonetic(
                Phonetic2Bytephonetic( onePhoneticWord ) );
            for( unsigned i=0;i<bytePhonetic.size();++i )
            {
                wordPositions->push_back( iWord );
            }
        }
    
        tok = strtok_r( 0, " ", &last );
        ++iWord;
    }

    phoneticPhrase->erase( phoneticPhrase->size() - 1 );
    *bytePhonetic = Phonetic2Bytephonetic( *phoneticPhrase );

    delete [] tokbuf;
    return 0;
}


/** This is key.  If you don't run it, you'll get a mysterious failure if
 any phonetic spellings in your phonetic dictionary contain phonemes that
 aren't known to your byte-phonetic dictionary.  This can happen very easily
 when you add your own useful words and names to the phonetic dictionary.
*/
void
Phonics::CheckForUnknownPhonemes()
{
    for( PhonDict::const_iterator i = m_phonDict.begin();
         i != m_phonDict.end();
         ++i )
    {
        char * buf = new char[ i->second.size() + 1 ];
        strcpy( buf, i->second.c_str() );
        
        char * lastch;
        char * tok = strtok_r( buf, " ", &lastch );
        while( tok )
        {
            if( ! m_bytePhonDict.Contains( tok ) )
            {
                std::cerr << "Found unknown phoneme |" << tok << "| in phonetic "
                     << "spelling of " << i->first << '\n';
                std::cerr << "Either change that spelling, or add the phoneme to "
                        "your phoneme list (probably ../data/phonemes.txt)\n";
                assert(0);
            }
            tok = strtok_r( 0, " ", &lastch );
        }
        delete [] buf;
    }
}


unsigned char
BytePhonDict::Find( string str ) const
{
    return m_str2charDict.find( str )->second;
}

char const *
BytePhonDict::ReverseFind( unsigned char c ) const
{
    return m_char2strDict.find( c )->second.c_str();
}

bool
BytePhonDict::Contains( string str ) const
{
    return m_str2charDict.find( str ) != m_str2charDict.end();
}

void
BytePhonDict::Insert( string str, unsigned char c )
{
    m_str2charDict.insert( make_pair(string(str),c) );
    m_char2strDict.insert( make_pair(c, string(str)) );
    assert( m_str2charDict.size() == m_char2strDict.size() );
}

unsigned int
BytePhonDict::Size() const
{
    return m_str2charDict.size();
}


unsigned
PhonemeCompare::operator()( unsigned char pHaystack, unsigned char pNeedle ) const
{
    assert( (pHaystack < m_distances.size())
        &&  (pNeedle < m_distances[pHaystack].size()) );
    return abs(m_distances[pHaystack][pNeedle]);
}

BytePhoneticString::const_iterator
Phonics::FindClosestMatch( BytePhoneticString::const_iterator haystackBegin,
                           BytePhoneticString::const_iterator haystackEnd,
                           BytePhoneticString::const_iterator needleBegin,
                           BytePhoneticString::const_iterator needleEnd,
                           int wordCutoff,
                           unsigned& sumPhonemeDistances )
  const
{
    Trace t("Phonics::FindClosestMatch()");
    typedef BytePhoneticString::const_iterator Iter;
    PhonemeCompare pc( m_phonemeDistances,
                       m_bytePhonDict, m_cmdline );

    // "loci" is the number of positions, in the haystack (a verse of
    // Shakespeare) against which we can try to match the needle (the "clue").
    int loci = 1 + (haystackEnd - haystackBegin) - (needleEnd - needleBegin);
    if( loci < 1 ) // case of haystack shorter than needle
    {
        sumPhonemeDistances = __INT_MAX__;
        return haystackBegin;
    }
    std::vector<unsigned> distTotals( loci );
    std::fill( distTotals.begin(), distTotals.end(), __INT_MAX__ );

    int locus = 0;
    for( Iter it_h=haystackBegin;
         it_h < haystackEnd - (needleEnd-needleBegin);
         ++it_h, ++locus )
    {
        int skips = 0;
        int const maxSkips = 1;
        distTotals[locus] = 0;
        for( Iter it_n=needleBegin; it_n!=needleEnd; ++it_n )
        {
            int dist = pc( *(it_h + skips + (it_n - needleBegin)), *it_n );
            distTotals[locus] += dist;
            if( distTotals[locus] > wordCutoff )
            {
                // Before giving up on this verse, try skipping over a phoneme
                // of the haystack; see if the needle might match a disjoint
                // subset of the haystack.
                if(  (skips < maxSkips) && (it_n > needleBegin)
                  && (it_h + skips + 1 + (needleEnd-it_n) < haystackEnd) )
                {
                    ++skips;
                    distTotals[locus] -= dist;
                    --it_n;
                } else
                {
                    break;
                }
            }
        }
        distTotals[locus] += m_cmdline.DisjointPenalty()*skips;
    }

    std::vector<unsigned>::const_iterator minelem =
        std::min_element( distTotals.begin(), distTotals.end() );
    sumPhonemeDistances = *minelem;
    return haystackBegin + (minelem - distTotals.begin());
}


PhoneticComputer::PhoneticComputer()
{
    InitTable();
}

string
PhoneticComputer::Convert(string word) const
{
    Trace t("PhoneticComputer::Convert()");
    string result;
    unsigned int pos = 0;
    while(pos < word.size())
    {
        int len;
        for(len = 3; len > 0; --len)
        {
            if((pos < word.size() - len + 1)
            && (m_table.find(string(word, pos, len)) != m_table.end()))
            {
                PhonicTableT::const_iterator iter =
                    m_table.find(string(word, pos, len));
                result += " " + iter->second;
                pos += len;
                break;
            }
        }
        // If got here, it means we're missing a phoneme from m_table.
        if(len==0) t.Error("No phonetic for |%s|", word.c_str() + pos);
        assert(len>0);
    }
    assert(result.size() > 0);
    result.erase(result.begin(), result.begin()+1);
    // Trims off the leading space.
    return result;
}


void
PhoneticComputer::InitTable()
{
    m_table.insert(make_pair("A", "AH"));
    m_table.insert(make_pair("E", "EH"));
    m_table.insert(make_pair("I", "IH"));
    m_table.insert(make_pair("O", "AO"));
    m_table.insert(make_pair("U", "UH"));
    m_table.insert(make_pair("AE", "EH"));
    m_table.insert(make_pair("AI", "AY"));
    m_table.insert(make_pair("AU", "AW"));
    m_table.insert(make_pair("OI", "OY"));
    m_table.insert(make_pair("OE", "EH"));
    m_table.insert(make_pair("CAE", "CH EH"));
    m_table.insert(make_pair("CE", "CH EH"));
    m_table.insert(make_pair("CI", "CH IH"));
    m_table.insert(make_pair("GAE", "JH EH"));
    m_table.insert(make_pair("GE", "JH EH"));
    m_table.insert(make_pair("GI", "JH IH"));
    m_table.insert(make_pair("B", "B"));
    m_table.insert(make_pair("BB", "B"));
    m_table.insert(make_pair("C", "K"));
    m_table.insert(make_pair("D", "D"));
    m_table.insert(make_pair("DD", "D"));
    m_table.insert(make_pair("F", "F"));
    m_table.insert(make_pair("FF", "F"));
    m_table.insert(make_pair("G", "G"));
    m_table.insert(make_pair("H", "HH"));
    m_table.insert(make_pair("K", "K"));
    m_table.insert(make_pair("KK", "K"));
    m_table.insert(make_pair("L", "L"));
    m_table.insert(make_pair("LL", "L"));
    m_table.insert(make_pair("M", "M"));
    m_table.insert(make_pair("MM", "M"));
    m_table.insert(make_pair("N", "N"));
    m_table.insert(make_pair("NN", "N"));
    m_table.insert(make_pair("P", "P"));
    m_table.insert(make_pair("PP", "P"));
    m_table.insert(make_pair("Q", "K"));
    m_table.insert(make_pair("R", "R"));
    m_table.insert(make_pair("RR", "R"));
    m_table.insert(make_pair("S", "S"));
    m_table.insert(make_pair("SS", "S"));
    m_table.insert(make_pair("T", "T"));
    m_table.insert(make_pair("TT", "T"));
    m_table.insert(make_pair("V", "V"));
    m_table.insert(make_pair("X", "K S"));
    m_table.insert(make_pair("Y", "IH"));
    m_table.insert(make_pair("Z", "Z"));
    m_table.insert(make_pair("ZZ", "Z"));
}
