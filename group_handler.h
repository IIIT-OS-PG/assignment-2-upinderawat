#define SUCC 0
#define PRV_GRP 1

#define DIR_NULL -1

struct group_t{
	// std::unordered_set<std::string> u_ids;

	// std::unordered_set<std::string> get_u_ids(){
	// 	return u_ids;
	// }
	bool group_exists(std::string);
	int create_group(std::string, std::string);
};

