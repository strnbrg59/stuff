#include "phonics.hpp"
#include "punchlines.hpp"
#include "punserver.hpp"
#include "punutils.hpp"
#include "trace.hpp"
#include "cmdline.hpp"
#include "exceptions.hpp"
#include "network-utils.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <cstring>
#include <sstream>
#include <set>
#include <boost/tokenizer.hpp>
using boost::tokenizer;
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

using std::cin; using std::cout; using std::cerr; using std::endl;
using std::vector;
using std::string;

int main( int argc, char * * argv )
{
    Cmdline cmdline( argc, argv );
    CmdlineFactory::Init(cmdline);
    Phonics phonics( DATADIR, cmdline ); // See Makefile.am for DATADIR
    Punchlines punchlines( (string(DATADIR) + cmdline.Punchlines()).c_str(),
                           phonics );
    PunServer server( cmdline, phonics, punchlines  );
}


PunServer::PunServer( Cmdline& cmdline, Phonics const & phonics,
                      Punchlines const & punchlines )
  : m_cmdline( cmdline ),
    m_phonics( phonics ),
    m_punchlines( punchlines )
{
    Trace t("main()");
    int const maxClueLength = 80;
    char cluePkg[ maxClueLength ];

    if( m_cmdline.ClueSource() == string("stdin") )
    {
        // Return phonetic spellings interactively.
        cout << "% " << std::flush;
        char * fgetsRetn = fgets( cluePkg, maxClueLength, stdin );
        while( fgetsRetn )
        {
            cout << FindPun( string(cluePkg) );
            cout << "% " << std::flush;
            fgetsRetn = fgets( cluePkg, maxClueLength, stdin );
        }
    } else
    if( m_cmdline.ClueSource() == string("inet") )
    {
        NetworkUtils netutils;
        netutils.BecomeDaemon( m_cmdline.LogFile() );
        std::ofstream logfile( m_cmdline.LogFile().c_str() );
        int sock = netutils.GetServerSocket( m_cmdline.InetPort() );
        t.Info( "Server is ready." );
    
        // Accept connections from clients.
        while( 1 )
        {
            sockaddr_in client_addr;
            socklen_t sizeof_client_addr = sizeof( client_addr );
            int sock1 =
                accept( sock, (sockaddr*)&client_addr, &sizeof_client_addr);
            if( sock1 < 0 )
            {
                t.FatalError( "accept() failed" );
            }
            m_cmdline.Reload();

            // We can run in no-forking mode, for easier debugging.
            if( m_cmdline.DoFork() )
            {
                t.Warning( "Forking..." );
                netutils.ForkGrandchild( sock1 );
            }
            // grandchild from here on.


            while( ConverseWithNetClient( sock1, netutils ) )
            {
            }

            close( sock1 );
            if( m_cmdline.DoFork() )
            {
                exit(0);
            }
        }
    } else // From a file -- good for debugging
    {
        FILE * cluefile = fopen( m_cmdline.ClueSource().c_str(), "r" );
        if( ! cluefile )
        {
            t.FatalError( "Unable to open file|%s|", cluefile );
        }
        
        char * pc = fgets( cluePkg, maxClueLength, cluefile );
        while( ! feof(cluefile) )
        {
            cerr << "cluePkg=" << cluePkg;
            cerr << FindPun( string(cluePkg) );
            cerr << endl;
            pc = fgets( cluePkg, maxClueLength, cluefile );
        }
    }        
}


/** Arg cluePkg is clue[|badness[|multispan]]. */
void
PunServer::UnpackCluePackage( string cluePkg, string * clue,
                   int * badness, bool * multispan )
{
    Trace t("UnpackCluePackage()");

    char * buf = new char[cluePkg.size()+1];
    strcpy( buf, cluePkg.c_str() );
    char * lastChar;
    *clue = strtok_r( buf, "|\n", &lastChar );

    char * ctol = strtok_r( 0, "|\n", &lastChar );
    if( ctol )
    {
        *badness = atoi( ctol );
        char * cspan = strtok_r( 0, "\n", &lastChar );
        if( cspan )
        {
            *multispan = atoi( cspan );
        }
    }
    delete[] buf;
}


