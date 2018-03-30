#ifndef SERVER_H
#define SERVER_H

#ifndef BACKLOG
#define BACKLOG 5
#endif

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
		void BroadcastMessages(ClientInfo*, vector<string>&);
		bool HandleCommand();
		int GetFileDescIndex(const int, ClientInfo*);
		
		bool keepSelecting = true;
		ofstream chatLogFile;
		ClientInfo* connectedList;
};

#endif // SERVER_H
