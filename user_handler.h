#define SUCC 0
#define LOGON_FAILED 1
#define INC_PSSWD 2//incorrect passwd
#define INV_USR 3//invalid user
#define PRV_LOG 4//already logged in

class user_t{
	private:
		//active implicitly => user exists
		bool status;//is_user_authenticated
		std::string u_id;
		std::string passwd;
		std::string ipv4;
		std::string port;
	
	public:
	user_t(std::string, std::string, std::string, std::string);
	std::string get_uid(){
		return u_id;
	}
	std::string get_ipv4(){
		return ipv4;
	}
	std::string get_port(){
		return port;
	}
	bool user_exists();
	int create_user();
	int login_user();
	int logout();
};

