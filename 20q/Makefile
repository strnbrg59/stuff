HEADERS = btree.hpp
OBJECTS = main.o btree.o

20q : $(OBJECTS)
	g++ $(CXXFLAGS) -o $@ $^

%.o : %.cpp $(HEADERS)
	g++ -c $(CXXFLAGS) -o $@ $<

clean:
	rm -f *.o *~ 20q btree
