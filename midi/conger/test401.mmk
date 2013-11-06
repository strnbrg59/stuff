# test401.mmk  Microsoft Make file for test401.c

test401.obj:	test401.c
    msc test401;

test401.exe:	test401.obj
    link test401+io401;


