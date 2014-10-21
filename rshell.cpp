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
  char noRun[] = "exit";
  string commandLine;
  cout << "$ ";
  getline(cin,commandLine);
  char *charCmmdLine = new char[commandLine.length() + 1];
  strcpy(charCmmdLine, commandLine.c_str());
  char connectors[] = "&|;";
  char * indvCommands;
  indvCommands = strtok(charCmmdLine, connectors);

  while (indvCommands != NULL){
    if(strcmp(indvCommands,noRun) == 0){
      exit(EXIT_SUCCESS);
    }
    list[0] = strtok(indvCommands," ");
    list[1] = strtok(NULL," ");
    pid_t childPID;
    childPID = fork();
    if(childPID >= 0){ //fork was successful
      if(childPID == 0){ //child process
        int r = execvp(list[0], list);
        if(r == -1){
	  perror("execvp failed");
	  exit(1);
        }
      }
      else{ //parent process
        wait(NULL);
      }
    }
    else{ // fork failed
      perror("fork failed");
      exit(1);
    }
    indvCommands = strtok(NULL, connectors);
  }
  delete [] charCmmdLine;

  return 0;
}
