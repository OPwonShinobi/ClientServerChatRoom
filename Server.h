#ifndef SERVER_H
#define SERVER_H
#include "SharedUtils.h"

struct ClientInfo
{
	int FileDesc;
	string IpAddress;
};

class Server
{
    public:
        Server();
        virtual ~Server();

	private:
		void SelectLoop(const int);
		int AddNewClient(const int, ClientInfo*);
		int ReadClientMessage(const int, ClientInfo*, vector<string>&);
		void BroadCastMessages(ClientInfo*, vector<string>&);
		bool HandleCommand();
		int GetFileDescIndex(const int, ClientInfo*);
		
		bool keepSelecting = true;
		ofstream chatLogFile;
};

#endif // SERVER_H
