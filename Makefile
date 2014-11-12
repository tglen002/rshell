all: rshell ls cp

clean:
	-rm -r bin;
	rm cp ls rshell;

rshell: rshell.cpp
	g++ rshell.cpp -Wall -Werror -ansi -pedantic -o rshell
	-mkdir bin
	cd bin; cp ../rshell .

ls: ls.cpp
	g++ ls.cpp -Wall -Werror -ansi -pedantic -o ls
	-mkdir bin
	cd bin; cp ../ls .

cp: cp.cpp
	g++ cp.cpp -Wall -Werror -ansi -pedantic -o cp
	-mkdir bin
	cd bin; cp ../cp .
