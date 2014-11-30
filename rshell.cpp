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

#define ANDCONNECTOR 0
#define ORCONNECTOR 1
#define PIPECONN 2
#define INPUTREDIR 3
#define OUTPUTREDIR 4
#define SEMICOLON 5

const string PROMPT = "$MONEY$: ";
const char COMMENT = '#';
const char NoRun[] = "exit";
const char SpaceNoRun[] = " exit";
const unsigned NumConnectors = 6;
const string FullConnectors[] = {"&&", "||", "|", "<", ">", ";"};

void ConnectorsUsed(string commandLine, vector<int> &connectorsUsed){
    vector<double> connectorAndPos;
    for(unsigned numConnect = 0; numConnect < NumConnectors; numConnect++){
      size_t posCommandLine = 0;
      size_t currPos = 0;
      string subCommandLine = commandLine;
      while(currPos != string::npos){
        currPos = subCommandLine.find(FullConnectors[numConnect]);
        if(currPos == string::npos){break;}
        if(currPos == 0){cout << "syntax error near unextected token '"
            << FullConnectors[numConnect] << "'" << endl;
            exit(1);
        }
	if((currPos == commandLine.length() - 2) && (numConnect <= ORCONNECTOR)){continue;}
	else if((currPos == commandLine.length() - 1) && (numConnect < SEMICOLON)){continue;}
	else if((currPos == commandLine.length() - 1) && (numConnect == SEMICOLON)){break;}
        posCommandLine = posCommandLine + currPos;
        connectorAndPos.push_back(posCommandLine+(double(numConnect)/double(10)));
        string tempCommandLine = subCommandLine;
        subCommandLine = tempCommandLine.substr(currPos+2);
	cout << "pos and conn: " << posCommandLine+(double(numConnect)/double(10)) << endl;
      }
    }
    stable_sort(connectorAndPos.begin(), connectorAndPos.end());
    double connectorValue = 0.0;
    for(unsigned i = 0; i < connectorAndPos.size(); i++){
      int temp = static_cast<int>(connectorAndPos.at(i));
      if(i < connectorAndPos.size() - 1){
      		if(int(connectorAndPos.at(i)) == int(connectorAndPos.at(i+1))){
			if(connectorAndPos.at(i) < connectorAndPos.at(i+1))
				{connectorAndPos.erase(connectorAndPos.begin()+(i+1));}
			else{connectorAndPos.erase(connectorAndPos.begin()+i);}
		}
      }
      connectorValue = 10 * (connectorAndPos.at(i) - temp);
      if((connectorValue < ANDCONNECTOR + 0.5) && (connectorValue > ANDCONNECTOR - 0.5))
		{connectorsUsed.push_back(ANDCONNECTOR);}
      else if((connectorValue < ORCONNECTOR + 0.5) && (connectorValue > ORCONNECTOR - 0.5))
		{connectorsUsed.push_back(ORCONNECTOR);}
      else if((connectorValue < PIPECONN + 0.5) && (connectorValue > PIPECONN - 0.5))
                {connectorsUsed.push_back(PIPECONN);}
      else if((connectorValue < INPUTREDIR + 0.5) && (connectorValue > INPUTREDIR - 0.5))
                {connectorsUsed.push_back(INPUTREDIR);}
      else if((connectorValue < OUTPUTREDIR + 0.5) && (connectorValue > OUTPUTREDIR - 0.5))
                {connectorsUsed.push_back(OUTPUTREDIR);}
      else{connectorsUsed.push_back(SEMICOLON);}
        cout << "connector " << i << " = " << connectorsUsed.at(i) << endl;

    }
}

int main()
{
  while(true){ //for multiple command lines
    string commandLine;
    cout << PROMPT;
    fflush(stdout);
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
    vector<int> connectorsUsed;
    ConnectorsUsed(commandLine, connectorsUsed);
    //if //if the last object is a connector get more input

    bool success = true; //states whether previous command on same line was successful
    int lineConnectorNum = 0; //location within the connector line array
    bool isComment = false;

    while (indvCommands != NULL){ //for multiple commands on one line
      if(strcmp(indvCommands,NoRun) == 0 || strcmp(indvCommands,SpaceNoRun) == 0){
        exit(EXIT_SUCCESS);
      }
 
      if(isComment == true){break;}

      unsigned iterator = 0;

      if(true){
        list[0] = strtok(indvCommands," ");
        while((list[iterator] != NULL) && (iterator < commandLine.length()+1)){
          iterator++;
          list[iterator] = strtok(NULL," ");
        }
      }

      if(*list[0] == COMMENT){break;} //check for comment
      unsigned temp = 0;
      while(temp < iterator){
        if(*list[temp] == COMMENT){isComment = true;} //check for comment
        if(isComment == true){list[temp] = NULL;}
        temp++;
      }

      bool run = true;
cout << "success top: " << success << endl;
      if(lineConnectorNum == 0){run = true;} //first element
      else if((connectorsUsed.at(lineConnectorNum - 1) == ANDCONNECTOR) && (success == true)){
		cout << "processed as &&: connect " << connectorsUsed.at(lineConnectorNum - 1) << ", success " << success << endl;
		run = true;} //&&
      else if((connectorsUsed.at(lineConnectorNum - 1) == ANDCONNECTOR) && (success == false)){
		cout << "processed as &&: connect " << connectorsUsed.at(lineConnectorNum - 1) << ", success " << success << endl;
		if(lineConnectorNum - 2 > 0){
			cerr << "current position: " << lineConnectorNum - 2 << endl;
			 if(connectorsUsed.at(lineConnectorNum - 2) == ORCONNECTOR){run = true;}
		}
		else{run = false;} 
      }
      else if((connectorsUsed.at(lineConnectorNum - 1) == ORCONNECTOR) && (success == true)){
		cout << "processed as ||: connect " << connectorsUsed.at(lineConnectorNum - 1) << ", success " << success << endl; 
		run = false;} //||
      else if(connectorsUsed.at(lineConnectorNum - 1) == SEMICOLON){cout << "processed as ;" << endl; run = true;} //;
      else{cout << "untracked connector: connect " << connectorsUsed.at(lineConnectorNum - 1) << ", success " << success << endl; run = true;}

      int failure = 0;
      if(run == true){
        pid_t childPID; //fork section
        childPID = fork();
        if(childPID >= 0){ //fork was successful
          if(childPID == 0){ //child process
            failure = execvp(list[0], list);
	    cout << "success1: " << success << " failure: " << failure << endl;
            if(failure == -1){success = false; perror("execvp failed"); exit(1);}
	    //if(r == -1){success = false;}
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
      }
      indvCommands = strtok_r(NULL, connectors, &currCmmdLine);
      lineConnectorNum++; //moving to the next connector in array
      if(failure == -1){success = false;}
      cout << "success2: " << success << " failure: " << failure << endl;

      if(indvCommands == NULL){break;}
    }
    delete [] charCmmdLine;
    delete [] list;
  }


  return 0;
}
