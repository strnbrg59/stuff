initact : initact.o 
	gcc -o initact initact.o aqc.a -lm -lcurses -ltermcap
initact.o : main/initact.c aqc.h
	gcc -c main/initact.c
