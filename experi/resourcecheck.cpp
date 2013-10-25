#include <cstdio>
#include <cassert>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <boost/tokenizer.hpp>
using std::cout;
using std::string;
using std::map;
using std::pair;
using std::vector;
using boost::tokenizer;

// Key: partition name (e.g. /dev/sda1)
// Value: Start and End cylinder, as given by /sbin/sfdisk.
typedef map<string, pair<int,int> > PartitionCylinders;


/** Exception-safe wrapper around stdio FILE */
struct CPPFILE
{
    CPPFILE(FILE* f) : m_rep(f) { }
    ~CPPFILE() { fclose(m_rep); }
    operator FILE*() { return m_rep; }
    FILE* m_rep;
};

/** In bytes. */
long
getTotalSystemMemory()
{
    CPPFILE infile = popen("cat /proc/meminfo | grep 'MemTotal:' |"
                           "awk '{printf(\"%d\",$2)}'", "r");
    char buf[80];
    fgets(buf, 80, infile);
    char* endptr;
    long kb = strtol(buf, &endptr, 10);
    assert(endptr && (*endptr == '\0'));
    return kb * 1024;
}


void
findPartitionCylinderInfo(PartitionCylinders& cylinders,
                          int& totalCylinders)
{
    typedef tokenizer<boost::char_separator<char> > Tok;
    CPPFILE infile = popen("/sbin/sfdisk -l", "r");
    assert(infile);
    int const maxlinelen = 256;
    char buf[maxlinelen];
    int iLine = 1;

    fgets(buf, maxlinelen, infile); // blank line
    ++iLine;

    //
    // Find total cylinders (third token on second line, e.g.
    // "Disk /dev/sda: <n> cylinders, <etc>
    //
    fgets(buf, maxlinelen, infile);
    ++iLine;
    string str(buf);
    Tok tok(str, boost::char_separator<char>(" "));
    vector<std::string> tokvect(tok.begin(), tok.end());
    char* endptr;
    totalCylinders = strtol(tokvect[2].c_str(), &endptr, 10);
    assert(endptr && (*endptr == '\0'));

    //
    // Fill PartitionCylinders -- start and end cylinder for each partition.
    // Starts at line 7;
    //
    while (iLine < 7) { // Skipping over lines we're not interested in.
        fgets(buf, maxlinelen, infile);
        ++iLine;
    }

    // Getting the lines we care about -- they start with /dev/sda<n>.
    while (!feof(infile)) {
        string str(buf);
        fgets(buf, maxlinelen, infile);
        Tok tok(str, boost::char_separator<char>(" +-"));
        vector<std::string> tokvect(tok.begin(), tok.end());
        string partitionName = tokvect[0];
        int extracol =  (tokvect[1] == "*");
        int startCyl = strtol(tokvect[1+extracol].c_str(), &endptr, 10);
        int endCyl = strtol(tokvect[2+extracol].c_str(), &endptr, 10);
        cylinders[partitionName] = std::make_pair(startCyl, endCyl);
    }
}
    

int main()
{
    double bytesPerCylinder = 8225280;
    PartitionCylinders partitionCylinders;
    int totalCylinders;
    findPartitionCylinderInfo(partitionCylinders, totalCylinders);

    // Print partition info:
    cout << "Partition info:\n";
    for (PartitionCylinders::iterator iter=partitionCylinders.begin();
         iter != partitionCylinders.end();
         ++iter) {
        cout << "  (" << iter->first << ", [" << iter->second.first
             << ", " << iter->second.second << "])\n";
    }

    cout << "Total cylinders = " << totalCylinders << '\n';
    cout << "Disk size = " << totalCylinders*bytesPerCylinder/1E9 << "GB\n";
    cout << "System memory = " << getTotalSystemMemory()/1E9 << "GB\n";
}
