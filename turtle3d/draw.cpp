// Author: Ted Sternberg, strnbrg59@gmail.com

#include "draw.hpp"
#include "mouse.hpp"
#include "reader.hpp"

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

/** Registered with glutIdleFunc().
 *  Reads logo commands on stdin (commands that come from parent process).
*/
void IdleFunc()
{
    vector<string> commands( TheReader()->ReadCommands() );
    for( unsigned int c=0; c<commands.size(); ++c )
    {
        char * cmd = new char[ commands[c].size()+1 ];
        strcpy( cmd, commands[c].c_str() );
        s_drawing->Interpreter( TokenizeCommand( cmd ) );
        delete [] cmd;
    }
}


void
Drawing::Interpreter( vector<string> const & cmd_words )
{
    if( cmd_words[0] == "fd" )
    {
        this->Forward( atof(cmd_words[1].c_str()) );
    } else
    if( cmd_words[0] == "bk" )
    {
        this->Forward( -atof(cmd_words[1].c_str()) );
    } else
    if( cmd_words[0] == "sfd" )
    {
        this->SphereForward( atof(cmd_words[1].c_str()) );
    } else
    if( cmd_words[0] == "sbk" )
    {
        this->SphereForward( -atof(cmd_words[1].c_str()) );
    } else
    if( cmd_words[0] == "rt" )
    {
        this->Right( atof(cmd_words[1].c_str()) );
    } else
    if( cmd_words[0] == "lt" )
    {
        this->Right( -atof(cmd_words[1].c_str()) );
    } else
    if( cmd_words[0] == "up" )
    {
        this->Up( atof(cmd_words[1].c_str()) );
    } else
    if( cmd_words[0] == "dn" )
    {
        this->Up( -atof(cmd_words[1].c_str()) );
    } else
    if( cmd_words[0] == "rl" )
    {
        this->Roll( -atof(cmd_words[1].c_str()) );
    } else
    if( cmd_words[0] == "ht" )
    {
        this->SetTurtleVisible( false );
    } else
    if( cmd_words[0] == "st" )
    {
        this->SetTurtleVisible( true );
    } else
    if( cmd_words[0] == "pd" )
    {
        this->PenDown();
    } else
    if( cmd_words[0] == "pu" )
    {
        this->PenUp();
    } else
    if( cmd_words[0] == "setpencolor" )
    {
        if( cmd_words.size() == 4 )
        {
            this->SetPenColor( atof(cmd_words[1].c_str()),
                               atof(cmd_words[2].c_str()),
                               atof(cmd_words[3].c_str()) );
        } else
        if( cmd_words.size() == 2 )
        { // Doing this because it's hard to do 3-arg functions in popenwrite().
            int encodedColor = atoi(cmd_words[1].c_str());
            float rgb[3];
            for( int i=2;i>=0;--i )
            {
                rgb[i] = (encodedColor % 256) / 255.0;
                encodedColor /= 255;
            }
            this->SetPenColor( rgb[0], rgb[1], rgb[2] );
        } else
        {
            cerr << "size() neither 4 nor 2!" << endl;
        }
    } else
    if( cmd_words[0] == "clean" )
    {
        this->Clean();
    } else
    if( cmd_words[0] == "sphere" )
    {
        if(      cmd_words[1] == "1" )  this->ShowSphere(true);
        else if( cmd_words[1] == "0" ) this->ShowSphere(false);
        else
        {
            cerr << "Error!  Usage: \"sphere 1\" or \"sphere 0\"" << endl;
            return;
        }
    } else
    if( cmd_words[0] == "lighting" )
    {
        if(      cmd_words[1] == "1" )  this->SetLighting(true);
        else if( cmd_words[1] == "0" ) this->SetLighting(false);
        else
        {
            cerr << "Error!  Usage: \"lighting 1\" or \"lighting 0\"" <<endl;
            return;
        }
    }
    if( cmd_words[0] == "bye" )
    {
        exit(0);
    }

    glutPostRedisplay();

}


