#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
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

//run
int Run(char **list, char **additionalList, const vector<int> &connectors, unsigned connectorNum, int &previousOutput){
	int failure = 0;
	int communication[2];
	pipe(communication);
	int connectorPre = SEMICOLON;
	int connectorPost = SEMICOLON;
	int pipeOutput[2];
        pipe(pipeOutput);
	if(connectorNum > 0){
		connectorPre = connectors.at(connectorNum - 1);
	}
	if(connectorNum < connectors.size()){
		connectorPost = connectors.at(connectorNum);
	}
        pid_t childPID; //fork section
        childPID = fork();
        if(childPID >= 0){ //fork was successful
          if(childPID == 0){ //child process
	    if(connectorPre == PIPECONN){
		dup2(previousOutput, STDIN_FILENO);
	    }
            else if(connectorPre == INPUTREDIR){
		string input_file = additionalList[0];
		int fd_in = open(input_file.c_str(), O_RDONLY, 0);
                dup2(fd_in, STDIN_FILENO);
            }
	    else if(connectorPre == OUTPUTREDIR){
		string output_file = additionalList[0];
		int fd_out = creat(output_file.c_str(), 0644);
		dup2(fd_out, STDOUT_FILENO);
	    }
	    else if(connectorPost == PIPECONN){
		close(pipeOutput[0]);
		dup2(pipeOutput[1], STDOUT_FILENO);
	    }
	    close(communication[0]);
            failure = execvp(list[0], list);
	    write(communication[1], &failure, sizeof(failure));
	    close(communication[1]);
            if(failure == -1){perror("execvp failed"); exit(1);}
          }
          else{ //parent process
	    close(communication[1]);
	    close(pipeOutput[1]);
            int r = wait(NULL);
            if(r == -1){perror("wait failed"); exit(1);}
	    if(connectorPost == PIPECONN){
	 	read(pipeOutput[0], &previousOutput, sizeof(previousOutput));
		close(pipeOutput[0]);
	    }
	    read(communication[0], &failure, sizeof(failure));
	    close(communication[0]);
	    if(failure == 0){}
	    else{failure = -1;}
          }
        }
        else{ // fork failed
          perror("fork failed");
          exit(1);
        }
	return failure;
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
    char connectors[] = "&|<>;";
    char *indvCommands;
    char *currCmmdLine;
    indvCommands = strtok_r(charCmmdLine, connectors, &currCmmdLine);

    char **list = new char*[commandLine.length() + 1];
    char **talkBetweenChunks = new char*[commandLine.length() + 1];
    for(unsigned i = 0; i < commandLine.length() + 1; i++){ 
	list[i] = '\0';
	talkBetweenChunks[i] = '\0';
    }

    //determines where connectors are and what they are
    vector<int> connectorsUsed;
    ConnectorsUsed(commandLine, connectorsUsed);
    //if //if the last object is a connector get more input

    string outputFile = "outputFile";
    int previousOutput = creat(outputFile.c_str(), 0644);

    bool success = true; //states whether previous command on same line was successful
    unsigned lineConnectorNum = 0; //location within the connector line array
    bool isComment = false;

    while (indvCommands != NULL){ //for multiple commands on one line
      if(strcmp(indvCommands,NoRun) == 0 || strcmp(indvCommands,SpaceNoRun) == 0){
        exit(EXIT_SUCCESS);
      }
cout << "command chunk: " << indvCommands << endl;
 
      if(isComment == true){break;}

      unsigned iterator = 0;

      if(true){
	//unsigned iterator = 0;
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
//cout << "success top: " << success << endl;
      if(lineConnectorNum == 0){
		cout << "entering line connector num == 0" << endl;
		if(connectorsUsed.size() > 0){
			if((connectorsUsed.at(lineConnectorNum) == INPUTREDIR) || 
				(connectorsUsed.at(lineConnectorNum) == OUTPUTREDIR)){
				unsigned iterator = 0;
				while(list[iterator] != NULL){
					talkBetweenChunks[iterator] = list[iterator];
					iterator++;
				}
				run = false;
			} //first element
			else{cout << "entering else" << endl; run = true;}
		}
		else{run = true;}
      }
      else if(lineConnectorNum < connectorsUsed.size()){ 
		if((connectorsUsed.at(lineConnectorNum) == INPUTREDIR) ||
                	(connectorsUsed.at(lineConnectorNum) == OUTPUTREDIR)){
			unsigned iterator = 0;
                	while(list[iterator] != NULL){
                        	talkBetweenChunks[iterator] = list[iterator];
                                iterator++;
                        }
		}
                run = false;
      }
      else if((connectorsUsed.at(lineConnectorNum - 1) == INPUTREDIR) || 
                (connectorsUsed.at(lineConnectorNum - 1) == OUTPUTREDIR)){
		char **switchArrays = new char*[commandLine.length() + 1];
    		for(unsigned i = 0; i < commandLine.length() + 1; i++){
			switchArrays[i] = list[i];
			list[i] = talkBetweenChunks[i];
			talkBetweenChunks[i] = switchArrays[i];
    		}
		run = true;
      }
      else if(connectorsUsed.at(lineConnectorNum - 1) == OUTPUTREDIR){
		
      }
      else if((connectorsUsed.at(lineConnectorNum - 1) == ANDCONNECTOR) && (success == true)){
		cout << "processed as &&: success == true" << endl;
		run = true;} //&&
      else if((connectorsUsed.at(lineConnectorNum - 1) == ANDCONNECTOR) && (success == false)){
		if(lineConnectorNum - 2 > 0){
			 if(connectorsUsed.at(lineConnectorNum - 2) == ORCONNECTOR){
				cout << "processed as &&, previous connector == ||, success == false" << endl; 
				run = true;}
			else{run = false;}
		}
		else{run = false;} 
      }
      else if((connectorsUsed.at(lineConnectorNum - 1) == ORCONNECTOR) && (success == true)){
		cout << "processed as ||: success == true" << endl; 
		run = false;} //||
      else if((connectorsUsed.at(lineConnectorNum - 1) == ORCONNECTOR) && (success == false)){
		cout << "processed as ||: success == false" << endl;
		run = true;}
      else if(connectorsUsed.at(lineConnectorNum - 1) == SEMICOLON){cout << "processed as ;" << endl; run = true;} //;
      else{cout << "untracked connector: connect " << connectorsUsed.at(lineConnectorNum - 1) << ", success " << success << endl; run = true;}

      int failure = 0;
      if(run == true){
cout << "entering run statement" << endl;
	  failure = Run(list, talkBetweenChunks, connectorsUsed, lineConnectorNum, previousOutput);
      }
      indvCommands = strtok_r(NULL, connectors, &currCmmdLine);
      lineConnectorNum++; //moving to the next connector in array
      if(failure == -1){success = false;}

      if(indvCommands == NULL){break;}
    }
    delete [] charCmmdLine;
    delete [] list;
    delete [] talkBetweenChunks;
    close(previousOutput);
  }


  return 0;
}
