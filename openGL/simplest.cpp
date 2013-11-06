#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(500,500);
    glutCreateWindow("elevations");

    glClearColor(0.,0.,0.,0.);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.,1.,1.);
    glOrtho(0.,1.,0.,1.,-1.,1.);
    glBegin(GL_POLYGON);
    glVertex3f(.25,.25,0.);
    glVertex3f(.75,.25,0.);
    glVertex3f(.75,.75,0.);
    glVertex3f(.25,.75,0.);
    glEnd();
    
    glutSwapBuffers();
    glutMainLoop();
}
