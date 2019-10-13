#include <iostream>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include "user_handler.h"

using namespace std;


user_t:: user_t(string u_id, string passwd, string ipv4, string port){
	this->u_id = u_id;
	this->passwd = passwd;
	this->ipv4 = ipv4;
	this->port = port;
	this->status = false;//denotes current process is the one real user
}

bool user_t::user_exists(){
	fstream fs("Metadata/Users/u_ids/" +u_id, ios::in);
	if(fs.is_open()){
		fs.close();
		return true;
	}
	else{
		return false;
	}
}
int user_t::create_user(){
	if(!user_exists()){
		fstream ofs("Metadata/Users/u_ids/" +u_id, ios::out | ios::trunc);
		ofs<<passwd<<"\n";
		ofs<<0<<"\n";//active status
		ofs<<ipv4<<"\n";
		ofs<<port<<"\n";
		ofs.close();
		string pathname("Metadata/Users/user_data/"); 
		pathname += u_id;
		if(mkdir(pathname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0){
			return DIR_NULL;
		}	
		return SUCC;
	}
	else{
		return INV_USR;	
	}
}
int user_t::login_user(){
	string valid_passwd;
	int active_status;
	// string active_ipv4;
	// string active_port;

	if(user_exists()){
		fstream fs ("Metadata/Users/u_ids/"+u_id);
		
		//read correct values from user file
		fs>>valid_passwd;
		fs>>active_status;//active status
		// fs>>active_ipv4;//prev logged ip
		// fs>>active_port;//prev logged port

		if(valid_passwd.compare(this->passwd)==0){
			if(active_status == 0){
				this->status = 1;
				// active_ipv4 = this->ipv4;
				// active_port = this->port;
				
				//move put head to begin
				fs.seekp(0,ios::beg);
				fs<<passwd<<"\n";
				fs<<this->status<<"\n";
				fs<<this->ipv4<<"\n";
				fs<<this->port<<"\n";
				
				fs.close();
				return SUCC;
			}
			else{//if user is already active
				fs.close();
				return PRV_LOG;
			}
		}
		else{//invalid passwd
			fs.close();
			return INC_PSSWD;
		}
	}
	else{
		return LOGON_FAILED;
	}
}
//s
int user_t::logout(){
	string valid_passwd;
	int active_status;

	if(this->status){
		ofstream ofs("Metadata/Users/u_ids/"+u_id);
		ofs<<this->passwd<<"\n";
		ofs<<0<<"\n";	
		ofs<<this->ipv4<<"\n";
		ofs<<this->port<<"\n";
		ofs.close();
		return SUCC;
	}	
	else{
		return LOGON_FAILED;
	}
}
