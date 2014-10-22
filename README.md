rshell
======

Licensing Information: READ LICENSE
------------

Project sourse can be downloaded from https://github.com/tglen002/rshell.git
-----------

Authors and Contributors List
-----------
Tiffany Glenn-Hall
Mike Izbicki

File List
--------
~~~~~
.:

Makefile

LICENSE

README.md

./src

./tests
~~~~~~
/src:

rshell.cpp
~~~~~~~
/tests:
exec.script

How to run file
----------------

Once you have cloned the directory to your local machine, follow the directions below:
1. cd into rshell directory
2. call make
3. cd into bin
4. call rshell

Program overview:
This program was written to implement a simple version of command shell called rshell.
Rshell can support simple inputs similar to those used in BASH command shell.
It makes commands call through the use of the execvp function and uses a build in exit command to shotdown the program.

Bugs:
1. && and || work as if ;
2. 2nd command statment skipped (this was created when I tried to implement && and ||
3. cd does not function 




