#include "Server.h"
#ifndef BACKLOG
#define BACKLOG 5
#endif

Server::Server()
{
	int port = PromptForInteger("(Enter a new port):");
	// create socket
	int listenSocket;
	if ((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		Die("Failed to create listenSocket");

	// set socket flag SO_REUSEADDR so port can be resused after exit
	if (setsockopt (listenSocket, SOL_SOCKET, SO_REUSEADDR, &port, sizeof(port)) == -1)
		Die("Failed to set listenSocket to flag SO_REUSEADDR");

	struct sockaddr_in serverInetAddress;
	memset(&serverInetAddress, 0, sizeof(struct sockaddr_in));
	serverInetAddress.sin_family = AF_INET;
	serverInetAddress.sin_port = htons(port);
	serverInetAddress.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client

	// Bind an address to the socket
	// tcp calls for bind b4 accept
	if (bind(listenSocket, (struct sockaddr *)&serverInetAddress, sizeof(serverInetAddress)) == -1)
		Die("Failed to bind listenSocket to port:" + port);
	
	string fileName = PromptForChatlog();
	if (fileName != "")
		chatLogFile.open(fileName);

	listen(listenSocket, BACKLOG);
	PrintWelcomeMessage("SERVER", chatLogFile.is_open(), GetCurrentIP(), GetCurrentIP(), port);
	thread serverThread(&Server::SelectLoop, this, listenSocket);
	while(this->HandleCommand());
	keepSelecting = false;
	serverThread.join();
	close(listenSocket);
}

void Server::SelectLoop(const int listenSocket)
{
	int lastAllocFileDesc = listenSocket;

   	// can run 4096 times on some machines
	// does not incl. listen socket. indexes are fileDescValue - 4 *only if not localhost, use GetFileDescIndex instead
	ClientInfo clientList[FD_SETSIZE];
	this->connectedList = clientList;
	for (int i = 0; i < FD_SETSIZE; i++)
		clientList[i].FileDesc = -1;             // -1 indicates available entry

    fd_set allFileDescSet;
    fd_set readFileDescSet; //incl. listening socket
 	FD_ZERO(&allFileDescSet);
   	FD_SET(listenSocket, &allFileDescSet);
	while (keepSelecting)
	{
   		readFileDescSet = allFileDescSet;               // structure assignment
		struct timeval timeout;
		timeout.tv_sec = 0;             
		timeout.tv_usec = 10;

		int readReadyCount = select(lastAllocFileDesc + 1, &readFileDescSet, NULL, NULL, &timeout);

		// if listenSocket is in set of readReady fds, a new client needs to be accepted
		if (FD_ISSET(listenSocket, &readFileDescSet))  
		{
			int newClientFileDesc = this->AddNewClient(listenSocket, clientList);
			FD_SET (newClientFileDesc, &allFileDescSet);     // add new descriptor to set
			if (newClientFileDesc > lastAllocFileDesc)
				lastAllocFileDesc = newClientFileDesc;
			// no more readable descriptors
			if (--readReadyCount <= 0)
				continue;	
		}
		vector<string> listOfPackets;
		// read all clients data so fds are write ready
		int lastAllocFileDescIndex = this->GetFileDescIndex(lastAllocFileDesc, clientList);
		for (int i = 0; i <= lastAllocFileDescIndex; i++)
		{
			int clientSocket;
			if ((clientSocket = clientList[i].FileDesc) == -1)
				continue;

			if (FD_ISSET(clientSocket, &readFileDescSet))
			{
				int bytesJustRead = this->ReadClientMessage(clientSocket, clientList, listOfPackets);
				if (bytesJustRead == 0) // connection closed by client
				{
					int clientSocketIndex = this->GetFileDescIndex(clientSocket, clientList);
					cout << "* Client disconnected: "<< clientList[clientSocketIndex].IpAddress << endl;
					if (chatLogFile.is_open())
						chatLogFile << "* Client disconnected: "<< clientList[clientSocketIndex].IpAddress << endl;
					close(clientSocket);
					FD_CLR(clientSocket, &allFileDescSet);
					clientList[clientSocketIndex].FileDesc = -1;
				}						            				
				// no more readable descriptors
				if (--readReadyCount <= 0)
					break;        
			} //endif
		} // endfor
		// write to all clients
		this->BroadcastMessages(clientList, listOfPackets);						            				
   	} //endwhile
}

Server::~Server()
{
	keepSelecting = false;
}

/* returns fd of new client. */
/* accepts new client & adds it to clientList */
int Server::AddNewClient(const int listenSocket, ClientInfo* clientList)
{
	struct sockaddr_in clientInetAddress;
	int clientSocket; 
	socklen_t tempLen = sizeof clientInetAddress; //not used ever again
	if ((clientSocket = accept((unsigned int)listenSocket, (struct sockaddr *) &clientInetAddress, &tempLen) ) == -1)
		Die("Failed to accept:");
    cout << "* New client connected:" << inet_ntoa(clientInetAddress.sin_addr) << endl;
	if (chatLogFile.is_open())
		chatLogFile << "* New client connected:" << inet_ntoa(clientInetAddress.sin_addr) << endl;
	
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);
    // incre thru clientList, set first available fd to new client
    // cannot use lastAlloc, bc previous client might have disconnected & left an opening
	// since i < FD_SETSIZE, can never exceed client limit
    for (int i = 0; i < FD_SETSIZE; i++)
    {
		if (clientList[i].FileDesc == -1)
		{
			clientList[i].FileDesc = clientSocket;	// save descriptor
			clientList[i].IpAddress = string(inet_ntoa(clientInetAddress.sin_addr));
			break;
		}		    	
    }
	return clientSocket;
}

int Server::ReadClientMessage(const int clientSocket, ClientInfo* clientList, vector<string>& listOfPackets)
{
	int packetBytesRemaining = BUFLEN; //used for setting read size
	int bytesJustRead; // used for ++bufferTail
	int bytesTotalRead = 0; // if nothing read ->bytesJustRead=0, if everything read->bytesJustRead=0
	char* recvBuffer = (char*)calloc(BUFLEN, sizeof(char)); 
	char* bufferTail = recvBuffer;
	while ((bytesJustRead = read(clientSocket, bufferTail, packetBytesRemaining)) > 0)
	{
		bufferTail += bytesJustRead;
		bytesTotalRead += bytesJustRead;
		packetBytesRemaining -= bytesJustRead;
	} 
	if (bytesTotalRead > 0)
	{
		//create packetized version of buffer on stack	
		string noIpInfoPacket = string(recvBuffer);
		int clientSocketIndex = this->GetFileDescIndex(clientSocket, clientList);
		string completedPacket = GetIpedPacket(clientList[clientSocketIndex].IpAddress, noIpInfoPacket);
		listOfPackets.push_back(completedPacket); 
	}
	//string constructor is deep copy, this is fine
	free(recvBuffer); 
	return bytesTotalRead;
}

void Server::BroadcastMessages(ClientInfo* clientList, vector<string>& listOfPackets)
{	
	//cant go in forloop sadly
	for (vector<string>::iterator msg = listOfPackets.begin() ; msg != listOfPackets.end(); ++msg)
	{
		//another for loop thru each client here
		for (int i = 0; i < FD_SETSIZE; ++i)
		{
			if (clientList[i].FileDesc != -1)
			{
				string packetIp = GetIpFromPacket(*msg);
				if (packetIp == clientList[i].IpAddress)
				{
					cout << *msg << endl;
					if (chatLogFile.is_open())
						chatLogFile << *msg << endl;
				}
				else
				{
					SendPacket(clientList[i].FileDesc,*msg);	
				}
			}			
		}
	}	
	listOfPackets.clear();
}

bool Server::HandleCommand()
{
	string line;
	cout << "(Enter cmd at any time): " << endl;
	getline(cin, line);
	
	char cmdIndicator = '/';
	if (line[0] == cmdIndicator)
	{
		string cmd = line.substr(0, line.find(" "));
		if (cmd == "/help" || cmd == "/h")
		{
			PrintHelpMessage();
		}
		else if (cmd == "/disconnect" || cmd == "/d")
		{
			return false;		
		}
		else if (cmd == "/exit")
		{
			cout << "* Please /disconnect first. " << endl;		
		}
		else if (cmd == "/clients")
		{
			if (this->connectedList != nullptr)
				for(int i = 0; i < FD_SETSIZE; ++i)
				{
					if (connectedList[i].FileDesc != -1 )
						cout << "- " << connectedList[i].IpAddress << endl;
				}
			else 
				cout << "No clients connected" << endl;
		}
		else
		{
			cout << "* Unsupported cmd '" << cmd << "'. Use /help to see all commands." << endl;
		}
	}
	return true;
}

/*theoretically can convert fileDesc to index in clientList by
 * fileDesc-4, but not on localhost since client takes up additional FileDesc.  */
int Server::GetFileDescIndex(const int fileDesc, ClientInfo* clientList)
{
	for (int i = 0; i < FD_SETSIZE; ++i)
	{
		if (clientList[i].FileDesc == fileDesc)
			return i;
	}
	return -1;
}