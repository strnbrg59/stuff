#include <cstdlib>
#include <iostream>
using namespace std;

#define identify_status(symbolic, status) cout << #symbolic << " : " << \
    symbolic(status) << '\n';

int main()
{
    int status = 3072;
    identify_status(WIFEXITED, status);
    identify_status(WEXITSTATUS, status);
    identify_status(WIFSIGNALED, status);
    identify_status(WTERMSIG, status);
    identify_status(WIFSTOPPED, status);
    identify_status(WSTOPSIG, status);
    identify_status(WIFCONTINUED, status);
}

