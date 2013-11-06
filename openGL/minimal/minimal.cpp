#include <vector>
#include <string>
#include <list>

void Display();
void Reshape( int w, int h );


class Drawing
{
  public:
    Drawing( int argc, char * * argv );
    void Clean();
    void Redraw() const;
    void Reshape( int, int ) const;

  private:
    void DrawTurtle() const;
    int        m_windowSize;
    float      m_turtleMatrix[16];
};


#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <unistd.h>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <string>

using std::string;
using std::vector;
using std::cerr;
using std::cout;
using std::cin;
using std::endl;

static Drawing * s_drawing;

Drawing::Drawing( int argc, char * * argv )
  : m_windowSize( 500 )
{
    glutInit( &argc, argv );
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize( m_windowSize, m_windowSize );
    glutInitWindowPosition(
        int(m_windowSize*0.5 + 0.5),
        int(m_windowSize*0.5 + 0.5) );
    glutCreateWindow(argv[0]);
    glShadeModel( GL_FLAT );

    // Initialize the turtle's homogeneous coordinates.
    glMatrixMode( GL_MODELVIEW_MATRIX );
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( 0.0, 0.0, 0.2 );
    glGetFloatv( GL_MODELVIEW_MATRIX, m_turtleMatrix );
    glPopMatrix();

    glutDisplayFunc(Display);
    glutReshapeFunc(::Reshape);
}


void Reshape( int w, int h )
{
    s_drawing->Reshape( w, h );
}

void
Drawing::Reshape( int w, int h ) const
{
    glViewport(0,0, (GLsizei)w, (GLsizei)h );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 15, 1.0, 0.02, 10.0 );
    glMatrixMode(GL_MODELVIEW);
}


void
Drawing::Redraw() const
{
//    glEnable(GL_LIGHTING);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(0.0,0.0,1.,  0.0,0.0,0,  0,1,0);

    this->DrawTurtle();

    glutSwapBuffers();
}


void
Drawing::DrawTurtle() const
{
    float size( 0.015 );
    glColor3f(1,1,0);
    GLfloat white_light[] = {0,0,1,1};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);


    glPushMatrix(); // Cuz we have to xform relative to where the mouse has
                    // moved the scene.
    glMultMatrixf( m_turtleMatrix );

    glRectf(-0.75f,0.75f, 0.75f, -0.75f);

/*
    glBegin(GL_POLYGON);
    glVertex3f( -0.5 * size, 0.0,        0.0 );
    glVertex3f(  0.5 * size, 0.0,        0.0 );
    glVertex3f(  0.0,        -2 * size,   0.0 );
    glEnd();
*/
    glPopMatrix();
}


/** Erase all turtle lines. */
void
Drawing::Clean()
{
}

void Display()
{
    s_drawing->Redraw();
}


int main( int argc, char * * argv )
{
    glClearColor(1.,1.,1.,0.);
    s_drawing = new Drawing( argc, argv );
    glutMainLoop();

    delete s_drawing;
    return 0;
}
