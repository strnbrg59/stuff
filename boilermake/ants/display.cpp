#include "field.hpp"
#include "display.hpp"
#include "antutils.hpp"
#include "cmdline.hpp"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <unistd.h>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <string>
#include <vector>
#include <cmath>
#include <string>
#include <list>
#include <time.h>
using namespace std;


static int s_windowSize = 500;
#define BACKGROUND_GREY .29

long long g_iter = 0;
static long long g_maxIters = -300;

void IdleFunc()
{
    FieldFactory::TheField().Reposition();
    glutPostRedisplay();
    fsleep(CmdlineFactory::TheCmdline().Delay()/1000.0);

    if( ++g_iter == g_maxIters )
    {
        exit(0);
    }
}


void DrawCircle(double xctr, double yctr, double radius)
{
    double vectorX,vectorY;
    double const NU_ANGLESTEP=M_PI/12;
    glBegin(GL_LINE_STRIP);
    for(double angle=0; angle < 2.0*M_PI + NU_ANGLESTEP; angle+= NU_ANGLESTEP)
    {
        vectorX= xctr + radius * cos(angle);
        vectorY= yctr + radius * sin(angle);        
        glVertex3d(vectorX,vectorY,0);
    }
    glEnd();
    glFlush();
}

void InitGL(int argc, char** argv)
{
    glutInit( &argc, argv );
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(s_windowSize, s_windowSize);
    glutInitWindowPosition(
        int(s_windowSize*0.5 + 0.5),
        int(s_windowSize*0.5 + 0.5) );
    glutCreateWindow(argv[0]);
    glShadeModel( GL_FLAT );
    glClearColor(BACKGROUND_GREY,BACKGROUND_GREY,BACKGROUND_GREY,0.0);

    glutDisplayFunc(Display);
    glutReshapeFunc(::Reshape);
    glutIdleFunc(IdleFunc);
}


void Reshape(int w, int h)
{
    glViewport(0,0, (GLsizei)w, (GLsizei)h );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 15, 1.0, 0.02, 10.0 );
    glMatrixMode(GL_MODELVIEW);
}


void Display()
{
    bool showPheromone(CmdlineFactory::TheCmdline().ShowPheromone());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0,0.0,1.,  0.0,0.0,0,  0,1,0);
    Field& field(FieldFactory::TheField());
    DrawFieldBoundary();

    for(coord_t x=0; x<Field::rank; ++x)
    {
        for(coord_t y=0; y<Field::rank; ++y)
        {
            if(field[x][y].amtPheromone > 0)
            {
                field[x][y].DecayPheromone();
                if(showPheromone && (field[x][y].amtPheromone > 0))
                {
                    DrawPheromone(x, y, field[x][y].amtPheromone);
                }
            }
            assert(field[x][y].nAnts >= 0);
            if(field[x][y].GetAmtFood() > 0)
            {
                if(Coords(x,y) == field.GetHome())
                {
                    DrawNest(field[x][y].GetAmtFood());
                } else
                {
                    DrawFood(x,y);
                }
            }
            if(field[x][y].nAnts > 0)
            {
                DrawAnt(x,y, (field[x][y].nLoadedAnts > 0));
                assert(field[x][y].nLoadedAnts >= 0);
            }
        }
    }

    glutSwapBuffers();
}

static double s_stretchFactor = 4.0;
static double s_shift = 0.125;

void DrawFieldBoundary()
{
    glColor3f(1,1,1);
    double dy = s_shift;
    double dx = s_shift;
    double stretcher = 1/(Field::rank*s_stretchFactor);

    glBegin(GL_LINE_LOOP);
    glVertex3f(stretcher*0 - dx, stretcher*0 - dy, 0.0);
    glVertex3f(stretcher*0 - dx, stretcher*Field::rank - dy, 0.0);
    glVertex3f(stretcher*Field::rank - dx, stretcher*Field::rank - dy, 0.0);
    glVertex3f(stretcher*Field::rank - dx, stretcher*0 - dy, 0.0);
    glEnd();
}


void DrawNest(unsigned int amtFood)
{
    glColor3f(1,0,0);

    double radius = pow(amtFood, 0.5);
    Field& field(FieldFactory::TheField());
    double x = field.GetHome().x;
    double y = field.GetHome().y;

    double dy = s_shift;
    double dx = s_shift;
    double stretcher = 1/(Field::rank*s_stretchFactor);

    DrawCircle(stretcher*x-dx, stretcher*y-dy, stretcher*radius);
}


void DrawPixel(coord_t x, coord_t y, float r, float g, float b)
{
    glColor3f(r,g,b);

    double dy = s_shift;
    double dx = s_shift;
    double stretcher = 1/(Field::rank*s_stretchFactor);

    glBegin(GL_POINTS);
    glVertex3f(stretcher*x-dx, stretcher*y-dy, 0.0 );
    glEnd();
}

void DrawAnt(coord_t x, coord_t y, bool loaded)
{
    if(loaded)  DrawPixel(x, y, 1, 0, 0);
    else        DrawPixel(x, y, 1, 1, 0);
}

void DrawFood(coord_t x, coord_t y)
{
    DrawPixel(x, y, 1, 0, 0);
}


void DrawPheromone(coord_t x, coord_t y, pheromone_t amt)
{
    assert(amt >= 1);
    if(amt > 1)
    {
        DrawPixel(x, y, 0, amt/(255.0), 0);
    } else
    {
        DrawPixel(x, y, BACKGROUND_GREY, BACKGROUND_GREY, BACKGROUND_GREY);
    }
}


void DisplayMain(int argc, char * * argv)
{
    InitGL(argc, argv);
    glutMainLoop();
}
