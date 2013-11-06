// Author: Ted Sternberg, strnbrg59@gmail.com

#include "cmdline.hpp"
#include "draw.hpp"
#include "mouse.hpp"
#include "elevations.hpp"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <cassert>

GLuint g_sceneryDisplayListHandle;
extern Cmdline* g_cmdline;

void Init(int argc, char** argv, int windowSize)
{
    //
    // glut stuff
    //
    glutInit( &argc, argv );
    glutInitWindowSize( windowSize, windowSize );
    glutInitWindowPosition(
        int(windowSize*0.5 + 0.5),
        int(windowSize*0.5 + 0.5) );
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutCreateWindow(argv[0]);

    //glShadeModel(GL_SMOOTH);
    glShadeModel(GL_FLAT);

    //
    // Set up lighting
    //
    GLfloat mat_specular[] = {0.1,0.1,0.1,1};
    GLfloat mat_shininess[] = {1.0};
    GLfloat light_position[] = {0.5,0.3,1,1000};
    GLfloat white_light[] = {1,1,1,1};
    GLfloat lmodel_ambient[] = {1.,1.,1.,1.0 };

    glClearColor(1.,0.,0., 0.5);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular );
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    //
    // Register GL callbacks
    //
    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutMouseFunc(MousePress);
    glutMotionFunc(MouseMotion);

    //
    // Draw the scenery into a display list
    //
    g_sceneryDisplayListHandle = glGenLists(1);
    glNewList(g_sceneryDisplayListHandle, GL_COMPILE);
    DrawScenery();
    glEndList();

}


void Reshape( int w, int h )
{
    glViewport(0,0, (GLsizei)w, (GLsizei)h );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 15, 1.0, 0.02, 10.0 );
    glMatrixMode(GL_MODELVIEW);
}


void Display()
{
    SetView();
    glCallList(g_sceneryDisplayListHandle);
    glutSwapBuffers();
}


void
SetView()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(0.0,0.0,TheMouse()->focalDistance,  0.0,0.0,0,  0,1,0);

    glTranslatef( TheMouse()->transX, TheMouse()->transY, TheMouse()->transZ );
    glRotatef( TheMouse()->rotX, 0,1,0 );
    glRotatef( TheMouse()->rotY, 1,0,0 );
}


void DrawScenery()
{
    Elevations elevations;
    int gridSize = g_cmdline->GridSize();
    int coarsening = g_cmdline->Coarsening();
    assert(gridSize > 1);
    double scale = gridSize;
    bool water=false;

    for(int i=1; i<gridSize/coarsening; ++i)
    {
        GL3Vect point(9,9,9), point_1(8,8,8), point_2(7,7,7), normvect(0,0,0);

        glBegin(GL_TRIANGLE_STRIP);
        for(int j=0; j<gridSize*2/coarsening; ++j)
        {
            if(j%2 == 0)
            {
                point[0] = -(i-1)*coarsening/scale;
                point[1] = -(j/2)*coarsening/scale;
                point[2] = elevations.val(i-1, j/2)/(10*scale);
            } else
            {
                point[0] = -i*coarsening/scale;
                point[1] = -(j/2)*coarsening/scale;
                point[2] = elevations.val(i,j/2)/(10*scale);
            }

            if(j>3) // Don't render first triangle as we don't have normals for
                    // its vertices.  Don't render second triangle either or
                    // edge will look ragged.
            {
                GL3Vect::normal(point-point_1, point_1-point_2, normvect);
                if(j%2 == 0) normvect*=-1;

                // Make it blue if land is near sea level.
                if((elevations.val(i,j/2) < 1) && (!water))
                {
                    glEnd();
                    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,
                                   &GL3Vect(0.,0.,1.));
                    glBegin(GL_TRIANGLE_STRIP);
                    water = true;
                } else
                if((elevations.val(i,j/2) >=1) && water)
                {
                    glEnd();
                    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,
                                   &GL3Vect(1.,1.,1.));
                    glBegin(GL_TRIANGLE_STRIP);
                    water = false;
                }

                glNormal3fv(&normvect);
                glVertex3fv(&point);
            }
            point_2 = point_1;
            point_1 = point;
        }
        glEnd();
    }
}
