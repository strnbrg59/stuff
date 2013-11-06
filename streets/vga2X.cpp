// X functions to replace functionality of vgalib calls we're making.
// There really aren't alot of them:
//	vga_drawline
//	vga_drawpixel
//	vga_init
//	vga_setcolor
//	vga_setmode

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include "streets.hpp"

void vga_drawline(int x1, int y1, int x2, int y2 ) {
	XDrawLine( GXstreets::display, GXstreets::win, GXstreets::gc,
		x1,y1,x2,y2 );
}

void vga_drawpixel( int x, int y ) {
	XDrawPoint( GXstreets::display, GXstreets::win, GXstreets::gc,
		x,y );
}

void vga_setcolor( int color ) {
	XSetForeground(GXstreets::display, *GXstreets::gc, color);
}

void vga_setmode( int ) {}
void vga_init() {}


