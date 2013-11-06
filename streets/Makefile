# Usage: at the command line, type either "make GRAPHICS=Xgraphics", 
#	"make GRAPHICS=vgagraphics", or "make GRAPHICS=nographics".
#	This clumsiness is necessary because I couldn't get the following
#	more appealing style to work: type "make Xgraphics", and the makefile
#	has
#		Xgraphics :
#			GRAPHICS = Xgraphics;\
#			OBJECTS:=$(OBJECTS) vga2X.o Xwrapper.o
#	and so forth.  The problem was that I couldn't get these definitions to
#	be "recognized", if I put them in the command list under a make condition
#	(i.e. "Xgraphics :" ).
#
# 	Type "make clean" if you want to make a different version (i.e. if
#	the .o files lying around are for Xgraphics and you want to go to
#	vgagraphics).

RPNPROG = $(HOME)/src/rpnprog
TEDSLIB = $(HOME)/src/util
DEBUGGING = debuggingoff

OBJECTS := main.o node.o car.o ctrlvar.o statevar.o util.o init-statics.o tedsutil.o
CFLAGS = -c -g -Wall -D$(GRAPHICS) -D$(DEBUGGING) -I$(RPNPROG) 

ifeq (Xgraphics,${GRAPHICS})
OBJECTS := $(OBJECTS) vga2X.o Xwrapper.o 
endif

ifeq (nographics,${GRAPHICS})
OBJECTS := $(OBJECTS) nographics.o
endif

ifeq (vgagraphics,${GRAPHICS})
OBJECTS := $(OBJECTS)
endif

streets : $(OBJECTS) streets.hpp
	g++ -o streets_$(GRAPHICS) $(OBJECTS) $(RPNPROG)/libX/rpnprogX.a $(RPNPROG)/lib/rpnprog.a /usr/X11R6/lib/libX11.so  -lm -lvga

clean :
	rm *.o *~ 
spotless : 
	rm *.o *~ streets_Xgraphics streets_vgagraphics streets_nographics

Xwrapper.o : Xwrapper.cpp streets.hpp
	g++ $(CFLAGS) Xwrapper.cpp
car.o : car.cpp streets.hpp
	g++ $(CFLAGS) car.cpp
node.o : node.cpp streets.hpp
	g++ $(CFLAGS) node.cpp
main.o : main.cpp streets.hpp 
	g++ $(CFLAGS) main.cpp 
statevar.o : statevar.cpp streets.hpp
	g++ $(CFLAGS) statevar.cpp
ctrlvar.o : ctrlvar.cpp streets.hpp
	g++ $(CFLAGS) ctrlvar.cpp
util.o : util.cpp streets.hpp
	g++ $(CFLAGS) util.cpp
classes.o : classes.cpp streets.hpp
	g++ $(CFLAGS) classes.cpp
init-statics.o : init-statics.cpp streets.hpp
	g++ $(CFLAGS) init-statics.cpp
nographics.o : nographics.cpp
	g++ $(CFLAGS) nographics.cpp
tedsutil.o : $(TEDSLIB)/tedsutil.cpp $(TEDSLIB)/tedsutil.hpp
	g++ $(CFLAGS) $(TEDSLIB)/tedsutil.cpp
vga2X.o : vga2X.cpp 
	g++ $(CFLAGS) vga2X.cpp