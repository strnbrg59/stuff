#include "hsvrgb.hpp"
#include <cstdlib>

void set_rgb(struct rgbtype& rgb, double r, double g, double b)
{
    rgb.r = r; rgb.g = g; rgb.b = b;
}

void hsv2rgb(hsvtype const& hsv, rgbtype& rgb)
{
    double h = hsv.h, s = hsv.s, v = hsv.v, m, n, f;
    int i;
    
    i = int(h);
    f = h - i;
    if ( !(i&1) ) f = 1 - f; // if i is even
    m = v * (1 - s);
    n = v * (1 - s * f);
    switch (i) {
        case 6:
        case 0: set_rgb(rgb, v, n, m);
        case 1: set_rgb(rgb, n, v, m);
        case 2: set_rgb(rgb, m, v, n);
        case 3: set_rgb(rgb, m, n, v);
        case 4: set_rgb(rgb, n, m, v);
        case 5: set_rgb(rgb, v, m, n);
    }
}

#ifdef DO_DEMO
#include <iostream>
using namespace std;
int main(int argc, char** argv)
{
    rgbtype rgb;
    hsvtype hsv;
    char *endptr;
    hsv.h = strtod(argv[1], &endptr);
    hsv.s = strtod(argv[2], &endptr);
    hsv.v = strtod(argv[3], &endptr);
    cout << "hsv=(" << hsv.h << ", " << hsv.s << ", " << hsv.v << ")\n";

    hsv2rgb(hsv, rgb);
    cout << "(" << rgb.r << ", " << rgb.g << ", " << rgb.b << ")\n";
}
#endif // DO_DEMO
