#include "trace.hpp"
#include "myregexp.hpp"
#include "file_collector.hpp"

#include <algorithm>
#include <sys/types.h>
#include <dirent.h>

FileCollector::FileCollector( string directory, string suffix )
{
    Trace t("FileCollector ctor");    

    DIR * dir;
    dirent * next_elem;

    dir = opendir( directory.c_str() );
    if( !dir )
    {
        t.FatalError( "Can't open directory %s.", directory.c_str() );
    }

    MyRegexp suffix_re( suffix + "$");
    // FIXME: If there is a '.' in suffix, we need to escape it.

    next_elem = readdir(dir);
    while( next_elem )
    {
        while( next_elem && !suffix_re.matches(next_elem->d_name) )
        {
            next_elem = readdir(dir);
            //t.Info( "next_elem=%X, dname=%s", next_elem, next_elem->d_name );
        }
        if( next_elem)
        {
            m_filenames.push_back( next_elem->d_name );
        }
        next_elem = readdir(dir);
    }

    sort( m_filenames.begin(), m_filenames.end() );

    closedir( dir );
}

vector<string>
FileCollector::GetFiles() const
{
    return m_filenames;
}

