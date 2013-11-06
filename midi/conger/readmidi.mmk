# readmidi.mmk  Microsoft Make file for readmidi.c

readmidi.obj:	readmidi.c
    msc readmidi;

readmidi.exe:	readmidi.obj
    link readmidi+io401;


