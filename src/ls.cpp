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
#include<pwd.h>
#include<grp.h>
#include<time.h>

using namespace std;

const int NUM_FLAGS = 3;
const int FLAG_a = 0;
const int FLAG_l = 1;
const int FLAG_R = 2;
const char FLAG_SYMBOL = '-';
const int SCREEN_SIZE = 65;
const int EMPTY_SPACE_BETWEEN_WORDS = 1;
const string MONTH[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
const char READ = 'r';
const char WRITE = 'w';
const char EXECUTE = 'x';


void whatFlags(int *flag_array, int argc, char **argv){
	for(int i = 0; i < argc; i++){
		if(argv[i][0] == FLAG_SYMBOL){
			for(unsigned int j = 0; j < strlen(argv[i]); j++){
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

void lOutput(vector<string> &names, unsigned longest_name, string curr_dir){
	struct stat buf;
	int total_blocks = 0;

	for(unsigned int i = 0; i < names.size(); i++){
		string full_name = curr_dir + "/" + names.at(i);
		int error = stat(full_name.c_str(), &buf);
		if(error == -1){perror("stat failed"); exit(1);};
		total_blocks += buf.st_blocks;
	}

	cout << "total " << total_blocks/2 << endl;

	for(unsigned int i = 0; i < names.size(); i++){
		string full_name = curr_dir + "/" + names.at(i);
		int error = stat(full_name.c_str(), &buf);
		if(error == -1){perror("stat failed"); exit(1);}
		if(S_ISDIR(buf.st_mode) == true){cout << "d";}
		else{cout << FLAG_SYMBOL;}
		if(buf.st_mode & S_IRUSR){cout << READ;}
		else{cout << FLAG_SYMBOL;}
		if(buf.st_mode & S_IWUSR){cout << WRITE;}
		else{cout << FLAG_SYMBOL;}
		if(buf.st_mode & S_IXUSR){cout << EXECUTE;}
		else{cout << FLAG_SYMBOL;}
		if(buf.st_mode & S_IRGRP){cout << READ;}
		else{cout << FLAG_SYMBOL;}
		if(buf.st_mode & S_IWGRP){cout << WRITE;}
		else{cout << FLAG_SYMBOL;}
		if(buf.st_mode & S_IXGRP){cout << EXECUTE;}
		else{cout << FLAG_SYMBOL;}
		if(buf.st_mode & S_IROTH){cout << READ;}
		else{cout << FLAG_SYMBOL;}
		if(buf.st_mode & S_IWOTH){cout << WRITE;}
		else{cout << FLAG_SYMBOL;}
		if(buf.st_mode & S_IXOTH){cout << EXECUTE;}
		else{cout << FLAG_SYMBOL;}
		cout << " " << buf.st_nlink;
		struct passwd *user = getpwuid(buf.st_uid);
		if(user == NULL){perror("getpwuid failed"); exit(1);}
		cout << " " << user->pw_name;
		struct group *gr = getgrgid(buf.st_gid);
		if(gr == NULL){perror("getgrgid failed"); exit(1);}
		cout << " " << gr->gr_name;
		cout << " " << buf.st_size;
		struct tm *clock = localtime(&(buf.st_mtime));
		if(clock == NULL){perror("gmtime failed"); exit(1);}
		clock->tm_isdst = -1;
		cout << " " << MONTH[clock->tm_mon];
		cout << " " << clock->tm_mday;
		if(clock->tm_hour < 10){cout << " 0" << clock->tm_hour;}
		else{cout << " " << clock->tm_hour;}
		if(clock->tm_min < 10){cout << ":0" << clock->tm_min;}
		else{cout << ":" << clock->tm_min;}
		cout << " " << names.at(i);
		cout << endl;
	}
	return;
}

void non_lOutput(vector<string> &names, unsigned longest_name){
	unsigned int space_per_word = longest_name + EMPTY_SPACE_BETWEEN_WORDS;
	unsigned int words_per_line = SCREEN_SIZE/(space_per_word);
	unsigned int total_lines = (names.size()/words_per_line) + 1;
	for(int i = words_per_line; i > 0; i--){
		if(names.size()%i < total_lines){words_per_line = i; break;}
	}
	total_lines = (names.size()/words_per_line);
	unsigned int line = 0;
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

void listContents(string dirName, int *flag_array){
	if(flag_array[FLAG_R] == 1){cout << dirName << ":" << endl;}
	vector<string> names;
        DIR *dirp = opendir(dirName.c_str());
	if(dirp == NULL){perror("opendir failed"); exit(1);}
	dirent *direntp;

	unsigned int i = 0;
	unsigned longest_name = 0;
	while((direntp = readdir(dirp))){
		if(direntp == NULL){perror("readdir failed"); exit(1);}
		if((flag_array[FLAG_a] == 0) && (direntp->d_name[0] == '.'))
			{continue;}
		names.push_back(direntp->d_name);
		if(names.at(i).size() > longest_name) {
			longest_name = names.at(i).size();
		}
		i++;
	}
	sort(names.begin(), names.end(), compareWordsInsensitiveToCase);
	if(flag_array[FLAG_l] == 1) {lOutput(names, longest_name, dirName);}
        else {non_lOutput(names, longest_name);}
	closedir(dirp);
	if(flag_array[FLAG_R] == 1){
		cout << endl;
		if(flag_array[FLAG_a] == 1){i = 2;}
		else{i = 0;}
		while(i < names.size()){
			struct stat buf;
			string possible_dir = dirName + "/" + names.at(i);
			int error = stat(possible_dir.c_str(), &buf);
			if(error == -1){perror("stat failed"); exit(1);}
			if(S_ISDIR(buf.st_mode) == true){
				listContents(possible_dir, flag_array);
			}
			i++;
		}
	}
	return;
}

int main(int argc, char **argv){
	int flag_array[NUM_FLAGS];
	for(int i = 0; i < NUM_FLAGS; i++) {flag_array[i] = 0;}
	whatFlags(flag_array, argc, argv);
	string dirName = ".";
	listContents(dirName, flag_array);
	return(0);
}
