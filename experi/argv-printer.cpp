#include <iostream>
using std::cout;

int main(int argc, char** argv)
{
    for (int i=0;i<argc;++i) {
        cout << argv[i] << " ";
    }
    cout << '\n';
}
