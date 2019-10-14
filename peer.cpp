#include <iostream>
#include <string>

#include <algorithm>
#include <cstring>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "readfiles.h"
#define CMD_SIZE 512

using namespace std;
		// #create_user <user_id> <passwod>
		// #login <user_id> <passwd>
		// #create_group <group_id>
		// #join_group <group_id>
		// leave_group <group_id>
		// #list_requests <group_id>
		// #accept_request <group_id> <user_id>
		// #list_groups
		// list_files <group_id>
		// upload_file <file_path> <group_id>
		// download_file <group_id> <file_name> <destination_path>
		// #logout
		// stop share <group_id> <file_name>

void execute_cmd(int clientSocket, const char* ip_port){
	string cmd;
	string g_id;
	string u_id;
	string passwd;
	string ipv4;
	string port;
	char* return_msg= new char[CMD_SIZE];
	auto v = split(string(ip_port),':');	
	if(v.size() != 2){
		cerr<<"Error in parsing ip and port number";
		exit(-1);
	}
	ipv4 = v[0];//To Do:
	port = v[1];//error handling needs to be done
	int num_bytes;
	while(1){
		cout<<">";
		getline(cin, cmd );
		if(std::all_of(cmd.cbegin(),cmd.cend(),[](char c) { return std::isspace(c); }))	continue;

		write(clientSocket, cmd.c_str(), cmd.size());
		if((num_bytes = read(clientSocket, return_msg, CMD_SIZE))>0){
			fprintf(stdout, "%s", (char*)return_msg);
		}
		memset(return_msg, '\0', CMD_SIZE);
	}
}

//tries to connect to available trackers,
//returns tracker_no on success, otherwise -1
int connect_to_tracker(int* clientSocket, string filename, struct sockaddr_in* serverAddr){
	string port;
	string ipv4;
	int tracker_no = 1;
	while(tracker_no<=2){
		read_ipv4_port(filename, tracker_no, ipv4, port);
		bzero(serverAddr, sizeof(*serverAddr));
		serverAddr->sin_family = AF_INET;
		serverAddr->sin_port = htons(stoi(port));
		inet_pton(AF_INET, ipv4.c_str(), &(serverAddr->sin_addr));		
		if(connect(*clientSocket, (struct sockaddr *)serverAddr, (socklen_t)sizeof(*serverAddr))>=0){
			return tracker_no;
		}	
		tracker_no++;
	}
	return -1;
}
int main(int argc, char const *argv[]){
	int clientSocket;
	int tracker_no;
	string filename;
	struct sockaddr_in serverAddr;
	char const* ip_port;
	if(argc != 3){
		cerr<<"usage: "<<argv[0]<<" hostname:port tracker_info.txt\n";
		exit(1);
	}
	
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket <=0){
		cerr<<argv[0]<<": error: cannot create socket\n";
		exit(1);
	}
	//clear the struct
	bzero(&serverAddr, sizeof(serverAddr));
	
	//input server address
	filename = argv[2];
	if((tracker_no =connect_to_tracker(&clientSocket, filename, &serverAddr)) < 0){
		cerr<<"No trackers available\n";
		close(clientSocket);
		exit(1);
	}
	else{
		clog<<"Connected to Tracker: "<<tracker_no<<"\n";
	}
	ip_port = argv[1];
	write(clientSocket, ip_port, strlen(ip_port));
	execute_cmd(clientSocket, ip_port);
	close(clientSocket);
	return 0;
}