#patchlib.mmk - Microsoft Make file to compile and link patchlib.c 

patchut1.obj:	patchut1.c
    MSC patchut1;

patchut2.obj:	patchut2.c
    MSC patchut2;

patchprt.obj:	patchprt.c
    MSC patchprt;

writscrn.obj:	writscrn.c writscrn.h
    MSC writscrn;

patchlod.obj:	patchlod.c
    MSC patchlod;

patched.obj:	patched.c patched.h
    MSC patched;

chain.obj:	chain.c chain.h
    MSC  chain;

patchlib.obj:	patchlib.c patchlib.h
    MSC patchlib;

patchlib.exe:	patchlib.obj patchut1.obj patchut2.obj patchprt.obj io401.obj\
		patched.obj patchlod.obj writscrn.obj chain.obj 
    LINK patchlib+patchut1+patchut2+patchprt+patched+patchlod+writscrn+chain+io401,,,bioscall;


