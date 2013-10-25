#ifndef INCLUDED_FILECOLLECTOR_H
#define INCLUDED_FILECOLLECTOR_H

#include <string>
#include <vector>
using std::string;
using std::vector;

/** Serves up all the files, in a directory, that have a given suffix.*/
class FileCollector
{
  public:
    FileCollector( string directory, string suffix );
    vector<string> GetFiles() const;
  private:
    vector<string> m_filenames;
};

#endif // INCLUDED_FILECOLLECTOR_H
