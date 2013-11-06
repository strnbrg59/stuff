


Xgraphics :
	TARGET = streets_Xgraphics
	GRAPHICS = Xgraphics
	OBJECTS = $(OBJECTS) vga2X.o Xwrapper.o
	make -f testme.mak $(TARGET);
vgagraphics :
	TARGET = streets_vgagraphics
	GRAPHICS = vgagraphics
	OBJECTS = $(OBJECTS) 
	make -f testme.mak $(TARGET);
nographics : 
	TARGET = streets_nographics
	GRAPHICS = nographics
	OBJECTS	= $(OBJECTS) nographics.o
	make -f testme.mak $(TARGET);

streets_Xgraphics : $(OBJECTS) streets.hpp
	g++ -o streets_Xgraphics $(OBJECTS) $(RPNPROG)/libX/rpnprogX.a $(RPNPROG)/lib/rpnprog.a /usr/X11R6/lib/libX11.so  -lm
streets_vgagraphics : $(OBJECTS) streets.hpp
	g++ -o streets_vgagraphics $(OBJECTS) $(RPNPROG)/lib/rpnprog.a -lm -lvga
streets_nographics : $(OBJECTS) streets.hpp
	g++ -o streets_nographics $(OBJECTS) $(RPNPROG)/lib/rpnprog.a -lm
clean :
	rm *.o *~ 

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