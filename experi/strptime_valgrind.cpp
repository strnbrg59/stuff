#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
#include <iostream>
#include <cassert>
using namespace std;


void func(const char* datetime)
{
    struct tm mytm;
    memset(&mytm, 0, sizeof(mytm));

    strptime(datetime, "%Y/%m/%d %T", &mytm);
//    strptime(wholething.c_str(), "%Y/%m/%d %T", &mytm);
//    strptime("15:21:27 2011/11/18", "%Y/%m/%d %T", &mytm);

    time_t epoch = mktime(&mytm);
}

int main()
{
    string mydate("2011/11/18");
    string mytime("15:21:27");
    string wholething(mydate + " " + mytime);

//  const char* wholething_str = wholething.c_str();
    func(wholething.c_str());

    if (wholething.c_str()) {
        cout << "hello\n";
    }
}
