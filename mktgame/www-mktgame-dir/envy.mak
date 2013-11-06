CFLAGS = -c -g -Wall -I/usr/include/tcl

OBJECTS = envy.o Tcl_ErrnoMsg.o

envy : $(OBJECTS)
	g++ -o envy $(OBJECTS) /usr/lib/libexpect /usr/lib/libtcl.so.1 

clean :
	rm envy; rm envy.o; rm *~;

envy.o : envy.cpp
	g++ $(CFLAGS) envy.cpp
Tcl_ErrnoMsg.o : $(HOME)/src/expect/C/Tcl_ErrnoMsg.c
	gcc -c $(HOME)/src/expect/C/Tcl_ErrnoMsg.c