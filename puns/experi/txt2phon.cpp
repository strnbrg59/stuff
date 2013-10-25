#include <string>
#include <iostream>
#include <map>
using namespace std;

/** Computes, rather than looks up, phonetic representations of words. */
class PhoneticComputer
{
public:
    PhoneticComputer()
    {
        InitTable();
    }

    string Convert(string word)
    {
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
            if(len==0) cerr << "Error: no phonetic for |"
                            << (word.c_str() + pos) << "|\n";
            assert(len>0);
        }
        assert(result.size() > 0);
        result.erase(result.begin(), result.begin()+1);
        // Trims off the leading space.
        return result;
    }

private:
    struct LengthSort
    {
        bool operator()(string a, string b)
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
    
    void InitTable()
    {
        m_table.insert(make_pair("a", "AH"));
        m_table.insert(make_pair("e", "EH"));
        m_table.insert(make_pair("i", "IH"));
        m_table.insert(make_pair("o", "OH"));
        m_table.insert(make_pair("u", "UH"));
        m_table.insert(make_pair("ae", "EH"));
        m_table.insert(make_pair("ai", "AY"));
        m_table.insert(make_pair("au", "AW"));
        m_table.insert(make_pair("oi", "OY"));
        m_table.insert(make_pair("oe", "EH"));
        m_table.insert(make_pair("cae", "CH EH"));
        m_table.insert(make_pair("ce", "CH EH"));
        m_table.insert(make_pair("ci", "CH IH"));
        m_table.insert(make_pair("gae", "JH EH"));
        m_table.insert(make_pair("ge", "JH EH"));
        m_table.insert(make_pair("gi", "JH IH"));
        m_table.insert(make_pair("b", "B"));
        m_table.insert(make_pair("c", "K"));
        m_table.insert(make_pair("d", "D"));
        m_table.insert(make_pair("f", "F"));
        m_table.insert(make_pair("g", "G"));
        m_table.insert(make_pair("h", "HH"));
        m_table.insert(make_pair("l", "L"));
        m_table.insert(make_pair("m", "M"));
        m_table.insert(make_pair("n", "N"));
        m_table.insert(make_pair("p", "P"));
        m_table.insert(make_pair("q", "K"));
        m_table.insert(make_pair("r", "R"));
        m_table.insert(make_pair("s", "S"));
        m_table.insert(make_pair("t", "T"));
        m_table.insert(make_pair("v", "V"));
        m_table.insert(make_pair("x", "K S"));
        m_table.insert(make_pair("z", "Z"));
    }
};

#ifdef UNIT_TEST    
int main()
{
    PhoneticComputer pc;
    cerr << "pc.Convert(\"aeo\")=|" << pc.Convert("aeo") << "|\n";
    cerr << "pc.Convert(\"auctor\")=|" << pc.Convert("auctor") << "|\n";
    cerr << "pc.Convert(\"incipio\")=|" << pc.Convert("incipio") << "|\n";
    cerr << "pc.Convert(\"ingenuum\")=|" << pc.Convert("ingenuum") << "|\n";
    cerr << "pc.Convert(\"caesar\")=|" << pc.Convert("caesar") << "|\n";
//  Can't do this one -- gotta break it down to individual words first.
//    cerr << "pc.Convert(\"bonis nocet quisquis malis pepercit\")=|" << pc.Convert("bonis nocet quisquis malis pepercit") << "|\n";
}
#endif // UNIT_TEST
