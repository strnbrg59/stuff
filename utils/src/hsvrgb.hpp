struct rgbtype
{
    // r, g and b are on [0,1].
    rgbtype() {}
    rgbtype(double _r, double _g, double _b) : r(_r), g(_g), b(_b) {}
    double r;
    double g;
    double b;
};

struct  hsvtype
{
    // h in [0,6], s and v in [0,1].
    hsvtype() { }
    hsvtype(double _h, double _s, double _v) : h(_h), s(_s), v(_v) {}
    double h; double s; double v;
};

void hsv2rgb(hsvtype const& hsv, rgbtype& rgb);
