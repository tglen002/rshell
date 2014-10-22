#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<errno.h>
#include<iostream>
#include<string>
#include<vector>
#include<algorithm>

using namespace std;

int main()
{
  char noRun[] = "exit";
  char spaceNoRun[] = " exit";
  char comment = '#';
  string prompt = "$ ";
  unsigned numConnectors = 3;
  string fullConnectors[] = {"&&", "||", ";"};


  while(true){ //for multiple command lines
    cout << prompt;
    fflush(stdout);
    string commandLine;
    getline(cin,commandLine);
    char *charCmmdLine = new char[commandLine.length() + 1];
    strcpy(charCmmdLine, commandLine.c_str());
    char connectors[] = "&|;";
    char *indvCommands;
    char *currCmmdLine;
    indvCommands = strtok_r(charCmmdLine, connectors, &currCmmdLine);

    char **list = new char*[commandLine.length() + 1];
    for(unsigned i = 0; i < commandLine.length() + 1; i++){ list[i] = '\0';}


    //determines where connectors are and what they are
    vector<double> connectorsUsed;
    for(unsigned numConnect = 0; numConnect < numConnectors; numConnect++){
      size_t posCommandLine = 0;
      size_t currPos = 0;
      string subCommandLine = commandLine;
      while(currPos != string::npos){
        currPos = subCommandLine.find_first_of(fullConnectors[numConnect]);
        if(currPos == string::npos){break;}
        posCommandLine = posCommandLine + currPos;
        connectorsUsed.push_back(posCommandLine+(double(numConnect)/double(10)));
        string tempCommandLine = subCommandLine;
        subCommandLine = tempCommandLine.substr(currPos+2);
      }
    }
    stable_sort(connectorsUsed.begin(), connectorsUsed.end());
    for(unsigned i = 0; i < connectorsUsed.size(); i++){
      int temp = static_cast<int>(connectorsUsed.at(i));
      connectorsUsed.at(i) = connectorsUsed.at(i) - temp;
    }

    //int numTokens = -1;
    //int r = 0;
    
    while (indvCommands != NULL){ //for multiple commands on one line
      if(strcmp(indvCommands,noRun) == 0 || strcmp(indvCommands,spaceNoRun) == 0){
        exit(EXIT_SUCCESS);
      }
      //if(r == -2){break;}
     // if(numTokens != -1){
        //cout << "at the beginning" << endl;
       // if(connectorsUsed.at(numTokens) == 0){if(r == -1){r = -2; break;}}
       // else if(connectorsUsed.at(numTokens) == 0.1){if(r != -1){r = -2; break;}}
       // cout << "at the end" << endl;
      //}

      if(true){
        list[0] = strtok(indvCommands," ");
        unsigned iterator = 1;
        while((indvCommands != NULL) && (iterator < commandLine.length()+1)){
          list[iterator] = strtok(NULL," ");
          iterator++;
        }
      }
      if(*list[0] == comment){break;} //check for comment

      indvCommands = strtok_r(NULL, connectors, &currCmmdLine);
      pid_t childPID; //fork section
      childPID = fork();
      if(childPID >= 0){ //fork was successful
        if(childPID == 0){ //child process
          int r = execvp(list[0], list);
          if(r == -1){perror("execvp failed"); exit(1);}
        }
        else{ //parent process
          int r = wait(NULL);
          if(r == -1){perror("wait failed"); exit(1);}
        }
      }
      else{ // fork failed
        perror("fork failed");
        exit(1);
      }
      indvCommands = strtok_r(NULL, connectors, &currCmmdLine);
      //numTokens++;
    }
    delete [] charCmmdLine;
    delete [] list;
  }


  return 0;
}
