#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main(int argc, char** argv)
{
    ifstream infile(argv[1]);
    char oneline[80];
    string aline;
    infile >> aline;
    while(!infile.eof())
    {
        cout << aline << '\n';
        infile >> aline;
    } 
}
