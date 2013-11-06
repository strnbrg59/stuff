X11_HOME=/usr/X11R6
OPENGL_HOME=/usr/X11R6

CPPFLAGS = -Wall -c -g -I$(X11_HOME)/include -I$(OPENGL_HOME)/include
LDFLAGS = -L$(X11_HOME)/lib -L$(OPENGL_HOME)/lib

CC = g++
%.o : %.cpp %.hpp
	$(CC) $(CPPFLAGS) $<

OFILES = threed.o

threed : $(OFILES)
	$(CC) $(LDFLAGS) -o threed $(OFILES) -lGL -lGLU -lglut -lXmu

clean :
	-rm *.o threed core.* *.core
