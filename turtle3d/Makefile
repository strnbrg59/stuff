# Author: Ted Sternberg, strnbrg@trhj.homeunix.net

X11_HOME=/usr/X11R6
OPENGL_HOME=$(HOME)/usr/
#OPENGL_HOME=/usr/X11R6

CPPFLAGS = -Wall -c -g -I$(OPENGL_HOME)/include -I$(X11_HOME)/include
LDFLAGS = -L$(OPENGL_HOME)/lib -L$(X11_HOME)/lib 

CC = g++
%.o : %.cpp %.hpp
	$(CC) $(CPPFLAGS) $<

OFILES = draw.o mouse.o reader.o

draw : $(OFILES)
	$(CC) $(LDFLAGS) -o draw $(OFILES) -lGL -lGLU -lglut -lXmu

dist :
	(make clean; cd ..; tar cvfz turtle3d.tar.gz `find turtle3d ! -type d | egrep -v 'CVS|experi|agenda'`; mv turtle3d.tar.gz turtle3d)

clean :
	-rm *.o draw core.* *.core *~ *.pyc
