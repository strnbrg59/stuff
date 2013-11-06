#ifndef INCLUDED_MOUSE_HPP
#define INCLUDED_MOUSE_HPP

struct Mouse
{
    Mouse() : lastX(0), lastY(0), lastZ(0), lastRotX(0), lastRotY(0),
              transX(0.0), transY(0.0), transZ(0.0),
              rotX(0.0), rotY(0.0),
              focalDistance(6.0)
    {
        button[0] = button[1] = button[2] = false;
    }

    void MousePress( int button_, int state, int x, int y );
    void MouseMotion( int x, int y );    

    int lastX, lastY, lastZ, lastRotX, lastRotY;
    bool button[3]; // Indicates whether it's currently depressed
    float transX, transY, transZ;
    float rotX, rotY;
    float focalDistance;
};

Mouse * TheMouse();
void MousePress( int button, int state, int x, int y );
void MouseMotion( int x, int y );

#endif
