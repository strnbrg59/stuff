#include <cassert>
#include <iostream>
using namespace std;

class IsVowel
{
public:
    IsVowel()
    {
        memset(m_mask, 0, 256*sizeof(int));
        m_mask['a']=m_mask['e']=m_mask['i']=m_mask['o']=m_mask['u']=
        m_mask['A']=m_mask['E']=m_mask['I']=m_mask['O']=m_mask['U']=1;
    }
    bool operator()(char c) { return m_mask[c]; }

private:
    int m_mask[256];
};

static IsVowel s_isVowel;


void reversedVowels(char* mystr)
{
    int nVowels=0;
    char* vowelBuf = new char[strlen(mystr)];
    char* pMystr = mystr;

    /* Collect the vowels. */
    while(*pMystr)
    {
        if(s_isVowel(*pMystr))
        {
            vowelBuf[nVowels] = *pMystr;
            ++nVowels;
        }
        ++pMystr;
    }

    /* Scan mystr again, inserting collected vowels from back of vowelBuf. */
    int vowelBufPos = nVowels-1;
    pMystr = mystr;
    while(*pMystr)
    {
        if(s_isVowel(*pMystr))
        {
            *pMystr = vowelBuf[vowelBufPos];
            --vowelBufPos;
        }
        ++pMystr;
    }

    delete[] vowelBuf;
}


int main(int argc, char** argv)
{
    assert(argc==2);
    char* buf = new char[strlen(argv[1]) + 1];
    strcpy(buf, argv[1]);

    reversedVowels(buf);
    cout << buf << '\n';

    delete[] buf;
}
