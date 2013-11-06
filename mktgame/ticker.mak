ticker : cncttime.o forcesal.o options.o bldordr.o cancel.o dispordr.o disptrad.o ticker.o menu.o\
       client.o orders.o password.o holdings.o trades.o util.o diary.o loadsec.o xmem.o
	cc -o ticker cncttime.o forcesal.o options.o bldordr.o cancel.o dispordr.o disptrad.o orders.o ticker.o \
		menu.o password.o holdings.o trades.o util.o\
		client.o diary.o loadsec.o xmem.o -lm -lcursesX -ltermcap
bldordr.o : bldordr.c aqc.h
	cc -c -g bldordr.c
cncttime.o : cncttime.c aqc.h
	cc -c -g cncttime.c
forcesal.o : forcesal.c aqc.h
	cc -c -g forcesal.c
options.o : options.c aqc.h
	cc -c -g options.c
cancel.o : cancel.c aqc.h
	cc -c -g cancel.c 
client.o : client.c aqc.h communic.h
	cc -c -g client.c
diary.o : diary.c aqc.h
	cc -c -g diary.c
dispordr.o : dispordr.c aqc.h
	cc -c -g dispordr.c 
disptrad.o : disptrad.c aqc.h
	cc -c -g disptrad.c
ticker.o : ticker.c aqc.h
	cc -c -g ticker.c
menu.o : menu.c aqc.h
	cc -c -g menu.c 
orders.o : orders.c aqc.h
	cc -c -g orders.c
password.o : password.c aqc.h
	cc -c -g password.c
holdings.o : holdings.c aqc.h
	cc -c -g holdings.c
trades.o : trades.c aqc.h
	cc -c -g trades.c
util.o : util.c aqc.h
	cc -c -g util.c
loadsec.o : loadsec.c aqc.h
	cc -c -g loadsec.c
xmem.o : xmem.c
	cc -c -g xmem.c
