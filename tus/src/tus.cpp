#include <GL/glut.h>
#include "draw.hpp"
#include "cmdline.hpp"

Cmdline* g_cmdline;

int main( int argc, char * * argv )
{
    g_cmdline = new Cmdline(argc, argv);
    Init(argc, argv, 700);
    glutMainLoop();
    delete g_cmdline;
}
