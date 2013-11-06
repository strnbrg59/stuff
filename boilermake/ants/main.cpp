#include "display.hpp"
#include "ant.hpp"
#include "field.hpp"
#include "antutils.hpp"
#include "cmdline.hpp"
#include <iostream>
using std::cout;

int main(int argc, char** argv)
{
    Cmdline cmdline(argc, argv);
    CmdlineFactory::Init(cmdline);
    FieldFactory::Init();
    AntFactory::Init();

    srand(CmdlineFactory::TheCmdline().Seed());

    DisplayMain(argc, argv);
}
