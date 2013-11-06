server : server.o servsock.o
	gcc -o server server.o servsock.o -lm
server.o : main/server.cpp communic.hpp server_error.hpp
	gcc -c main/server.cpp
servsock.o : servsock.cpp communic.hpp server_error.hpp
	gcc -c servsock.cpp
