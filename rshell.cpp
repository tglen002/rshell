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
//#define INPUTREDIR 3
//#define OUTPUTREDIR 4
#define SEMICOLON 5

const string PROMPT = "$MONEY$: ";
const char COMMENT = '#';
const char NoRun[] = "exit";
const char SpaceNoRun[] = " exit";
const unsigned NumConnectors = 4;
const string FullConnectors[] = {"&&", "||", "|", ";"};

int communicateOutput[2];

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
	cout << "connectorandPos size = " << connectorAndPos.size() << endl;
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
      /*else if((connectorValue < INPUTREDIR + 0.5) && (connectorValue > INPUTREDIR - 0.5))
                {connectorsUsed.push_back(INPUTREDIR);}
      else if((connectorValue < OUTPUTREDIR + 0.5) && (connectorValue > OUTPUTREDIR - 0.5))
                {connectorsUsed.push_back(OUTPUTREDIR);}*/
      else{connectorsUsed.push_back(SEMICOLON);}
        cout << "connector " << i << " = " << connectorsUsed.at(i) << endl;

    }
}

//run
int Run(char **list, /*char **additionalList, */const vector<int> &connectors, unsigned connectorNum){
	int failure = 0;
	int saveSTDIN = dup(STDIN_FILENO);
	int saveSTDOUT = dup(STDOUT_FILENO);
	int communicateFailure[2];
	pipe(communicateFailure);
	int communicateInput[2];
	pipe(communicateInput);
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
	    //cout << "length of list: " << inputRedirSymbol << endl;
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
			//cout << "list1: " << list[iterator] << endl;
			i++;
			iterator++;
		}
		int fd_in = open(redirInput.c_str(), O_RDONLY, 0);
		if(fd_in == -1){perror("input redirection error"); exit(1);}
		if(dup2(fd_in, STDIN_FILENO) == -1){perror("dup2 of redirInput"); exit(1);}
	    }
	    unsigned outputRedirSymbol = listLength;
	    //cout << "length of edited list: " << listLength << endl;
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
		//cout << "redirOutput: " << redirOutput << endl;
		unsigned i = outputRedirSymbol;
		unsigned iterator = 0;
		while(list[i] != NULL){
			cout << "list2: " << list[iterator] << endl;
			list[i] = NULL;
			i++;
			iterator++;
	    	}
		int fd_out = creat(redirOutput.c_str(), 0644);
		if(fd_out == -1){perror("open error"); exit(1);}
		int r = dup2(fd_out, STDOUT_FILENO);
		if(r == -1){perror("dup2 output error"); exit(1);}
	    }
//cout << "getting to here?" << endl;
            if((connectorNum - 1 >= 0) && (connectorNum - 1 < connectors.size())){
        //cout << "entering connectorNum - 1 >= 0" << endl;
                if(connectors.at(connectorNum - 1) == PIPECONN){
                        cerr << "entering pipe conditionals: pipe before chunk" << endl;
                        int r = write(communicateInput[1], &communicateOutput, sizeof(communicateOutput));
			if(r == -1){perror("write communicateOutput to communicateInput failed"); exit(1);}
			else if(r == 0){perror("communicateOutput was empty"); exit(1);}
                        r = dup2(communicateInput[0], STDIN_FILENO);
                        if(r == -1){perror("dup2 input error"); exit(1);}
                }
            }
	    if(connectorNum < connectors.size()){
		//cout << "entering connectNum < connectors.size()" << endl;
		if(connectors.at(connectorNum) == PIPECONN){
			cerr << "entering pipe conditionals: pipe after chunk" << endl;
			int r = dup2(communicateOutput[1], STDOUT_FILENO);
			if(r == -1){perror("dup2 output error"); exit(1);}
	    	}
	    }
cerr << "getting to here2?" << endl;
            if(close(communicateFailure[0]) == -1){perror("close communicateFailure[0] failed"); exit(1);}
            if(close(communicateInput[0]) == -1){perror("close communicateInput[0] failed"); exit(1);}
            if(close(communicateInput[1]) == -1){perror("close communicateInput[1] failed"); exit(1);}
            if(close(communicateOutput[0]) == -1){perror("close communicateOutput[0] failed"); exit(1);}
            if(close(communicateOutput[1]) == -1){perror("close communicateOutput[1] failed"); exit(1);}

            failure = execvp(list[0], list);
            if(failure == -1){perror("execvp failed"); exit(1);}
cerr << "getting to here3?" << endl;
	    if(write(communicateFailure[1], &failure, sizeof(failure)) < 0)
		{perror("write communicateFailure failured"); exit(1);}
