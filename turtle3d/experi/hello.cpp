#include <GL/glut.h>
#include <stdlib.h>

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1,1,1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef( 45, 0,0,1 );

    glBegin(GL_POLYGON);
    glVertex3f(0.25, 0.25, 0.0);
    glVertex3f(0.75, 0.25, 0.0);
    glVertex3f(0.75, 0.75, 0.0);
    glVertex3f(0.25, 0.75, 0.0);

    glPopMatrix();
    glEnd();
    glFlush();
}

void init()
{
    glClearColor(0,0,0,0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,1,0,1,-1,1);
}

int main( int argc, char * * argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(250, 250);
    glutInitWindowPosition(100,100);
    glutCreateWindow("hello");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