/** Returns pun(s) found, prefixed by its badness score.
 *  Arg cluePkg is pureClue[|badness[|multispan]].
*/
string
PunServer::FindPun( string cluePkg )
{
    Trace t("FindPun()");
    int badnessMultiplier(2);

    string pureClue;
    int    badness(1);
    bool   multispan(false);
    if( cluePkg.size() < 2 ) // Happens if user just hits "return".
    {
        return "";
    }
    UnpackCluePackage( cluePkg, &pureClue, &badness, &multispan );
    t.Info( "clue=%s, badness=%d", pureClue.c_str(), badness );
    int wordCutoff = badnessMultiplier * badness;

    StrNormalize( pureClue );
    vector<BytePhoneticString> bytePhoneticClues;

    //
    // If there's only one word in the clue, then consider alternative
    // pronunciations.  Otherwise, not until we code the correct code for the
    // cross-product of the pronunciations.
    //
    tokenizer<> ttokens(pureClue); // FIXME: words broken at apostrophes!
    vector<string> tokens(ttokens.begin(), ttokens.end());
    for(vector<string>::iterator titer = tokens.begin(); titer != tokens.end();
        ++titer)
    {
        std::pair<PhonDict::const_iterator, PhonDict::const_iterator>
            phoneticClues( m_phonics.equal_range( *titer ) );
        int pron_count = m_phonics.count( *titer );
        if( pron_count == 0 )
        {
            t.Info( "%s not found in dictionary.", titer->c_str() );
            return *titer + string(" not found in dictionary.  Sorry.\n");
        } else
        if( pron_count > 1 )
        {
            t.Info( "%d alternate pronunciations for %s",
                    pron_count-1, titer->c_str() );
        }

        if(tokens.size() == 1)
        {
            // Construct the collection of byte-phonetic representations of the
            // clue.
            for( PhonDict::const_iterator iClue = phoneticClues.first;
                 iClue != phoneticClues.second;
                 ++iClue )
            {
                bytePhoneticClues.push_back(
                   m_phonics.Phonetic2Bytephonetic( iClue->second ));
            }
        } else
        {
            // Construct a single byte-phonetic representation, but it'll be
            // the concatenation of all the clue words.
            PhonDict::const_iterator iClue = phoneticClues.first;
            if(bytePhoneticClues.empty())
            {
                bytePhoneticClues.push_back(
                   m_phonics.Phonetic2Bytephonetic( iClue->second ));
            } else
            {
                BytePhoneticString nextWord(m_phonics.Phonetic2Bytephonetic(
                                            iClue->second));
                bytePhoneticClues[0].insert(bytePhoneticClues[0].end(),
                                            nextWord.begin(), nextWord.end());
            }
        }
    }

    std::set<string> resultSet;
    int nMatchesFound(0);
    for( vector<BytePhoneticString>::const_iterator bpClue =
            bytePhoneticClues.begin();
         bpClue != bytePhoneticClues.end();
         ++bpClue )
    {
        // Go through all the punchlines, looking for a match.
        for( vector<Punchline>::const_iterator punchlinesIter
                = m_punchlines.begin();
             punchlinesIter != m_punchlines.end();
             ++punchlinesIter )
        {
            unsigned sumPhonemeDifferences = 0;
            BytePhoneticString::const_iterator pos =
                m_phonics.FindClosestMatch(
                                  punchlinesIter->bytePhonetic.begin(),
                                  punchlinesIter->bytePhonetic.end(),
                                  bpClue->begin(),
                                  bpClue->end(),
                                  wordCutoff,
                                  sumPhonemeDifferences );
            bool good = true;

            // Check that phoneme differences don't add up to too much.
            if( sumPhonemeDifferences > wordCutoff )
            {
                good = false;
            }

            // Exclude punchline, if it contains the clue word itself.
            if( punchlinesIter->normalizedEnglish.find(pureClue)
            != punchlinesIter->normalizedEnglish.npos )
            {
                good = false;
            }

            if( !good ) continue;

            // Check how many punchline words the clue maps to.
            int nWordsSpanned(0);
            string taggedEnglish( TagMatch( punchlinesIter,
                                            *bpClue, pos,
                                            &nWordsSpanned ) );
            if( (nWordsSpanned < 2) && multispan )
            {
                good = false;
            }

            // Found a good pun:
            if( good )
            {
                std::ostringstream tol_tag;
                tol_tag << '(' 
                  << int(0.5+sumPhonemeDifferences/(0.0+badnessMultiplier))
                  << ") " << taggedEnglish;
                resultSet.insert( tol_tag.str() );

                // Prevent denial-of-service attacks.  The downside is that
                // low-badness puns, if they come from late in our punchlines
                // file, won't show up if you choose a high badness badness.
                if( ((++nMatchesFound) >= m_cmdline.MaxMatchesInInetMode())
                &&  (m_cmdline.ClueSource() == "inet") )
                {
                    resultSet.insert( "<FONT COLOR=#FF0000>...and too many more"
                                      " to print</FONT>" );
                    break;
                }
            }
        }
    }

    string result("");
    t.Info( "resultSet.size()=%d", resultSet.size() );
    for( std::set<string>::const_iterator i=resultSet.begin();
         i!=resultSet.end();
         ++i )
    {
        result += *i + '\n';
    }
    if( resultSet.size() == 0 )
    {
        t.Info( "No matches found that meet all the selection criteria." );
        result = "No matches found that meet all the selection criteria."
                 "  Sorry.\n";
    }
    return result;
}

