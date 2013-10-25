C_OBJS = c.o
C_LIB = libcstruct.so
CPP_OBJS = cc.o
CPP_LIB = libcstruct_cpp.so

CFLAGS = -fPIC
CXXFLAGS = -fPIC

prog : $(C_LIB) $(CPP_LIB) main.o
	gcc $^ -o $@

$(C_LIB) : $(C_OBJS)
	gcc -shared -Wl,-soname,$@ -o $@ $^

$(CPP_LIB) : $(CPP_OBJS)
	g++ -shared -Wl,-soname,$@ -o $@ $^

clean :
	rm -f *.o *.so *~
