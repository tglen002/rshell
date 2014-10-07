#include<stdio.h>
#include<string.h>
#include<iostream>


using namespace std;

int main()
{
  string commandLine;
  cout << "$ ";
  getline(cin,commandLine);
  cout << "output: " << commandLine << endl;
  char *charCmmdLine = new char[commandLine.length() + 1];
  strcpy(charCmmdLine, commandLine.c_str());
  char connectors[] = "&|;";
  char * indvCommands;
  indvCommands = strtok(charCmmdLine, connectors);
  while (indvCommands != NULL)
    { printf("%s\n", indvCommands); indvCommands = strtok(NULL,connectors);}
  delete [] charCmmdLine;

  return 0;
}
