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
  char noRun[] = "exit";
  cout << "$ ";

  string commandLine;
  getline(cin,commandLine);
  char *charCmmdLine = new char[commandLine.length() + 1];
  strcpy(charCmmdLine, commandLine.c_str());
  char connectors[] = "&|;";
  char *indvCommands;
  indvCommands = strtok(charCmmdLine, connectors);


  char **list = new char*[commandLine.length() + 1];
  for(unsigned i = 0; i < commandLine.length() + 1; i++){ list[i] = '\0';}


  while (indvCommands != NULL){
    if(strcmp(indvCommands,noRun) == 0){
      exit(EXIT_SUCCESS);
    }
  if(true){
    list[0] = strtok(indvCommands," ");
    unsigned iterator = 1;
    while(indvCommands != NULL && iterator < commandLine.length()+1){
      list[iterator] = strtok(NULL," ");
      iterator++;
    }
  }
    //list[0] = strtok(indvCommands," ");
    //list[1] = strtok(NULL," ");
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
  delete [] list;
  return 0;
}
