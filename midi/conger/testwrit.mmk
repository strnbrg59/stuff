# testwrit.mmk  Microsoft Make file for testwrit.c

chain.obj:	chain.c
    msc chain;

writscrn.obj:	writscrn.c
    msc writscrn;

testwrit.obj:	testwrit.c
    msc testwrit;

testwrit.exe:	testwrit.obj chain.obj writscrn.obj
    link testwrit+chain+writscrn,,,bioscall;


