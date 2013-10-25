#ifndef __INCLUDED_DISPLAY_HPP__
#define __INCLUDED_DISPLAY_HPP__

#include "field.hpp"

void DrawFieldBoundary();
void DrawAnt(coord_t x, coord_t y, bool loaded);
void DrawFood(coord_t x, coord_t y);
void DrawPheromone(coord_t x, coord_t y, pheromone_t amt);
void DrawNest(unsigned int amtFood);
void DrawPixel(coord_t x, coord_t y, float r, float g, float b);
void Display();
void Reshape(int w, int h);
void DisplayMain(int argc, char** argv);
void InitGL(int argc, char** argv);

#endif
