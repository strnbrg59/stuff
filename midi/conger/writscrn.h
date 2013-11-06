/* writscrn.h  header file for writscrn.c */

struct selement {
    int xpos;
    int ypos;
    char content[15];
    int nup;
    int ndown;
    int nleft;
    int nright;
    int key;
};

