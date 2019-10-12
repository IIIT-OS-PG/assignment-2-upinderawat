#include <iostream>
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

#define CHUNK_SIZE 512
#define IP_ADDR_LEN 20
#define MAX_INC_CONN 5
typedef struct{
	int sockfd;
	struct sockaddr_in sock_addr;
	socklen_t addr_len;
}connection_t;

void * process_req(void* arg){
	connection_t* connection;
	int num_bytes;
	void* buff;
	buff = malloc(CHUNK_SIZE);
	
	if(!arg) pthread_exit(0);
	
	connection = (connection_t*)arg;
	while((num_bytes = read(connection->sockfd, buff, CHUNK_SIZE))>0){
		fprintf(stdout, "%s\n",(char*)buff);
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
