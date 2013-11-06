#include <GL/gl.h>
#include <iostream>
using namespace std;

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
