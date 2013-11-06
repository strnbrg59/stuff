#include "fingers.hpp"
#include "trace.hpp"
#include <iostream>
#include <fstream>
using namespace std;


Infile::Infile(string filename) : m_filename(filename)
{
    Trace t("Infile::Infile()");
    ifstream reader(filename.c_str());
    string oneline;
    while(!reader.eof()) {
        reader >> oneline;
        m_noteElements.push_back(Note(oneline));
        ostringstream ost;
        ost << "oneline=<" << oneline << ">";
        t.Info("%s", ost.str().c_str());
    } 
}


NoteIterator
Infile::begin() const
{
    return m_noteElements.begin();
}


NoteIterator
Infile::end() const
{
    return m_noteElements.end();
}
