X11_HOME=/usr/X11R6
#OPENGL_HOME=$(HOME)/usr/
OPENGL_HOME=/usr/X11R6

CPPFLAGS = -Wall -c -g -I$(OPENGL_HOME)/include -I$(X11_HOME)/include
LDFLAGS = -L$(OPENGL_HOME)/lib -L$(X11_HOME)/lib 

CC = g++
%.o : %.cpp %.hpp
	$(CC) $(CPPFLAGS) $<

OFILES = simplest.o

simplest : $(OFILES)
	$(CC) $(LDFLAGS) -o simplest $(OFILES) -lGL -lGLU -lglut -lXmu

clean :
	-rm *.o simplest core.* *.core *~
