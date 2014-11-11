#include<iostream>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/types.h>
#include<errno.h>
#include<sys/stat.h>
#include<vector>
#include<string>
#include<algorithm>
#include<iomanip>

using namespace std;

const int NUM_FLAGS = 3;
const int FLAG_a = 0;
const int FLAG_l = 1;
const int FLAG_R = 2;
const char FLAG_SYMBOL = '-';
const int SCREEN_SIZE = 65;
const int EMPTY_SPACE_BETWEEN_WORDS = 1;


void whatFlags(int *flag_array, int argc, char **argv){
	//for(int i = 0; i < NUM_FLAGS; i++) flag_array[i] = 0;
	for(int i = 0; i < argc; i++){
		if(argv[i][0] == FLAG_SYMBOL){
			for(int j = 0; j < strlen(argv[i]); j++){
				switch(argv[i][j]){
					case 'a':
						flag_array[FLAG_a] = 1; continue;
					case 'l':
						flag_array[FLAG_l] = 1; continue;
					case 'R':
						flag_array[FLAG_R] = 1; continue;
					default:
						continue;
				}
			}
		}
	}
}

struct caseInsensitive{
	bool operator ()(char partOfWordOne, char partOfWordTwo){
		return tolower(partOfWordOne) < tolower(partOfWordTwo);
	}
};

bool compareWordsInsensitiveToCase(const string &wordOne, const string &wordTwo){
	if((*wordOne.begin() == '.') && (*wordTwo.begin() == '.'))
		return lexicographical_compare(wordOne.begin()+1, wordOne.end(), wordTwo.begin()+1, wordTwo.end(), caseInsensitive());
	else if(*wordOne.begin() == '.')
		return lexicographical_compare(wordOne.begin()+1, wordOne.end(), wordTwo.begin(), wordTwo.end(), caseInsensitive());
        else if(*wordTwo.begin() == '.')
	        return lexicographical_compare(wordOne.begin(), wordOne.end(), wordTwo.begin()+1, wordTwo.end(), caseInsensitive());
	else
		return lexicographical_compare(wordOne.begin(), wordOne.end(), wordTwo.begin(), wordTwo.end(), caseInsensitive());
}

void lOutput(vector<string> &names, unsigned longest_name){
	struct stat buf;
	for(int i = 0; i < names.size(); i++){
		stat(names.at(i).c_str(), &buf);
		cout << buf.st_uid << " ";
		cout << buf.st_gid;
		cout << endl;
	}
	return;
}

void non_lOutput(vector<string> &names, unsigned longest_name){
	int space_per_word = longest_name + EMPTY_SPACE_BETWEEN_WORDS;
	int words_per_line = SCREEN_SIZE/(space_per_word);
	int total_lines = (names.size()/words_per_line) + 1;
	for(int i = words_per_line; i > 0; i--){
		if(names.size()%i < total_lines){words_per_line = i; break;}
	}
	total_lines = (names.size()/words_per_line);
	int line = 0;
	while(line < total_lines){
		int i = 0;
		while(i < (names.size())){
			if((i+line) < names.size()){
				cerr << setw(space_per_word) << left << names.at(i+line);
			}
			i += total_lines;
		}
		cout << endl;
		line++;
	}
		
	return;
}

void listContents(char *dirName, vector<string> &names, int *flag_array){
        DIR *dirp = opendir(dirName);
	dirent *direntp;

	int i = 0;
	unsigned longest_name = 0;
	while(direntp = readdir(dirp)){
		if((flag_array[FLAG_a] == 0) && (direntp->d_name[0] == '.'))
			{continue;}
		names.push_back(direntp->d_name);
		if(names.at(i).size() > longest_name) {
			longest_name = names.at(i).size();
		}
		i++;
	}
	sort(names.begin(), names.end(), compareWordsInsensitiveToCase);
	if(flag_array[FLAG_l] == 1) {lOutput(names, longest_name);}
        else {non_lOutput(names, longest_name);}
	closedir(dirp);
}

int main(int argc, char **argv){
	int flag_array[NUM_FLAGS];
	for(int i = 0; i < NUM_FLAGS; i++) flag_array[i] = 0;
	whatFlags(flag_array, argc, argv);
	char dirName[] = ".";
	vector<string> names;
	listContents(dirName, names, flag_array);
	return(0);
}
