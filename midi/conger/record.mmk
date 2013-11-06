# make file for record.c

record.obj:	record.c
    msc record;

inputf.obj:	inputf.c
    msc inputf;

chain.obj:	chain.c
    msc chain;

writscrn.obj:	writscrn.c
    msc writscrn;

record.exe:	record.obj inputf.obj 
    link record+inputf+io401+writscrn+chain,,,bioscall;


