
fwddeliv : forcesal.o options.o cash.o bldordr.o cancel.o dispordr.o disptrad.o fwddeliv.o menu.o\
       client.o orders.o password.o portf.o trades.o util.o diary.o xmem.o
	cc -o fwddeliv forcesal.o options.o cash.o bldordr.o cancel.o dispordr.o disptrad.o orders.o fwddeliv.o \
		menu.o password.o portf.o trades.o util.o\
		client.o diary.o xmem.o -lm  -lcursesX -ltermcap
bldordr.o : bldordr.c aqc.h
	cc -c -g bldordr.c
cash.o : cash.c aqc.h
	cc -c -g cash.c
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
fwddeliv.o : fwddeliv.c aqc.h
	cc -c -g fwddeliv.c
menu.o : menu.c aqc.h
	cc -c -g menu.c 
orders.o : orders.c aqc.h
	cc -c -g orders.c
password.o : password.c aqc.h
	cc -c -g password.c
portf.o : portf.c aqc.h
	cc -c -g portf.c
trades.o : trades.c aqc.h
	cc -c -g trades.c
util.o : util.c aqc.h
	cc -c -g util.c
xmem.o : xmem.c
	cc -c -g xmem.c
