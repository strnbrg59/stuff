revsqz : cncttime.o forcesal.o options.o bldordr.o cancel.o dispordr.o disptrad.o revsqz.o menu.o\
       client.o orders.o password.o holdings.o trades.o util.o diary.o loadsec.o xmem.o
	g++ -o revsqz cncttime.o forcesal.o options.o bldordr.o cancel.o dispordr.o disptrad.o orders.o revsqz.o \
		menu.o password.o holdings.o trades.o util.o\
		client.o diary.o loadsec.o xmem.o -lm -lcursesX -ltermcap
bldordr.o : bldordr.c aqc.h
	g++ -c -g bldordr.c
cncttime.o : cncttime.c aqc.h
	g++ -c -g cncttime.c
forcesal.o : forcesal.c aqc.h
	g++ -c -g forcesal.c
options.o : options.c aqc.h
	g++ -c -g options.c
cancel.o : cancel.c aqc.h
	g++ -c -g cancel.c 
client.o : client.c aqc.h communic.h
	g++ -c -g client.c
diary.o : diary.c aqc.h
	g++ -c -g diary.c
dispordr.o : dispordr.c aqc.h
	g++ -c -g dispordr.c 
disptrad.o : disptrad.c aqc.h
	g++ -c -g disptrad.c
revsqz.o : revsqz.c aqc.h
	g++ -c -g revsqz.c
menu.o : menu.c aqc.h
	g++ -c -g menu.c 
orders.o : orders.c aqc.h
	g++ -c -g orders.c
password.o : password.c aqc.h
	g++ -c -g password.c
holdings.o : holdings.c aqc.h
	g++ -c -g holdings.c
trades.o : trades.c aqc.h
	g++ -c -g trades.c
util.o : util.c aqc.h
	g++ -c -g util.c
loadsec.o : loadsec.c aqc.h
	g++ -c -g loadsec.c
xmem.o : xmem.c
	g++ -c -g xmem.c
