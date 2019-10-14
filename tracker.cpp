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
	int num_bytes;
	void* cmd;
	cmd = malloc(CMD_SIZE);
	
	if(!arg) pthread_exit(0);

	string g_id;
	string u_id;
	string passwd;
	char* ipv4_port;
	string ipv4;
	string port;
	string cmd_s;
	string return_msg;
	int status;

	user_t* user=nullptr;
	connection = (connection_t*)arg;
	ipv4_port = (char*)malloc(CMD_SIZE);

	//read ip and port sent from client
	if(read(connection->sockfd, ipv4_port, CMD_SIZE) <= 0){
		cerr<<"Unresponsive peer\n";
	}
	else{
		auto v = split(string(ipv4_port),':');//To Do error handling
		ipv4 = v[0];
		port = v[1];
		
		while((num_bytes = read(connection->sockfd, cmd, CMD_SIZE))>0){
			// fprintf(stdout, "%s\n",(char*)cmd);
			cmd_s = (char*)cmd;
			auto args = split(cmd_s, ' ');
			if(args.size() <= 0){
				continue;
			}
			if(args[0].compare("create_user")==0){
				u_id = args[1];
				passwd = args[2];
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
			write(connection->sockfd, return_msg.c_str(), return_msg.size());
			memset(cmd, '\0', CMD_SIZE);
		}
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
