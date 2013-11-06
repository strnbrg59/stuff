// Author: Ted Sternberg, strnbrg@trhj.homeunix.net

#include "mouse.hpp"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

static Mouse * s_mouse(0);

Mouse * TheMouse()
{
    if( ! s_mouse )
    {
        s_mouse = new Mouse;
    }
    return s_mouse;
}

void MousePress( int button, int state, int x, int y )
{
    TheMouse()->MousePress( button, state, x, y );
}

void
Mouse::MousePress( int button_, int state, int x, int y )
{
    switch( button_ )
    {
        case GLUT_LEFT_BUTTON :
        {
            button[0] = true;
            button[1] = false;
            button[2] = false;
            lastRotX = x;
            lastRotY = y;
            break;
        }
        case GLUT_MIDDLE_BUTTON :
        {
            button[0] = false;
            button[1] = true;
            button[2] = false;
            lastX = x;
            lastY = y;
            break;
        }
        case GLUT_RIGHT_BUTTON :
        {
            button[0] = false;
            button[1] = false;
            button[2] = true;
            lastZ = y; // for zooming
            break;
        }
        default : break;
    }
}


void MouseMotion( int x, int y )
{
    TheMouse()->MouseMotion( x, y );
}

void
Mouse::MouseMotion( int x, int y )
{
    if( button[0] )
    {
        rotX += (x - lastRotX) * 360.0/500.0;
        rotY += (y - lastRotY) * 360.0/500.0;
        lastRotX = x;
        lastRotY = y;
    } else
    if( button[1] )
    {
        transX += focalDistance
                            * (x - lastX)/500.0;
        transY -= focalDistance
                            * (y - lastY)/500.0;
        lastX = x;
        lastY = y;
    } else
    if( button[2] )
    {
        transZ += focalDistance
                            * (y - lastZ)/500.0;
        lastZ = y;
    }
        
    glutPostRedisplay();
}    
