# testchan.mmk  Microsoft Make file for testchan.c

testchan.obj:	testchan.c
    msc testchan;

chain.obj:	chain.c
    msc chain;

testchan.exe:	testchan.obj
    link testchan+chain;


