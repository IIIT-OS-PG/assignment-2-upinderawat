#include <iostream>
#include <cstdio>//for rename, remvove functions
#include <fstream>
#include <string>
#include <unordered_set>
#include <sys/stat.h>
#include "group_handler.h"
#include <dirent.h>

#include <vector>
using namespace std;

//To Do:
//avoid sending duplicate join_group requests

bool group_t:: group_exists(const string& g_id){
	string pathname = "Metadata/Groups/g_ids/"+g_id;
	fstream f(pathname);
	if(f.is_open()){
		f.close();
		return true;
	}
	else{
		return false;
	}
}

bool group_t::is_member(const std::string& pathname, const std::string& g_id, const std::string& target_uid){
	ifstream ifs(pathname + g_id);
	std::string u_id;
	while(!ifs.eof()){
		ifs>>u_id;
		if(u_id.compare(target_uid) == 0){
			ifs.close();
			return true;
		}
	}
	ifs.close();
	return false;
}
//first user will be admin i.e first row
bool group_t::is_admin(const std::string& g_id, const std::string& u_id){
	string pathname = "Metadata/Groups/g_ids/"+g_id;
	string admin_uid;
	ifstream ifs(pathname);
	ifs>>admin_uid;
	ifs.close();
	if(admin_uid.compare(u_id) == 0){
		return true;
	}
	else{
		return false;
	}
}

void group_t:: add_user(const std::string& g_id, const std::string& u_id){
	string pathname = "Metadata/Groups/g_ids/"+g_id;
	ofstream ofs(pathname, ios::out| ios::app);
	ofs<<u_id<<"\n";
	ofs.close();
}
int group_t:: create_group(const string& g_id,const string& u_id){
	if(!group_exists(g_id)){
		//file to maintain list of members
		string pathname = "Metadata/Groups/g_ids/"+g_id;
		fstream ofs(pathname.c_str(), ios::out| ios::trunc);
		ofs<<u_id<<"\n";
		ofs.close();

		//create a file to maintain requests to that group
		pathname = "Metadata/Groups/g_ids_requests/"+g_id;
		ofs.open(pathname.c_str(), ios::out| ios::trunc);
		ofs.close();

		//this dir will hold all shared files chunk by groups
		pathname = "Metadata/Groups/group_data/"+g_id;
		if(mkdir(pathname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0){
			return DIR_NULL;
		}
		else{
			return SUCC;
		}		
	}
	else{
		return PRV_GRP;
	}
}
std::vector<std::string> group_t:: list_groups(){
	string pathname = "Metadata/Groups/g_ids/";
	std::vector<std::string> v;
	struct dirent *file;
	DIR *dir = opendir(pathname.c_str());

	if(dir == NULL){
		cerr<<"Directory: Metadata/Groups/g_ids/ does'nt exist\n";
	}
	else{
		while((file = readdir(dir)) != NULL){
			if(file->d_name[0] != '.')
				v.push_back(file->d_name);
		}
	}
	return v;
}
int group_t:: join_group(const std::string& g_id, const std::string& u_id){
	if(!group_exists(g_id)){
		return INV_GRP;
	}

	if(is_member("Metadata/Groups/g_ids/", g_id, u_id)){
		return USR_EXISTS;
	}
	else if(is_member("Metadata/Groups/g_ids_requests/", g_id, u_id)){
		return DUP_REQ;
	}
	else{
		ofstream ofs("Metadata/Groups/g_ids_requests/"+g_id, ios::out|ios::app);
		ofs<<u_id<<"\n";
		ofs.close();
		return SUCC;
	}
}
//perform user existential validation at caller 
//to avoid excess coupling
int group_t:: accept_request(const std::string& g_id, const std::string& admin_uid, const std::string& target_uid){
	string pathname= "Metadata/Groups/g_ids_requests/"+g_id;
	std::string u_id;
	bool found = false;
	if(!group_exists(g_id)){
		return INV_GRP;
	}
	if(!is_admin(g_id, admin_uid)){
		return PER_DEN;
	}
	ifstream ifs(pathname);
	fstream ofs("Metadata/Groups/g_ids_requests/temp.txt", ios::out| ios::trunc);
	while(!ifs.eof()){
		getline(ifs, u_id);
		if(u_id.compare(target_uid) == 0){
			found = true;
		}
		else
			ofs<<u_id<<"\n";
	}
	ofs.close();
	ifs.close();
	if(remove(pathname.c_str()) != 0){
		cerr<<"---group_handler: Remove file not succ\n";
		return PER_DEN;
	}
	if(rename("Metadata/Groups/g_ids_requests/temp.txt", pathname.c_str())!=0){
		cerr<<"---group_handler: Rename not succ\n";
		return PER_DEN;
	}
	if(found){	
		add_user(g_id, target_uid);
		return SUCC;
	}
	return INV_USR;
}
//To Do
//empty file check
int group_t :: list_requests(const std::string& g_id, const std::string& target_uid){
	string u_id;
	if(group_exists(g_id)){
		if(is_admin(g_id, target_uid)){
			ifstream ifs("Metadata/Groups/g_ids_requests/"+g_id);
			while(!ifs.eof()){
				getline(ifs,u_id);
				cout<<u_id<<"\n";
			}
			ifs.close();
			return SUCC;
		}
		else{
			return PER_DEN;
		}
	}	
	else{
		return INV_GRP;
	}
}