cerr << "getting to here4?" << endl;
            //if(close(communicateFailure[0]) == -1){perror("close communicateFailure[0] failed"); exit(1);}
            if(close(communicateFailure[1]) == -1){perror("close communicateFailure[1] failed"); exit(1);}
            //if(close(communicateInput[0]) == -1){perror("close communicateInput[0] failed"); exit(1);}
            //if(close(communicateInput[1]) == -1){perror("close communicateInput[1] failed"); exit(1);}
            //if(close(communicateOutput[0]) == -1){perror("close communicateOutput[0] failed"); exit(1);}
            //if(close(communicateOutput[1]) == -1){perror("close communicateOutput[1] failed"); exit(1);}
          }
          else{ //parent process
cerr << "entering the parent process" << endl;
	    if(close(communicateFailure[1]) == -1){perror("close communicateFailure[1] failed"); exit(1);}
            int r = wait(NULL);
            if(r == -1){perror("wait failed"); exit(1);}
	    cerr << "got back from the child process" << endl;
	    if(read(communicateFailure[0], &failure, sizeof(failure)) == -1)
		{perror("read communicateFailure failed"); exit(1);}
cerr << "is my problem here?" << endl;
	    if(close(communicateFailure[0]) == -1){perror("close communicateFailure[0] failed"); exit(1);}
	    //if(close(communicateFailure[1]) == -1){perror("close communicateFailure[1] failed"); exit(1);}
	    if(close(communicateInput[0]) == -1){perror("close communicateInput[0] failed"); exit(1);}
	    if(close(communicateInput[1]) == -1){perror("close communicateInput[1] failed"); exit(1);}
	    if(close(communicateOutput[0]) == -1){perror("close communicateOutput[0] failed"); exit(1);}
	    if(close(communicateOutput[1]) == -1){perror("close communicateOutput[1] failed"); exit(1);}
	    if(failure == 0){}
	    else{failure = -1;}
          }
        }
        else{ // fork failed
          perror("fork failed");
          exit(1);
        }
	if(dup2(saveSTDIN, 0) == -1){perror("dup2 saveSTDIN failure"); exit(1);}
        if(dup2(saveSTDOUT, 1) == -1){perror("dup2 saveSTDOUT failure"); exit(1);}
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
    //char **talkBetweenChunks = new char*[commandLine.length() + 1];
    for(unsigned i = 0; i < commandLine.length() + 1; i++){ 
	list[i] = '\0';
	//talkBetweenChunks[i] = '\0';
    }

    //determines where connectors are and what they are
    vector<int> connectorsUsed;
    ConnectorsUsed(commandLine, connectorsUsed);
    //if //if the last object is a connector get more input

    //int communicateOutput[2];
    int r = pipe(communicateOutput);
    if(r == -1){perror("pipe error"); exit(1);}

    //string outputFile = "outputFile";
    //int previousOutput = creat(outputFile.c_str(), 0644);
    //close(previousOutput);

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
      if(lineConnectorNum == 0){cout << "first chunk" << endl; run = true;}
		/*cout << "entering line connector num == 0" << endl;
		if(connectorsUsed.size() > 0){
			if((connectorsUsed.at(lineConnectorNum) == INPUTREDIR) || 
				(connectorsUsed.at(lineConnectorNum) == OUTPUTREDIR)){
					run = true;
			} //first element
			else{cout << "entering else" << endl; run = true;}
		}
		else{run = true;}
      }*/
      /*else if((lineConnectorNum < connectorsUsed.size())){
	cout << "entering lineNum < size" << endl; 
		if((connectorsUsed.at(lineConnectorNum) == INPUTREDIR) ||
                	(connectorsUsed.at(lineConnectorNum) == OUTPUTREDIR)){
			unsigned iterator = 0;
                	while(list[iterator] != NULL){
                        	talkBetweenChunks[iterator] = list[iterator];
                                iterator++;
                        }
			run = false;
		}
	}*/
	else if(((lineConnectorNum - 1) >= 0) && (lineConnectorNum - 1 < connectorsUsed.size())){
		cout << "entering linNum - 1 >= 0" << endl;
                /*if((connectorsUsed.at(lineConnectorNum - 1) == INPUTREDIR) ||
                	(connectorsUsed.at(lineConnectorNum - 1) == OUTPUTREDIR)){
                	char **switchArrays = new char*[commandLine.length() + 1];
                	for(unsigned i = 0; i < commandLine.length() + 1; i++){
                        	switchArrays[i] = list[i];
                        	list[i] = talkBetweenChunks[i];
                        	talkBetweenChunks[i] = switchArrays[i];
                	}
                	run = true;
      		}*/
		if((connectorsUsed.at(lineConnectorNum - 1) == ANDCONNECTOR) && (success == true)){
                	cout << "processed as &&: success == true" << endl;
                	run = true;
		} //&&
      		else if((connectorsUsed.at(lineConnectorNum - 1) == ANDCONNECTOR) && (success == false)){
			if((lineConnectorNum - 2 >= 0) && (lineConnectorNum - 2 <= connectorsUsed.size())){
                		if(lineConnectorNum - 2 > 0){
                        		if(connectorsUsed.at(lineConnectorNum - 2) == ORCONNECTOR){
                                		cout << "processed as &&, previous connector == ||, success == false" << endl; 
                                		run = true;
					}
                        		else{run = false;}
                		}
                		else{run = false;} 
			}
			else{run = false;}
      		}
      		else if((connectorsUsed.at(lineConnectorNum - 1) == ORCONNECTOR) && (success == true)){
                	cout << "processed as ||: success == true" << endl; 
                	run = false;
		} //||
      		else if((connectorsUsed.at(lineConnectorNum - 1) == ORCONNECTOR) && (success == false)){
                	cout << "processed as ||: success == false" << endl;
                	run = true;
		}
      		else if(connectorsUsed.at(lineConnectorNum - 1) == SEMICOLON){
			cout << "processed as ;" << endl; run = true;
		} //;
      		else{
			cout << "untracked connector: connect " << connectorsUsed.at(lineConnectorNum - 1) << ", success " << success << endl; 
			run = true;
		}
      }
      int failure = 0;
      if(run == true){
	  //cout << "entering run statement" << endl;
	  failure = Run(list, /*talkBetweenChunks, */connectorsUsed, lineConnectorNum/*, communicateOutput*/);
      }
      indvCommands = strtok_r(NULL, connectors, &currCmmdLine);
      lineConnectorNum++; //moving to the next connector in array
      if(failure == -1){success = false;}

      if(indvCommands == NULL){break;}
    }
    delete [] charCmmdLine;
    delete [] list;
    //delete [] talkBetweenChunks;
    //close(previousOutput);
  }


  return 0;
}
