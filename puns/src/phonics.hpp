#ifndef _PHONICS_HPP_
#define _PHONICS_HPP_

#include <string>
#include <map>
#include <vector>
using std::string;
using std::map;
using std::multimap;
using std::vector;
class Cmdline;

typedef multimap<string,string> PhonDict;
class BytePhonDict
{
  public:
    unsigned char Find( string str ) const;
    char const * ReverseFind( unsigned char c ) const;
    bool Contains( string str ) const;
    void Insert( string, unsigned char c );
    unsigned int Size() const;
  private:
    map<string,unsigned char> m_str2charDict;
    map<unsigned char,string> m_char2strDict;
};

typedef vector<unsigned char> BytePhoneticString;

class PhonemeCompare
{
  public:
    PhonemeCompare(
        vector<vector<int> > const & distances,
        BytePhonDict const & bytePhonDict,
        Cmdline const & cmdline )
      : m_distances( distances ),
        m_bytePhonDict( bytePhonDict ),
        m_cmdline( cmdline )
    {
    }
    unsigned operator()( unsigned char pHaystack, unsigned char pNeedle ) const;
  private:
    vector<vector<int> > const & m_distances;
    BytePhonDict const & m_bytePhonDict;
    Cmdline const & m_cmdline;
};


/** Computes, rather than looks up, phonetic representations of words. */
class PhoneticComputer
{
public:
    PhoneticComputer();
    string Convert(string word) const;

private:
    struct LengthSort
    {
        bool operator()(string a, string b) const
        {
            if(a.size() == b.size())
            {
                return a < b;
            } else
            {
                return a.size() > b.size();
            }
        }
    };
    
    typedef map<string, string, LengthSort> PhonicTableT;
    PhonicTableT m_table;
    void InitTable();
};


class Phonics
{
  public:
    enum XlationMethod {phon_dict, latin_eccl};
    Phonics( string datadir, Cmdline const & );
    BytePhoneticString Phonetic2Bytephonetic( string phonetic ) const;
    int NormalizedEnglish2BytePhonetic( string normalizedEnglish,
                                        XlationMethod xlationMethod,
                                        string * phonetic,
                                        BytePhoneticString * bytePhonetic,
                                        vector<int> * wordPositions ) const;
    vector< vector<int> > const & GetPhonemeDistances() const
    {
        return m_phonemeDistances;
    }
    BytePhonDict const & GetBytePhonDict() const
    {
        return m_bytePhonDict;
    }
    std::pair<PhonDict::const_iterator, PhonDict::const_iterator>
        equal_range( string clue ) const
    {
        return m_phonDict.equal_range( clue );
    }
    int count( string clue ) const { return m_phonDict.count( clue ); }

    BytePhoneticString::const_iterator
    FindClosestMatch( BytePhoneticString::const_iterator haystackBegin,
                      BytePhoneticString::const_iterator haystackEnd,
                      BytePhoneticString::const_iterator needleBegin,
                      BytePhoneticString::const_iterator needleEnd,
                      int wordCutoff,
                      unsigned& sumPhonemeDistances ) const;

  private:
    void LoadPhoneticDictionary( char const * phoneticDictionaryFile );
    void CheckForUnknownPhonemes();
    void LoadBytePhonDict( char const * infilename );
    void LoadPhonemeDistances( char const * infilename );

    Cmdline const &       m_cmdline;
    PhonDict              m_phonDict;
    BytePhonDict          m_bytePhonDict;
    vector< vector<int> > m_phonemeDistances;
    PhoneticComputer      m_phoneticComputer;
};    

#endif // _PHONICS_HPP_
