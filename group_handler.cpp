#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <sys/stat.h>
#include "group_handler.h"
using namespace std;

bool group_t:: group_exists(string g_id){
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
int group_t:: create_group(string g_id, string u_id){
	if(!group_exists(g_id)){
		string pathname = "Metadata/Groups/g_ids/"+g_id;
		fstream ofs(pathname.c_str(), ios::out| ios::trunc);
		ofs<<u_id<<"\n";
		ofs.close();	
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

