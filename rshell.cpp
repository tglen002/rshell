#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<errno.h>
#include<iostream>


using namespace std;

int main()
{
  char* list[3] = {'\0'};
  string commandLine;
  cout << "$ ";
  getline(cin,commandLine);
  //cout << "output: " << commandLine << endl;
  char *charCmmdLine = new char[commandLine.length() + 1];
  strcpy(charCmmdLine, commandLine.c_str());
  char connectors[] = "&|;";
  char * indvCommands;
  indvCommands = strtok(charCmmdLine, connectors);
  //if indivCommands = exit then exit the program

  while (indvCommands != NULL){
    //{ printf("%s\n", indvCommands); indvCommands = strtok(NULL,connectors);}
    list[0] = strtok(indvCommands," ");
    list[1] = strtok(NULL," ");
    pid_t childPID;
    childPID = fork();
    if(childPID >= 0){ //fork was successful
      if(childPID == 0){ //child process
      //contains execvp
        //cout << "I am currently in the child" << endl;
        int r = execvp(list[0], list);
	if(r == -1){
	  perror("execvp failed");
	  exit(1);
        }
      }
      else{ //parent process
        wait(NULL);
        //cout << "I am currently in the parent" << endl;
      }
    }
    else{ // fork failed
      perror("fork failed");
      exit(1);
    }
    indvCommands = strtok(NULL, connectors);
  }

  delete [] charCmmdLine;
  
  //char *argv[2]; //need to change in order to fit unlimited number of calls
  

  return 0;
}
