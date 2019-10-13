#include <iostream>
#include <string>

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
#include "user_handler.h"
#include "group_handler.h"
using namespace std;
		// create_user <user_id> <passwod>
		// login <user_id> <passwd>
		// create_group <group_id>
		// join_group <group_id>
		// leave_group <group_id>
		// list_requests <group_id>
		// accept_request <group_id> <user_id>
		// list_groups
		// list_files <group_id>
		// upload_file <file_path> <group_id>
		// download_file <group_id> <file_name> <destination_path>
		// logout
		// stop share <group_id> <file_name>

void execute_cmd(const char* ip_port){
	string cmd;
	string g_id;
	string u_id;
	string file_path;
	string passwd;
	string ipv4;
	string port;
	int status;
	auto v = split(string(ip_port),':');	
	if(v.size() != 2){
		cerr<<"Error in parsing ip and port number";
		exit(-1);
	}
	ipv4 = v[0];//To Do:
	port = v[1];//error handling needs to be done
	user_t* user=nullptr;
	struct group_t group;
	while(1){
		cout<<">";
		cin>>cmd;
		if(cmd.compare("create_user")==0){
			cin>>u_id>>passwd;
			if(user != nullptr){
				cerr<<"Not permitted in Login state\nLogout first or start process from different shell\n";
				
				continue;
			}
			user= new user_t(u_id, passwd, ipv4, port);
			if((status = user->create_user()) == SUCC){
				cout<<"New u_id:"<<u_id<<" created\n";
			}
			else if(status == DIR_NULL){
				cerr<<"Not able to create user_data dir";
			}
			else{
				cerr<<"User with u_id: "<<u_id<<" already exists!!\n";
			}
			delete user;
			user = nullptr;
		}
		else if(cmd.compare("login")==0){
			cin>>u_id>>passwd;
			if(user != nullptr){
				cerr<<"User logged in from different account\n";
				cerr<<"Logout first or start process from different shell\n";
				continue;
			}
			user = new user_t(u_id, passwd, ipv4, port);
			status = user->login_user();
			switch(status){
				case SUCC:
					cout<<"User: "<<u_id<<" logged in\n";
					break;
				case PRV_LOG:
					cout<<"User: "<<u_id<<" already logged in. You spy process >:#\n";
					delete user;
					user = nullptr;
					break;
				case INC_PSSWD:
					cout<<"Invalid Password\n";
					delete user;
					user = nullptr;
					break;//To Do 
					//promt for passwd again
				case LOGON_FAILED:
					cout<<"User: "<<u_id<<" No such user exists!!\n";
					delete user;
					user = nullptr;
					break;
			}
		}
		else if(cmd.compare("create_group")==0){
			cin>>g_id;
			if(user == nullptr){
				cerr<<"Login required\n";
			}
			else{
				if((status = group.create_group(g_id, user->get_uid())) == SUCC){
					cout<<"Group: "<<g_id<<" created\n";
				}
				else if(status == PRV_GRP){
					cerr<<"Group: "<<g_id<<" already exists!!\n";
				}
				else if(status == DIR_NULL){
					cerr<<"Group: "<<g_id<<" metadata still exists in group_data\n";
				}
			}
		}
		else if(cmd.compare("join_group")==0){
			
		}
		else if(cmd.compare("leave_group")==0){
			
		}
		else if(cmd.compare("list_requests")==0){
			
		}
		else if(cmd.compare("accept_request")==0){
			
		}
		else if(cmd.compare("list_groups")==0){
			
		}
		else if(cmd.compare("list_files")==0){
			
		}
		else if(cmd.compare("upload_file")==0){
			
		}
		else if(cmd.compare("download_file")==0){
			
		}
		else if(cmd.compare("logout")==0){
			if(user != nullptr and (status = user->logout() != LOGON_FAILED)){
				cout<<"Logged out \n";
			}
			else{
				cerr<<"Login required\n";
			}
			delete user;
			user = nullptr;
		}
		else if(cmd.compare("stop_share")==0){
			
		}
		
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
	execute_cmd(ip_port);
	close(clientSocket);
	return 0;
}