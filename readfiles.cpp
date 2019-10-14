#include "readfiles.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

vector<string> split(const string& s, char delimiter){
	vector<string> tokens;
	string token;
	istringstream tokenstream(s);
	while(getline(tokenstream, token, delimiter)){
		tokens.push_back(token);
	}
	return tokens;
}

int read_ipv4_port(string filename, int tracker_no, string& ip, string& port){
	int tracker;
	ifstream ifs(filename);
	if(ifs.is_open()){
		while(!ifs.eof()){
			ifs>>tracker>>ip>>port;
			if(tracker_no == tracker){
				break;
			}
		}
	}
	else{
		cerr<<filename<<"doesn't exist";
		return -1;
	}
	ifs.close();
	return 0;
}