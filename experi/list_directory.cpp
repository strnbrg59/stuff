#include <cstdio>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <string>
using std::cout;
using std::string;

int main(int argc, char** argv)
{
    struct dirent** namelist;
    string filepath;
    int ret;
    int count = scandir(argv[1], &namelist, 0, alphasort);
    for (int i=0; i<count; ++i) {
        filepath = string(argv[1]) + "/" + namelist[i]->d_name;
        if ((filepath != ".") && (filepath != "..")) {
            /*
            ret = unlink(filepath.c_str());
            if (ret) {
                perror(ret);
                exit(1);
            }
            */
            cout << filepath << "\n";
        }
        free(namelist[i]);
    }
    free(namelist);

    cout << '\n';
}
