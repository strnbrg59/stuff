#include "fingers.hpp"
#include "cmdline.hpp"
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char** argv)
{
    Cmdline cmdline(argc, argv);
    CmdlineFactory::Init(cmdline);

    Infile infile(cmdline.Infile());

    Tree tree(infile);
    tree.Grow();
}
