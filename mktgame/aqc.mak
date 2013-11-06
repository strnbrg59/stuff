aqc : aqc.o 
	gcc -o aqc aqc.o aqc.a -lm -lncurses -ltermcap
aqc.o : main/aqc.c aqc.h
	gcc -c main/aqc.c
