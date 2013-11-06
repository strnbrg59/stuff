poisson : poisson.o 
	gcc -o poisson poisson.o aqc.a -lm -lcurses -ltermcap
poisson.o : main/poisson.c aqc.h
	gcc -c main/poisson.c
