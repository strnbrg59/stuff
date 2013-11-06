# Warning! The ifeq() construction doesn't work if you leave any space
# 	between the second argument and the closing parenthesis!

MACHINE = Linux
SOCKETS = True_sockets

# All objects except the client/fclient ones.
	MOSTOBJECTS = bldordr.o cancel.o diary.o dispordr.o disptrad.o forcesal.o holdings.o html-util.o initG.o loadsec.o menu.o options.o orders.o password.o trades.o util.o xmem.o

ifeq ($(SOCKETS), Filebased_sockets)
	OBJECTS = $(MOSTOBJECTS) fclient.o
else
	OBJECTS = $(MOSTOBJECTS) client.o
endif

CFLAGS = -c -g -Wall -D$(MACHINE) -D$(SOCKETS)

CGIPROGS = CGInamepwd CGImenu1 CGImenu2 CGIcancel CGIbldordr \
CGIoption CGIfinger 

ifeq ($(MACHINE), Rahul)
	CGIDIR = /local/lib/httpd/strnbrg/cgi-bin
else
	CGIDIR = /users/ted/public_html/cgi-bin
endif

all : $(CGIPROGS) fserver server
	mv $(CGIPROGS) $(CGIDIR);\
	chmod 6755 $(CGIDIR)/CGI*;\
#	strip $(CGIDIR)/CGI*;\

clean :
	rm $(CGIPROGS) *~ *.o

#####################################
### Utility Programs ###

initact : initact.o $(OBJECTS)
	g++ -o initact initact.o $(OBJECTS) -lm
initact.o : main/initact.cpp
	g++ $(CFLAGS) main/initact.cpp

poisson : poisson.o $(OBJECTS)
	g++ -o poisson poisson.o $(OBJECTS) -lm
	mv poisson $(CGIDIR)
poisson.o : main/poisson.cpp
	g++ $(CFLAGS) main/poisson.cpp

discwind : discwind.o $(OBJECTS)
	g++ -o discwind discwind.o $(OBJECTS) -lm
	mv discwind $(CGIDIR)
discwind.o : main/discwind.cpp
	g++ $(CFLAGS) main/discwind.cpp

exdiv : exdiv.o $(OBJECTS)
	g++ -o exdiv exdiv.o $(OBJECTS) -lm
exdiv.o : main/exdiv.cpp
	g++ $(CFLAGS) main/exdiv.cpp

#####################################
### CGI Programs ###
CGInamepwd: CGInamepwd.o $(OBJECTS)
	g++ -o CGInamepwd CGInamepwd.o $(OBJECTS) -lm

CGIoption: CGIoption.o $(OBJECTS)
	g++ -o CGIoption CGIoption.o $(OBJECTS) -lm

CGImenu1: CGImenu1.o $(OBJECTS)
	g++ -o CGImenu1 CGImenu1.o $(OBJECTS) -lm

CGImenu2: CGImenu2.o $(OBJECTS)
	g++ -o CGImenu2 CGImenu2.o $(OBJECTS) -lm

CGIcancel: CGIcancel.o $(OBJECTS)
	g++ -o CGIcancel CGIcancel.o $(OBJECTS) -lm

CGIfinger: CGIfinger.o $(OBJECTS)
	g++ -o CGIfinger CGIfinger.o $(OBJECTS) -lm

CGIbldordr: CGIbldordr.o $(OBJECTS)
	g++ -o CGIbldordr CGIbldordr.o $(OBJECTS) -lm

CGInamepwd.o : CGInamepwd.cpp aqc.hpp html-util.hpp
	g++ $(CFLAGS) CGInamepwd.cpp 

CGIoption.o: CGIoption.cpp aqc.hpp html-util.hpp
	g++ $(CFLAGS) CGIoption.cpp 

CGImenu1.o: CGImenu1.cpp aqc.hpp html-util.hpp
	g++ $(CFLAGS) CGImenu1.cpp 

CGImenu2.o: CGImenu2.cpp  aqc.hpp html-util.hpp 
	g++ $(CFLAGS) CGImenu2.cpp 

CGIcancel.o: CGIcancel.cpp  aqc.hpp html-util.hpp 
	g++ $(CFLAGS) CGIcancel.cpp 

CGIfinger.o: CGIfinger.cpp aqc.hpp html-util.hpp 
	g++ $(CFLAGS) CGIfinger.cpp 

CGIbldordr.o: CGIbldordr.cpp aqc.hpp html-util.hpp 
	g++ $(CFLAGS) CGIbldordr.cpp 

##################################
### Objects Common to All Programs

bldordr.o : bldordr.cpp aqc.hpp 
	g++ $(CFLAGS) bldordr.cpp 
cancel.o : cancel.cpp aqc.hpp 
	g++ $(CFLAGS) cancel.cpp 
client.o : client.cpp communic.hpp
	g++ $(CFLAGS) client.cpp
fclient.o : fclient.cpp fcommunic.hpp
	g++ $(CFLAGS) fclient.cpp
diary.o : diary.cpp aqc.hpp
	g++ $(CFLAGS) diary.cpp 
dispordr.o : dispordr.cpp aqc.hpp 
	g++ $(CFLAGS) dispordr.cpp 
disptrad.o : disptrad.cpp aqc.hpp 
	g++ $(CFLAGS) disptrad.cpp 
forcesal.o : forcesal.cpp aqc.hpp
	g++ $(CFLAGS) forcesal.cpp 
holdings.o : holdings.cpp aqc.hpp
	g++ $(CFLAGS) holdings.cpp 
html-util.o : html-util.cpp html-util.hpp
	g++ $(CFLAGS) html-util.cpp 
initG.o : initG.cpp aqc.hpp
	g++ $(CFLAGS) initG.cpp 
loadsec.o : loadsec.cpp aqc.hpp 
	g++ $(CFLAGS) loadsec.cpp 
menu.o : menu.cpp aqc.hpp 
	g++ $(CFLAGS) menu.cpp 
options.o : options.cpp aqc.hpp 
	g++ $(CFLAGS) options.cpp 
orders.o : orders.cpp aqc.hpp 
	g++ $(CFLAGS) orders.cpp 
password.o : password.cpp aqc.hpp
	g++ $(CFLAGS) password.cpp 
trades.o : trades.cpp aqc.hpp 
	g++ $(CFLAGS) trades.cpp 
util.o : util.cpp aqc.hpp 
	g++ $(CFLAGS) util.cpp 
xmem.o : xmem.cpp 
	g++ $(CFLAGS) xmem.cpp 

###################################

#################
### Servers ###
server : server.o servsock.o
	g++ -o server server.o servsock.o;\
	mv server $(CGIDIR)

server.o : main/server.cpp communic.hpp server_error.hpp
	g++ $(CFLAGS) main/server.cpp
servsock.o : servsock.cpp communic.hpp server_error.hpp
	g++ $(CFLAGS) servsock.cpp

fserver : fserver.o
	g++ -o fserver fserver.o;\
	mv fserver $(CGIDIR)

fserver.o : fserver.cpp fcommunic.hpp
	g++ $(CFLAGS) fserver.cpp
