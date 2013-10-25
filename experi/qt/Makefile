CXXFLAGS = -g -c -I/usr/include/QtGui
LDFLAGS = -lQtGui
OBJECTS = analogclock.o main.o

%.o : %.cpp analogclock.h
	g++ $(CXXFLAGS) $< -o $@

a.out : $(OBJECTS)
	g++ $(OBJECTS) $(LDFLAGS) -o $@
