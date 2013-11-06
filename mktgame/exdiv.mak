exdiv : exdiv.o 
	gcc -o exdiv exdiv.o aqc.a -lm -lcurses -ltermcap
exdiv.o : main/exdiv.c aqc.h
	gcc -c main/exdiv.c
