#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <iostream>
#include <cassert>
using namespace std;

int main()
{
    string mydate("2011/11/18");
    string mytime("15:21:27");
    string wholething(mydate + " " + mytime);

    struct tm mytm;
    memset(&mytm, 0, sizeof(mytm));
    const char* wholething_str = wholething.c_str();
    char* ret = strptime(wholething_str, "%Y/%m/%d %T", &mytm);
    if (*ret) {
        cerr << "Error: *ret=|" << *ret << "|, ret-wholething_str="
             << (ret-wholething_str) << "\n";
        exit(1);
    }
    cout << asctime(&mytm) << '\n';
    time_t epoch = mktime(&mytm);
    cout << "epoch = " << epoch << '\n';
    struct tm newtm;
    time_t new_epoch = epoch - 60;
    localtime_r(&new_epoch, &newtm);
    cout << "asctime(newtm) = " << asctime(&newtm) << '\n';
    
}
