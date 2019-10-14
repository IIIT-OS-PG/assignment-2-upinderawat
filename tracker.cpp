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
#include <pthread.h>
#include <vector>

#include "readfiles.h"
#include "user_handler.h"
#include "group_handler.h"

#define CMD_SIZE 512
#define MAX_INC_CONN 5

typedef struct{
	int sockfd;
	struct sockaddr_in sock_addr;
	socklen_t addr_len;
}connection_t;

void * process_req(void* arg){
	connection_t* connection;
	
	if(!arg) pthread_exit(0);

	string g_id;
	string u_id;
	string passwd;
	string ipv4;
	string port;
	string cmd_s;
	string return_msg="";
	int status;

	group_t group;
	char* ipv4_port = new char[CMD_SIZE];
	char* cmd = new char[CMD_SIZE];
	user_t* user=nullptr;
	connection = (connection_t*)arg;

	//read ip and port sent from client
	if(read(connection->sockfd, ipv4_port, CMD_SIZE) <= 0){
		cerr<<"Unresponsive peer\n";
	}
	else{
		auto v = split(string(ipv4_port),':');//To Do error handling
		ipv4 = v[0];
		port = v[1];
		
		while(read(connection->sockfd, cmd, CMD_SIZE)>0){
			// fprintf(stdout, "%s\n",(char*)cmd);
			return_msg = "";
			cmd_s = (char*)cmd;
			auto argv = split(cmd_s, ' ');
			if(argv[0].compare("create_user")==0){
				if(argv.size() != 3){
					return_msg = "Incorrect command\n";
				}
				else{
					u_id = argv[1];
					passwd = argv[2];
					if(user != nullptr){
						return_msg = "Not permitted in Login state\nLogout first or start process from different shell\n";
					}
					else{	
						user= new user_t(u_id, passwd, ipv4, port);
						if((status = user->create_user()) == SUCC){
							return_msg = "New u_id:" + u_id + " created\n";
						}
						else if(status == DIR_NULL){
							return_msg = "Not able to create user_data dir\n";
						}
						else{
							return_msg = "User with u_id: " + u_id + " already exists!!\n";
						}
						delete user;
						user = nullptr;
					}
				}
			}//end of command
			else if(argv[0].compare("login")==0){
				if(argv.size() != 3){
					return_msg = "Incorrect command\n";
				}
				else{
					u_id = argv[1];
					passwd = argv[2];
					if(user != nullptr){
						return_msg = "User logged in from different account\n";
						return_msg += "Logout first or start process from different shell\n";
					}
					else{
						user = new user_t(u_id, passwd, ipv4, port);
						status = user->login_user();
						switch(status){
							case SUCC:
								return_msg = "User: "+u_id+" logged in\n";
								break;
							case PRV_LOG:
								return_msg = "User: " + u_id + " already logged in. You spy process >:#\n";
								delete user;
								user = nullptr;
								break;
							case INC_PSSWD:
								return_msg = "Invalid Password\n";
								delete user;
								user = nullptr;
								break;//To Do 
								//promt for passwd again
							case LOGON_FAILED:
								return_msg = "User: " + u_id + " No such user exists!!\n";
								delete user;
								user = nullptr;
								break;
						}
					}
				}
			}//end of command
			else if(argv[0].compare("create_group")==0){
				if(argv.size() != 2){
					return_msg = "Incorrect command\n";
				}
				else{	
					g_id = argv[1];
					if(user == nullptr){
						return_msg = "Login required\n";
					}
					else{
						if((status = group.create_group(g_id, user->get_uid())) == SUCC){
							return_msg = "Group: " + g_id + " created\n";
						}
						else if(status == PRV_GRP){
							return_msg = "Group: " + g_id + " already exists!!\n";
						}
						else if(status == DIR_NULL){
							return_msg = "Group: " + g_id + " metadata still exists in group_data\n";
						}
					}
				}
			}//end of command
			else if(argv[0].compare("join_group")==0){
				if(argv.size() != 2){
					return_msg = "Incorrect command\n";
				}
				else{
					g_id = argv[1];
					if(user == nullptr){
						return_msg = "Login required\n";
					}
					else{
						if((status=group.join_group(g_id, user->get_uid())) == SUCC){
							return_msg = "Group join request sent to admin\n";
						}
						else if(status == INV_GRP){
							return_msg = "Invalid group request\n";
						}
						else if(status == DUP_REQ){
							return_msg = "Duplicate group request\n";					
						}
						else if(status == USR_EXISTS){
							return_msg = "User: " + u_id + " already member\n";
						}
					}
				}
			}//end of command
			else if(argv[0].compare("leave_group")==0){
				
			}
			else if(argv[0].compare("list_requests")==0){
				if(argv.size() != 2){
					return_msg = "Incorrect command\n";
				}
				else{
					g_id = argv[1];
					if(user == nullptr){
						return_msg = "Login required\n";
					}
					else{
						std::pair<int, std::vector<std::string>> it = group.list_requests(g_id, user->get_uid());
						status = it.first;
						if(status == SUCC){
							if(!(it.second).empty()){
								for(auto u : it.second){
									return_msg += u;
								}
							}
							else{
								return_msg = "No pending Requests\n";
							}
						}
						else if(status == PER_DEN){
							return_msg = "User "+user->get_uid()+": insufficient privelleges\n";
						}
						else if(status == INV_GRP){
							return_msg = "Group "+g_id+": doesn't exist!!\n";					
						}
					}
				}
			}//end of command
			else if(argv[0].compare("accept_request")==0){
				if(argv.size() != 3){
					return_msg = "Incorrect command\n";
				}
				else{
					g_id = argv[1];
					u_id = argv[2];
					if(user == nullptr){
						return_msg = "Login required\n";
					}
					else{
						if((status = group.accept_request(g_id, user->get_uid(), u_id)) == SUCC){
							return_msg = "Group "+g_id+": "+"added "+u_id+"\n";
						}
						else if(status == INV_GRP){
							return_msg = "Group "+g_id+": doesn't exist!!\n";
						}
						else if(status == PER_DEN){
							return_msg = "User "+user->get_uid()+": insufficient privelleges\n";
						}
						else if(status == INV_USR){
							return_msg = "No request from"+u_id+"\n";
						}
					}
				}
			}
			else if(argv[0].compare("list_groups")==0){
				if(argv.size() != 1){
					return_msg = "Incorrect command\n";
				}
				else{
					std::vector<std::string> v;
					if(user == nullptr){
						return_msg = "Login required\n";
					}
					else{
						v = group.list_groups();
						if(v.empty()){
							return_msg = "No group Exists!!\n";
						}
						else{
							for(auto a: v){
								return_msg += a + "\n";
							}
						}
					}
				}
			}
			else if(argv[0].compare("list_files")==0){		
			}
			else if(argv[0].compare("upload_file")==0){
			}
			else if(argv[0].compare("download_file")==0){
			}
			else if(argv[0].compare("logout")==0){
				if(argv.size() != 1){
					return_msg = "Incorrect command\n";
				}
				else{
					if(user != nullptr and (status = user->logout() != LOGON_FAILED)){
						return_msg = "Logged out \n";
					}
					else{
						return_msg = "Login required\n";
					}
					delete user;
					user = nullptr;
				}
			}
			else if(argv[0].compare("stop_share")==0){	
			}
			else{
				return_msg = "Incorrect command\n";
			}
			write(connection->sockfd, return_msg.c_str(), return_msg.size());
			memset(cmd, '\0', CMD_SIZE);
			}//end of while
		}
		close(connection->sockfd);
		free(connection);
		pthread_exit(0);
	}
	int main(int argc, char const *argv[]){
		int welcomeSocket;
		string filename;
		int tracker_no;
		struct sockaddr_in serverAddr;
		connection_t* connection;
		string port;
		string ipv4;

		pthread_t thread;

		if(argc != 3){
			cerr<<"usage: "<<argv[0]<<" tracker_info.txt tracker_no\n";
			exit(1);
		}
		//reads ip and port of given tracker into port and ipv4
		filename = argv[1];
		tracker_no = atoi(argv[2]);
		if(read_ipv4_port(filename, tracker_no, ipv4, port)<0){
			cerr<<filename<<" Valid filename: tracker_info.txt\n";
			exit(1);
		}
		welcomeSocket = socket(AF_INET, SOCK_STREAM, 0);
		if(welcomeSocket <=0){
			cerr<<argv[0]<<": error: cannot create socket\n";
		exit(1);
	}

	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(stoi(port));
	inet_pton(AF_INET, ipv4.c_str(), &(serverAddr.sin_addr));

	bind(welcomeSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

	if(listen(welcomeSocket, MAX_INC_CONN)==0){
		printf("Listening\n");
	}
	else{
		perror("Listen error\n");
	}
	while(1){
		connection = (connection_t*)malloc(sizeof(connection_t));
		connection->sockfd = accept(welcomeSocket, (struct sockaddr* )&connection->sock_addr, &connection->addr_len);
		if(connection->sockfd <= 0){
			free(connection);
		}
		else{	
			pthread_create(&thread, NULL, process_req, (void *)connection);
			// process_req((void *)&welcomeSocket);
			// pthread_join(thread1, NULL);
			pthread_detach(thread);
		}
	}
}
