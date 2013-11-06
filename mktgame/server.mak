server : server.o servsock.o
	gcc -o server server.o servsock.o -lm
server.o : main/server.c communic.h server_error.h
	gcc -c main/server.c
servsock.o : servsock.c communic.h server_error.h
	gcc -c servsock.c
