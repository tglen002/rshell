rshell
======

Project source can be downloaded from https://github.com/tglen002/rshell.git
-----------

Authors and Contributors List
-----------
Tiffany Glenn-Hall

How to run rshell.cpp
----------------

Once you have cloned the directory to your local machine, follow the directions below:
<ol>
  <li>cd rshell</li>
  <li>make</li>
  <li>cd bin</li>
  <li>./rshell</li>
</ol>

Program overview:
This program was written to implement a simple version of command shell called rshell.
Rshell can support simple inputs similar to those used in BASH command shell.
It makes commands call through the use of the execvp function and uses a build in exit command to shotdown the program.

Bugs:
<ol>
  <li>| does not work</li>
  <li>cd does not function </li>
</ol>

How to run ls.cpp
----------------

Once you have cloned the directory to your local machine, follow the directions below:
<ol>
  <li>cd rshell</li>
  <li>make</li>
  <li>cd bin</li>
  <li>./ls</li>
</ol>

Program overview:
This program was created to act as the linux terminal command ls. It is able to take 3 commands when the program in run: l, a, R. These commands work exactly the same as ls with a few formatting discrepancies.

Bugs:
<ol>
  <li>-l flag does not have the appropriate formatting</li>
  <li>the program output does not have the expected coloration</li>
</ol>

How to run cp.cpp
----------------

Once you have cloned the directory to your local machine, follow the directions below:
<ol>
  <li>cd rshell</li>
  <li>make</li>
  <li>cd bin</li>
  <li>./cp</li>
</ol>

Program overview:
This program was created to act similarly to the linux terminal command cp. This program has two possible terminal outputs not typical to the linux kernel cp command. 
1. If a 3rd variable is written on the command line the "method" call will be assumed and wall, user, and system elapsed times will be presented for all three methods of cp used: get/put c++ functions one character at a time, read/write unix system calls one character at a time, and read/write one buffer at a time.
2. If a 3rd variable is not present then the read/write unix system calls are used one buffer at a time and no visible output is given in the terminal.
The program expects an input and output file listed in the command line. It will not accept a directory as either input or output. If the output file already exists the program will exit. 

Bugs:
<ol>
  <li>the method variable is the third variable, code requirements assume method can be places anywhere, but instead you can write anything in the 3rd parameter and get the method output</li>
  <li>if output is missing but method is given, the program will assume method is the name of the output file</li>
</ol>


