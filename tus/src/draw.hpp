#ifndef INCLUDED_DRAW_HPP
#define INCLUDED_DRAW_HPP

#include <GL/gl.h>
#include "IndexTM.hpp"

typedef IndexTM<GLfloat,3> GL3Vect;

/** First thing to call from main() */
void Init(int argc, char** argv, int windowSize);

/** Callback to glDisplayFunc(), and below it some helper functions */
void Display();
void SetView();
void DrawScenery();

/** Callback to glReshapeFunc() */
void Reshape( int w, int h );

#endif // include guard
