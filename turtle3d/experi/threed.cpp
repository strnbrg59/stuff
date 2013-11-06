#include <GL/glut.h>
#include <cstdlib>
#include <cstdio>

struct MouseState
{
    MouseState() : lastX(0), lastY(0), lastZ(0), lastRotX(0), lastRotY(0)
    {
        button[0] = button[1] = button[2] = false;
    }

    int lastX, lastY, lastZ, lastRotX, lastRotY;
    bool button[3]; // Indicates whether it's currently depressed
};

static MouseState s_mouseState;
static float s_transX, s_transY, s_transZ;
static float s_rotX, s_rotY;
static float s_focalDistance;


void InitLighting()
{
    GLfloat mat_specular[] = {0.6,0.6,0.6,1};
    GLfloat mat_shininess[] = {50.0};
    GLfloat light_position[] = {1,1,-1,0};
    GLfloat white_light[] = {0,0,1,1};
    GLfloat lmodel_ambient[] = {0.99,0.99,0.99,1.0 };

    glClearColor(1.0,1.0,0.6, 0);

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular );
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
}


void init()
{
    glClearColor(0,0,0,0);
    glShadeModel(GL_FLAT);

    s_transX = s_transY = 0.0;
    s_rotX = s_rotY = 0.0;
    s_focalDistance = 5;

    InitLighting();
}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1,1,1);
    glLoadIdentity();
    gluLookAt(0,0,s_focalDistance,  0,0,0,  0,1,0);
    glScalef(1,1,1);
    glTranslatef( s_transX, s_transY, s_transZ );
    glRotatef( s_rotX, 0,1,0 );
    glRotatef( s_rotY, 1,0,0 );

    glEnable(GL_LIGHTING);
    GLfloat white_light[] = {0,0,1,1};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light );
    glutSolidSphere(1, 40, 40 );

    glDisable(GL_LIGHTING);
    white_light[0] = 1; white_light[2] = 0;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glTranslatef( s_transX, s_transY, s_transZ );
    glutSolidCone(1.0, 2.0, 4, 1);

    glutSwapBuffers();
}


void reshape( int w, int h )
{
    glViewport(0,0, (GLsizei)w, (GLsizei)h );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1,1,-1,1,1.5,20.0);
    glMatrixMode(GL_MODELVIEW);
}


void mousePress( int button, int state, int x, int y )
{
    switch( button )
    {
        case GLUT_LEFT_BUTTON :
        {
            s_mouseState.button[0] = true;
            s_mouseState.button[1] = false;
            s_mouseState.button[2] = false;
            s_mouseState.lastRotX = x;
            s_mouseState.lastRotY = y;
            break;
        }
        case GLUT_MIDDLE_BUTTON :
        {
            s_mouseState.button[0] = false;
            s_mouseState.button[1] = true;
            s_mouseState.button[2] = false;
            s_mouseState.lastX = x;
            s_mouseState.lastY = y;
            break;
        }
        case GLUT_RIGHT_BUTTON :
        {
            s_mouseState.button[0] = false;
            s_mouseState.button[1] = false;
            s_mouseState.button[2] = true;
            s_mouseState.lastZ = y; // for zooming
            break;
        }
        default : break;
    }
}


void mouseMotion( int x, int y )
{
//  printf( "(x,y)=(%d,%d)\n", x,y );
    if( s_mouseState.button[0] )
    {
        s_rotX += (x - s_mouseState.lastRotX) * 360.0/500.0;
        s_rotY += (y - s_mouseState.lastRotY) * 360.0/500.0;
        s_mouseState.lastRotX = x;
        s_mouseState.lastRotY = y;
    } else
    if( s_mouseState.button[1] )
    {
        s_transX += s_focalDistance * (x - s_mouseState.lastX)/500.0;
        s_transY -= s_focalDistance * (y - s_mouseState.lastY)/500.0;
        s_mouseState.lastX = x;
        s_mouseState.lastY = y;
    } else
    if( s_mouseState.button[2] )
    {
        s_transZ += s_focalDistance * (y - s_mouseState.lastZ)/500.0;
        s_mouseState.lastZ = y;
    }
        
    glutPostRedisplay();
}    

int main(int argc, char * * argv )
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(500,500);
    glutInitWindowPosition(100,100);
    glutCreateWindow(argv[0]);
    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mousePress);
    glutMotionFunc(mouseMotion);

    glutMainLoop();
    return 0;
}
