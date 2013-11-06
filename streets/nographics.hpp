#define TEXT 0
#define G640x480x2 0

void vga_drawline( int, int, int, int );
void vga_drawpixel( int, int );
void vga_setcolor( int );
void vga_setmode( int );
void vga_init();
