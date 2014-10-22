all: rshell

rshell: rshell.cpp
	g++ rshell.cpp -Wall -Werror -ansi -pedantic -o rshell
	mkdir bin
	cd bin; mkdir rshell
	cp rshell bin/rshell
