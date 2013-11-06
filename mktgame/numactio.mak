numactio : cncttime.o forcesal.o options.o bldordr.o cancel.o dispordr.o disptrad.o numactio.o menu.o\
       client.o orders.o password.o holdings.o trades.o util.o diary.o loadsec.o xmem.o
	cc -o numactio cncttime.o forcesal.o options.o bldordr.o cancel.o dispordr.o disptrad.o orders.o numactio.o \
		menu.o password.o holdings.o trades.o util.o\
		client.o diary.o loadsec.o xmem.o -lm -lcursesX -ltermcap
bldordr.o : bldordr.c aqc.h
	cc -c bldordr.c
cncttime.o : cncttime.c aqc.h
	cc -c cncttime .c
forcesal.o : forcesal.c aqc.h
	cc -c forcesal.c
options.o : options.c aqc.h
	cc -c options.c
cancel.o : cancel.c aqc.h
	cc -c cancel.c 
client.o : client.c aqc.h communic.h
	cc -c client.c
diary.o : diary.c aqc.h
	cc -c diary.c
dispordr.o : dispordr.c aqc.h
	cc -c dispordr.c 
disptrad.o : disptrad.c aqc.h
	cc -c disptrad.c
numactio.o : numactio.c aqc.h
	cc -c numactio.c
menu.o : menu.c aqc.h
	cc -c menu.c 
orders.o : orders.c aqc.h
	cc -c orders.c
password.o : password.c aqc.h
	cc -c password.c
holdings.o : holdings.c aqc.h
	cc -c holdings.c
trades.o : trades.c aqc.h
	cc -c trades.c
util.o : util.c aqc.h
	cc -c util.c
loadsec.o : loadsec.c aqc.h
	cc -c loadsec.c
xmem.o : xmem.c
	cc -c xmem.c
