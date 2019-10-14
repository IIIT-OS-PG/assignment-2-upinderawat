#define SUCC 0
#define PRV_GRP 1
#define USR_EXISTS 2
#define INV_USR 3
#define INV_GRP 4
#define PER_DEN 5
#define DUP_REQ 6
#include <vector>

#define DIR_NULL -1
struct group_t{

	//All helper functions assume that g_id, u_id passed
	//to them are valid and corresponding group and user
	//exists as such.
	//CAUTION: Think twice before exposing them directly
	bool group_exists(const std::string&);
	bool is_member(const std::string&, const std::string&, const std::string&);
	bool is_admin(const std::string&, const std::string&);
	void add_user(const std::string&, const std::string&);

	//API
	int create_group(const std::string&, const std::string&);
	std::vector<std::string> list_groups();
	int join_group(const std::string& , const std::string& );
	int accept_request(const std::string&, const std::string &, const std::string&);
	int list_requests(const std::string& , const std::string& );

};

