CXXFLAGS = -g -O0

main: main.o delfunc.o
	g++ -g -O0 main.o delfunc.o -o main

delfunc.o : delfunc.cc c_auto_ptr.h delfunc.h
main.o : main.cc c_auto_ptr.h  delfunc.h

clean:
	rm -f main *.o *~