/** Returns 0 when client seems to have disconnected. */
int
PunServer::ConverseWithNetClient( int socket, NetworkUtils & netutils )
{
    Trace t("ConverseWithNetClient()");
    try
    {
        string fromClient( netutils.ReadLine( socket, 100 ) );
        t.Info( "Client sent|%s|", fromClient.c_str() );
        netutils.ToPeer( socket, FindPun( fromClient ) );
        t.Info( "Response sent to client.");
    }
    // Exceptions that might come from ReadLine().
    catch( EofException & eof )
    {
        t.Info( "Received EOF.  Normal, after QUIT."
                "Closing connection..." );
        return 0;
    }
    catch( ReadErrorException & re )
    {
        t.Info() << re << '\n';
    }
    catch( TimeoutException & toe )
    {
        t.Info() << toe << '\n';
    }
    catch( ... )
    {
        assert( 1 );
    }

    close(socket);
    return 0;

}


/** For every word in normalizedVerse find the position in rawVerse where that
 *  word starts.
 *  For example if
 *     rawVerse        = "With harsh-resounding trumpets'"
 *  and
 *     normalizedVerse = "WITH HARSH RESOUNDING TRUMPETS"
 *  then return a vector with the following values: {0,5,11,22}.
 *
 *  If you can't find the corresponding word in rawVerse, issue an error, but
 *  try fixing it up by choosing the previous word-start position, or 0 if we're
 *  on the first word.  But this should never happen.
*/
vector<int> 
PunServer::FindNormalizedWordStarts( string rawVerse,
                                     string normalizedVerse ) const
{
    Trace t("FindNormalizedWordStarts()");
    char * sraw = new char[rawVerse.size()+1];
    char * snorm = new char[normalizedVerse.size()+1];
    strcpy( sraw, rawVerse.c_str() );
    strcpy( snorm, normalizedVerse.c_str() );

    std::transform( sraw, sraw+rawVerse.size(), sraw, toupper );

    char * tok;
    char * lastChar;
    vector<int> result;
    int prevPos(0);
    int prevTokLen(0);
    tok = strtok_r( snorm, " ", &lastChar );
    while( tok )
    {
        char * pos = strstr( sraw + prevPos + prevTokLen, tok );
        if( pos )
        {
            prevPos = pos - sraw;
        } else
        {
            t.Error( "Can't find |%s| in |%s|!", tok, sraw );
        }
        result.push_back( prevPos );

        prevTokLen = strlen( tok );
        tok = strtok_r( 0, " ", &lastChar );
    }

    delete [] sraw;
    delete [] snorm;
    return result;
}


/** Return a punchline with html tags, denoting a contrasting color, surrounding
    the part that matches the clue.
    Unfortunately, the contrasting-color thing can't be as precise as we'd like.
    Ideally it would start right at the first sound of the punchline that
    matches the first sound of the clue.  The trouble, though, is that our
    phonetic dictionary works at the granularity of words; we know for example
    that "vienna" corresponds to "V IY0 EH1 N AH0", but we don't know which
    part of  "V IY0 EH1 N AH0" corresponds the beginning of the "enna" part of
    "vienna" (which we'd want to know if our clue was perhaps "enemy").
*/
string
PunServer::TagMatch( vector<Punchline>::const_iterator punchlinesIter,
          BytePhoneticString bytePhoneticClue,
          BytePhoneticString::const_iterator posBPP,
                // positionInBytePhoneticPunchline,
          int * nWordsSpanned )
{
        Trace t("TagMatch()");
        Cmdline const& cmdline(CmdlineFactory::TheCmdline());

        vector<int> wordPos1(
            FindNormalizedWordStarts( punchlinesIter->english,
                                      punchlinesIter->normalizedEnglish ) );
        vector<int> wordPos2;
        string tPhonetic;
        BytePhoneticString tBytePhonetic;
        int status( m_phonics.NormalizedEnglish2BytePhonetic(
            punchlinesIter->normalizedEnglish,
            cmdline.Latin() ? Phonics::latin_eccl : Phonics::phon_dict,
            &tPhonetic,
            &tBytePhonetic,
            &wordPos2 ));

        assert( status == 0 ); // If any of these words aren't in the dictionary
                               // we would have noticed that the first time
                               // called NormalizedEnglish2BytePhonetic() --
                               // in LoadPunchlines().
        int iPosBPP( // int position in bytePhoneticPunchline
              posBPP
            - punchlinesIter->bytePhonetic.begin() );
        int englishBeginMarkPos( wordPos1[wordPos2[
                                    iPosBPP]] );

        string hiliteBegin( "<FONT COLOR=#007700>" );
        string hiliteEnd( "</FONT>" );
        string result( punchlinesIter->english );
        result.insert( englishBeginMarkPos, hiliteBegin );

        int iPosBPPEnd = iPosBPP + bytePhoneticClue.size() - 1;
        assert( iPosBPPEnd < wordPos2.size() );

        if( wordPos2[iPosBPPEnd] == wordPos1.size()-1 )
        {
            result.append( hiliteEnd );
        } else
        {   
            int englishEndMarkPos = wordPos1[wordPos2[iPosBPPEnd]+1]
                                  + hiliteBegin.size();
            result.insert( englishEndMarkPos, hiliteEnd );
        }
        *nWordsSpanned = 1 + wordPos2[iPosBPPEnd] - wordPos2[iPosBPP];

        return result;
}
