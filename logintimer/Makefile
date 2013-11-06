CXXFLAGS = -g -c -I/usr/include/qt4
LDFLAGS = -lQtGui

OBJECTS = analogclock.o budget.o main.o

%.o : %.cpp analogclock.h budget.h
	g++ $(CXXFLAGS) $< -o $@

madzman : $(OBJECTS)
	g++ $(OBJECTS) $(LDFLAGS) -o $@

clean :
	rm -f *~ *.o madzman
