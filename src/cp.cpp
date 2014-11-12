#include <iostream>
#include "Timer.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

void outputTime(double wall, double user, double system)
{
	cout << "	wall time: " << wall << endl;
	cout << "	user time: " << user << endl;
	cout << "	system time: " << system << endl;
	return;
}

int main(int argc, char* argv[])
{
	string input, output;
	bool method = false;

	for(int i = 1; i < argc; i++)
	{
		if(i == 1)
			input = argv[i];
		else if(i == 2)
			output = argv[i];
		else if(i == 3)
			method = argv[i];
		else
		{
			cout << "error" << endl;
			exit(1);
		}
	}

	struct stat statbuf;
	stat(argv[1],&statbuf);
	if(S_ISDIR(statbuf.st_mode)){perror("Error: input is a directory"); exit(1);}

	stat(argv[2],&statbuf);
	if(S_ISDIR(statbuf.st_mode)){perror("Error: output is a directory"); exit(1);}

	int fout = open(output.c_str(),O_WRONLY);
	if(fout != -1){perror("Error: output filename already exists"); exit(1);}
	else{
		int fout2 = open(output.c_str(),O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP);
		if(fout2 == -1){perror("Error: could not create output file");exit(1);}

		int tmp = close(fout2);
		if(tmp == -1){perror("Error: could not close output file");exit(1);}
	}

	char c = '\0';
	if(method)
	{
		Timer t;
		double wall,user,system;
		t.start();
		std::fstream in;
		std::fstream out;

		in.open(input.c_str());
		out.open(output.c_str());

		while(in.good())
		{
			c = in.get();
			if(in.good())
				out.put(c);
		}
		in.close();
		out.close();
		t.elapsedTime(wall,user,system);
		cout << "Single Char Stream Times: " << endl;
		outputTime(wall,user,system);
	}
	
	if(method)
	{
		Timer t2;
		double wall,user,system;
		t2.start();
		char buf[1];

		int fdi = open(input.c_str(),O_RDONLY);
		if(fdi == -1){perror("open input failed"); exit(1);}

		int fdo = open(output.c_str(),O_WRONLY);
		if(fdo == -1){perror("open output failed"); exit(1);}

		int num = 0;

		while((num = read(fdi,buf,1))){
			if(num == -1){perror("read input failed"); exit(1);}
			int wnum = write(fdo,buf,1);
			if(wnum == -1){perror("write output failed"); exit(1);}

		}
		int tmp = close(fdi);
		if(tmp == -1){perror("fdi close failed"); exit(1);}
		
		tmp = close(fdo);
		if(tmp == -1){perror("fdo close failed"); exit(1);}

		t2.elapsedTime(wall,user,system);
		cout << "Single Char Unix Call Times: " << endl;
		outputTime(wall,user,system);
	}
	
	Timer t3;
	double wall,user,system;
	t3.start();
	char buff[BUFSIZ];
	int num2 = 0;
	
	int fdi2 = open(input.c_str(),O_RDONLY);
	if(fdi2 == -1){ perror("open fdi2 failed"); exit(1);}

	int fdo2 = open(output.c_str(),O_WRONLY);
	if(fdo2 == -1){ perror("open fdo2 failed"); exit(1);}
	
	while((num2 = read(fdi2,buff,BUFSIZ))){
		if(num2 == -1){ perror("fdi2 read failed"); exit(1);}
		int wnum2 = write(fdo2,buff,num2);
		if(wnum2 == -1){ perror("fdo2 write failed"); exit(1);}
	}
	int tmp3 = close(fdi2);
	if(tmp3 == -1){perror("fdi2 close failed"); exit(1);}
	tmp3 = close(fdo2);
	if(tmp3 == -1){perror("fdo2 close failed"); exit(1);}
	t3.elapsedTime(wall,user,system);
	if(method){
		cout << "Buffer Unix Call Times: " << endl;
		outputTime(wall,user,system);
	}

	return 0;
}