void InitLighting()
{
    GLfloat mat_specular[] = {0.9,0.9,0.9,1};
    GLfloat mat_shininess[] = {50.0};
    GLfloat light_position[] = {0.5,0.3,1,0};
    GLfloat white_light[] = {0,0,1,1};
    GLfloat lmodel_ambient[] = {0.4,0.4,0.4,1.0 };

    glClearColor(0.1,0.1,0.1, 0);

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


void
Drawing::SetLighting( bool b )
{
    m_lighting = b;
    if( b )
    {
        InitLighting();
    } else
    {
        glDisable(GL_LIGHTING);
        glClearColor(0,0,0,0);
    }
    this->Redraw();
}


Drawing::Drawing( int argc, char * * argv )
  : m_windowSize( 500 ),
    m_turtleVisible( true ),
    m_lighting( false ),
    m_sphereVisible( false ),
    m_sphereRadius( 100.0 )
{
    glutInit( &argc, argv );
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize( m_windowSize, m_windowSize );
    glutInitWindowPosition(
        int(m_windowSize*0.5 + 0.5),
        int(m_windowSize*0.5 + 0.5) );
    glutCreateWindow(argv[0]);
//  InitFog();
    glShadeModel( GL_FLAT );

    // antialiasing (doesn't work!)
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

    // Initialize the turtle's homogeneous coordinates.
    glMatrixMode( GL_MODELVIEW_MATRIX );
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( 0.0, 0.0, 0.2 );
    glGetFloatv( GL_MODELVIEW_MATRIX, m_turtleMatrix );
    glPopMatrix();

    glutDisplayFunc(Display);
    glutIdleFunc(IdleFunc);
    glutReshapeFunc(::Reshape);
    glutMouseFunc(MousePress);
    glutMotionFunc(MouseMotion);
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


void PrintMatrix( float const m[16] )
{
    for( int i=0;i<4;++i )
    {
        for( int j=0;j<4;++j )
        {
            cout << m[4*i+j] << " ";
        }
        cout << endl;
    }
}

void PrintModelviewMatrix()
{
    float buf[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, buf);
    PrintMatrix( buf );
}

void
Drawing::Redraw() const
{
    glDisable(GL_LIGHTING);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(0.0,0.0,TheMouse()->focalDistance,  0.0,0.0,0,  0,1,0);

    glTranslatef( TheMouse()->transX, TheMouse()->transY, TheMouse()->transZ );
    glRotatef( TheMouse()->rotX, 0,1,0 );
    glRotatef( TheMouse()->rotY, 1,0,0 );

    if( m_nodes.empty() )
    {
        cerr << "nodes empty" << endl;
        return;
    }

    bool penIsDown(false);

    for( EventList::const_iterator i = m_nodes.begin();
         i != m_nodes.end();
         ++i )
    {
        if( ( (i->modifier == Event::pd) && (penIsDown) )
        ||  ( (i->modifier == Event::pu) && (!penIsDown) ) )
        {   // no-ops
            continue;  
        }

        if(i->modifier == Event::pd)
        {        
            penIsDown = true;
            glBegin(GL_LINE_STRIP);
            glVertex3f( i->position.x, i->position.y, i->position.z );
        } else
        if(i->modifier == Event::pu)
        {
            penIsDown = false;
            glEnd();
        } else
        if( penIsDown )
        {
            if( i->modifier >= 0 ) // new color
            {
                int encodedColor = i->modifier;
                float rgb[3];
                for( int i=2;i>=0;--i )
                {
                    rgb[i] = (encodedColor % 256) / 255.0;
                    encodedColor /= 256;
                }
                glColor3fv( rgb );
            } else
            {
                glVertex3f( i->position.x, i->position.y, i->position.z );
            }
        }
    }
    if( penIsDown )
    {
        glEnd();
    }


    if( m_turtleVisible )
    {
        this->DrawTurtle();
    }

    if( m_sphereVisible )
    {
        DrawSphere( m_lighting );
    }

    glutSwapBuffers();
}


void
DrawSphere( bool withLighting )
{
    if( withLighting )
    {
        glEnable(GL_LIGHTING);
        GLfloat white_light[] = {0,0,1,1};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
        glutSolidSphere(0.1999, 100, 100 );
    } else
    {
        glColor3f( 0,0,1 );
        glutWireSphere(0.1999, 10, 10 );
    }
}


void
Drawing::DrawTurtle() const
{
    float size( 0.015 );

    glPushMatrix(); // Cuz we have to xform relative to where the mouse has
                    // moved the scene.
    glMultMatrixf( m_turtleMatrix );

    glBegin(GL_POLYGON);
    glVertex3f( -0.5 * size, 0.0,        0.0 );
    glVertex3f(  0.5 * size, 0.0,        0.0 );
    glVertex3f(  0.0,        2 * size,   0.0 );
    glEnd();

    glPopMatrix();
}


void
Drawing::PenUp()
{
    m_nodes.push_back(
        Event( m_turtleMatrix[12], m_turtleMatrix[13], m_turtleMatrix[14],
               Event::pu ) );
}


void
Drawing::PenDown()
{
    m_nodes.push_back(
        Event( m_turtleMatrix[12], m_turtleMatrix[13], m_turtleMatrix[14],
               Event::pd ) );
}


/** Erase all turtle lines. */
void
Drawing::Clean()
{
    EventList emptyList;
    m_nodes = emptyList;
    this->PenDown(); // Initialization
    this->Forward( 0 ); // Initialization
}

void
Drawing::Goto( float x, float y, float z )
{
    m_nodes.push_back( Event( x,y,z, Event::nothing ) );
}


void
Drawing::Forward( float x )
{
    float scaled_x = (x+0.0)/m_windowSize;

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadMatrixf( m_turtleMatrix );
    glTranslatef( 0, scaled_x, 0 ); 
        // Moves in direction of rotated coordinate system
    glGetFloatv( GL_MODELVIEW_MATRIX, m_turtleMatrix );
    glPopMatrix();

    this->Goto( m_turtleMatrix[12], m_turtleMatrix[13], m_turtleMatrix[14] );
}


void
Drawing::SphereForward( float degrees )
{
    float const degreesPerRadian( 180 / M_PI );
    float radians = degrees /degreesPerRadian;

    // Break it up into a bunch of small steps, so it really looks like we're
    // walking on the sphere.
    int n = std::max( int( fabs(radians) / (2*M_PI/360.0) ), 1 );
    float deltaRadians = radians / (n+0.0);
    float a = m_sphereRadius / cos(deltaRadians);
    float b = m_sphereRadius * tan(deltaRadians);
    for( int i=0;i<n;++i )
    {
        this->Forward( b );
        this->Up( -deltaRadians*degreesPerRadian - 90 );
        this->Forward( a - m_sphereRadius );
        this->Up( 90 );
    }
}
    

void
Drawing::Right( float degrees )
{
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadMatrixf( m_turtleMatrix );
    glRotatef( -degrees, 0,0,1 );
    glGetFloatv( GL_MODELVIEW_MATRIX, m_turtleMatrix );
    glPopMatrix();
}


void
Drawing::Up( float degrees )
{
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadMatrixf( m_turtleMatrix );
    glRotatef( degrees, 1,0,0 );
    glGetFloatv( GL_MODELVIEW_MATRIX, m_turtleMatrix );
    glPopMatrix();
}


void
Drawing::Roll( float degrees )
{
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadMatrixf( m_turtleMatrix );
    glRotatef( degrees, 0,1,0 );
    glGetFloatv( GL_MODELVIEW_MATRIX, m_turtleMatrix );
    glPopMatrix();
}


void
Drawing::SetTurtleVisible( bool b )
{
    m_turtleVisible = b;
}


void
Drawing::SetPenColor( float r, float g, float b )
{
    if( !( (r>=0) && (r<=1) && (g>=0) && (g<=1) && (b>=0) && (b<=1) ) )
    {
        cerr << "Error: (r,g,b) should be fractions of 1.0" << endl;
        return;
    }
    //cerr << "SetPenColor(" << r << "," << g << "," << b << ")\n";
    m_nodes.push_back(
        Event( m_turtleMatrix[12], m_turtleMatrix[13], m_turtleMatrix[14],
               int(r*255)*256*256 + int(g*255)*256 + int(b*255) ));
}


void
Drawing::ShowSphere( bool b )
{
    m_sphereVisible = b;
}


void
Drawing::SetSphereRadius( float r )
{
    m_sphereRadius = r;
}


void Display()
{
    s_drawing->Redraw();
}


/** Return the white-space delimited words that comprise arg cmd. */
std::vector<std::string>
TokenizeCommand( char * buf )
{
    char * ptrptr;
    char * tok;
    char const delims[] = {" \t\n"};

    vector<string> result;
    tok = strtok_r( buf, delims, &ptrptr );
    while( tok )
    {
        result.push_back( string(tok) );
        tok = strtok_r( 0, delims, &ptrptr );
    }

    return result;
}


void InitFog()
{
    glEnable(GL_FOG);
    glHint(GL_FOG_HINT, GL_DONT_CARE);
//  glFogi(GL_FOG_MODE, GL_EXP);
//  glFogf(GL_FOG_DENSITY, 0.55);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 1.0 );
    glFogf(GL_FOG_END, 3.0 );
}


int main( int argc, char * * argv )
{
    s_drawing = new Drawing( argc, argv );

    s_drawing->PenDown();
    s_drawing->SetPenColor(1,1,1);
    s_drawing->Forward( 0 );

    glutMainLoop();

    delete s_drawing;
    return 0;
}
