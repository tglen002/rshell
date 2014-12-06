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
#define SEMICOLON 5

const string PROMPT = "$MONEY$: ";
const char COMMENT = '#';
const char NoRun[] = "exit";
const char SpaceNoRun[] = " exit";
const unsigned NumConnectors = 4;
const string FullConnectors[] = {"&&", "||", "|", ";"};

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
      else{connectorsUsed.push_back(SEMICOLON);}
    }
}

//run
int Run(char **list, const vector<int> &connectors, unsigned connectorNum){
	int failure = 0;
	int communicateFailure[2];
	pipe(communicateFailure);
	unsigned i = 0;
	while(list[i] != NULL){
		i++;
	}
	unsigned listLength = i + 1;
	
        pid_t childPID; //fork section
        childPID = fork();
        if(childPID >= 0){ //fork was successful
          if(childPID == 0){ //child process
	    unsigned inputRedirSymbol = listLength;
	    unsigned iterator = 0;
	    string inputSymbol = "<";
	    while(list[iterator] != NULL){
		if(list[iterator] == inputSymbol){
			inputRedirSymbol = iterator;
			break;
		}
		iterator++;
	    }
	    if(inputRedirSymbol != listLength){
		string redirInput = list[inputRedirSymbol + 1];//input name
		unsigned i = inputRedirSymbol;
		unsigned iterator = 0;
		while(list[i] != NULL){
			list[i] = list[i+2];
			list[i+2] = NULL;
			i++;
			iterator++;
		}
		int fd_in = open(redirInput.c_str(), O_RDONLY, 0);
		if(fd_in == -1){perror("input redirection error"); exit(1);}
		if(dup2(fd_in, 0) == -1){perror("dup2 of redirInput"); exit(1);}
		int r = close(fd_in);
                if(r == -1){perror("input pipe did not close"); exit(1);}
	    }
	    unsigned outputRedirSymbol = listLength;
	    iterator = 0;
	    string outputSymbol = ">";
	    while(list[iterator] != NULL){
		if(list[iterator] == outputSymbol){
			outputRedirSymbol = iterator;
			break;
		}
		iterator++;
	    }
	    if(outputRedirSymbol != listLength){
		string redirOutput = list[outputRedirSymbol + 1];
		unsigned i = outputRedirSymbol;
		unsigned iterator = 0;
		while(list[i] != NULL){
			list[i] = NULL;
			i++;
			iterator++;
	    	}
		int fd_out = creat(redirOutput.c_str(), 0644);
		if(fd_out == -1){perror("open error"); exit(1);}
		int r = dup2(fd_out, 1);
		if(r == -1){perror("dup2 output error"); exit(1);}
		r = close(fd_out);
                if(r == -1){perror("output pipe did not close"); exit(1);}
	    }
            unsigned appendOutputRedirSymbol = listLength;
            iterator = 0;
            string appendOutputSymbol = ">>";
            while(list[iterator] != NULL){
                if(list[iterator] == appendOutputSymbol){
                        appendOutputRedirSymbol = iterator;
                        break;
                }
                iterator++;
            }
            if(appendOutputRedirSymbol != listLength){
                string appendRedirOutput = list[appendOutputRedirSymbol + 1];
                unsigned i = appendOutputRedirSymbol;
                unsigned iterator = 0;
                while(list[i] != NULL){
                        list[i] = NULL;
                        i++;
                        iterator++;
                }
                int fd_appout = open(appendRedirOutput.c_str(), O_WRONLY|O_APPEND);
                if(fd_appout == -1){perror("open error"); exit(1);}
                int r = dup2(fd_appout, 1);
                if(r == -1){perror("dup2 append output error"); exit(1);}
		r = close(fd_appout);
		if(r == -1){perror("append output pipe did not close"); exit(1);}
            }
            if(close(communicateFailure[0]) == -1){perror("close communicateFailure[0] failed"); exit(1);}
            failure = execvp(list[0], list);
            if(failure == -1){perror("execvp failed"); exit(1);}
            if(close(communicateFailure[1]) == -1){perror("close communicateFailure[1] failed"); exit(1);}
          }
          else{ //parent process
	    if(close(communicateFailure[1]) == -1){perror("close communicateFailure[1] failed"); exit(1);}
            int r = wait(NULL);
            if(r == -1){perror("wait failed"); exit(1);}

	    if(read(communicateFailure[0], &failure, sizeof(failure)) == -1)
		{perror("read communicateFailure failed"); exit(1);}
	    if(close(communicateFailure[0]) == -1){perror("close communicateFailure[0] failed"); exit(1);}
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
    char connectors[] = "&|;";
    char *indvCommands;
    char *currCmmdLine;
    indvCommands = strtok_r(charCmmdLine, connectors, &currCmmdLine);

    char **list = new char*[commandLine.length() + 1];
    for(unsigned i = 0; i < commandLine.length() + 1; i++){ 
	list[i] = '\0';
    }

    //determines where connectors are and what they are
    vector<int> connectorsUsed;
    ConnectorsUsed(commandLine, connectorsUsed);

    bool success = true; //states whether previous command on same line was successful
    unsigned lineConnectorNum = 0; //location within the connector line array
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
      if(lineConnectorNum == 0){run = true;}
	else if(((lineConnectorNum - 1) >= 0) && (lineConnectorNum - 1 < connectorsUsed.size())){
		if((connectorsUsed.at(lineConnectorNum - 1) == ANDCONNECTOR) && (success == true)){
                	run = true;
		} //&&
      		else if((connectorsUsed.at(lineConnectorNum - 1) == ANDCONNECTOR) && (success == false)){
			if((lineConnectorNum - 2 >= 0) && (lineConnectorNum - 2 <= connectorsUsed.size())){
                		if(lineConnectorNum - 2 > 0){
                        		if(connectorsUsed.at(lineConnectorNum - 2) == ORCONNECTOR){
                                		run = true;
					}
                        		else{run = false;}
                		}
                		else{run = false;} 
			}
			else{run = false;}
      		}
      		else if((connectorsUsed.at(lineConnectorNum - 1) == ORCONNECTOR) && (success == true)){
                	run = false;
		} //||
      		else if((connectorsUsed.at(lineConnectorNum - 1) == ORCONNECTOR) && (success == false)){
                	run = true;
		}
      		else if(connectorsUsed.at(lineConnectorNum - 1) == SEMICOLON){
			run = true;
		} //;
      		else{
			run = true;
		}
      }
      int failure = 0;
      if(run == true){
	  failure = Run(list, connectorsUsed, lineConnectorNum);
      }
      indvCommands = strtok_r(NULL, connectors, &currCmmdLine);
      lineConnectorNum++; //moving to the next connector in array
      if(failure == -1){success = false;}

      if(indvCommands == NULL){break;}
    }
    delete [] charCmmdLine;
    delete [] list;
  }


  return 0;
}
