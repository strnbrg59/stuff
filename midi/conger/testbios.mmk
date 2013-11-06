# testbios.mmk  Microsoft Make file for testbios.c

testbios.obj:	testbios.c
    msc testbios;

testbios.exe:	testbios.obj
    link testbios,,,bioscall;


