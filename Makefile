all: shelly

shelly: rshell.cpp
	g++ rshell.cpp -Wall -Werror -ansi -pedantic -o shelly
