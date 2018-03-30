#include "Server.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Server.cpp - Server mode class
--
-- PROGRAM: Linux Console Chat Room
--
-- DATE: Mar 25, 2018
--
-- DESIGNER: Alex Xia, Juliana French
--
-- PROGRAMMER: Alex Xia
--
-- FUNCTIONS:
--		void SelectLoop(const int);
--		int AddNewClient(const int, ClientInfo*);
--		int ReadClientMessage(const int, ClientInfo*, vector<string>&);
--		void BroadcastMessages(ClientInfo*, vector<string>&);
--		bool HandleCommand();
--		int GetFileDescIndex(const int, ClientInfo*);
--
-- INSTANCE VARIABLES:
--	 	bool keepSelecting = true; - controls background thread
--		ofstream chatLogFile; - file to print chat log to
--		ClientInfo* connectedList; - pointer to list of clients, only needed for /clients command
--	
-- STRUCT:
--		struct ClientInfo
--		{
--			int FileDesc;
--			string IpAddress;
--		};
--		Used by only Server class in an array to keep track of all client IPs & file descriptors
--	
-- NOTES:
-- This class controls everything that happens in program running in Server mode. It reads user input in a loop on the main
-- thread, and handles it depending either as a command or a message. 
-- Commands start with a '/', and they affect execution. Below are all supported client commands
-- 		/disconnect or /d : disconnect from server & return to main to choose a different mode (or exit program there)
--		/help or /h : prints message listing all client (and server) commands
--		/clients : a server-only command. Lists out all connected clients
--		/exit : an exit option (E) is already in main loop. Will instead print msg to tell user to call /disconnect
-- Commands are not printed to chatlog file if that option is enabled(see below). 	
-- 
-- Messages from clients connected to this server will be continously read & printed to stdout from a
-- background thread. Although the server can read in messages from the user, it does nothing with them.
-- User can enter commands or messages at anytime.
--
-- On entering server mode, user will bind to a TCP socket and listen for TCP connections; user must enter a port to bind
-- If socket created & bound successfully, user is then given the option (Y/N) to print 
-- the chatlog (message-only) to a file, and to enter a file name if they choose yes.
-- 
-- Messages are sent to every client but the message author/commenter.  
-- Note that packet messsages from client to server contains the message, a timestamp, and the commenter's IP. 
-- Packets from clients do not contain the ip in the msg body, but the server appends the Ip and sends it back. 
-- This is because the server can easily get client ip addresses, and a getCurrentIP function couldn't be
-- implemented until later into development.  
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION SelectLoop
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: void Server::SelectLoop(const int listenSocket)
--				listenSocket : file descriptor for server's listen socket
-- RETURNS: N/A
--
-- NOTES:
-- The server version of a select loop. This function should be run on a background thread. 
-- Uses the select API call with a 10 nanosecond timeout to poll server listening socket for new client connections,
-- and sockets to all connected clients for new packets.   
-- After reading from all read-ready the sockets, broadcasts the list of messages to all clients except their authors.
-- Main thread can stop this function by setting keepSelecting to false.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION Server destructor
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: Server::~Server()
--				
-- RETURNS: N/A
--
-- NOTES:
-- Stops background thread by setting a boolean controlling its loop to false.
-- Just a safeguard; boolean should already be set to false in server constructor.
----------------------------------------------------------------------------------------------------------------------*/
Server::~Server()
{
	keepSelecting = false;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION AddNewClient
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: int AddNewClient(const int listenSocket, ClientInfo* clientList)
--				listenSocket : file descriptor for server's listen socket
--				clientList : pointer to array of of ClientInfo structs, stores clients' socket descriptors & IPs
-- RETURNS: int : file descriptor of the newly connected client
--
-- NOTES:
-- As with all functions involved with receiving & listening, this should be called on a background thread.
-- Calls accept on a new client & prints a msg telling user a client has connected (and logs it to chat log file if opened)
-- Sets the new client socket to non-blocking and adds its file descriptor and Ip address to an array of ClientInfo structs.
-- Then returns the file descriptor.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION ReadClientMessage
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: int ReadClientMessage(const int clientSocket, ClientInfo* clientList, vector<string>& listOfPackets)
--				listenSocket : file descriptor for server's listen socket
--				clientList : pointer to array of of ClientInfo structs, stores clients' socket descriptors & IPs
--				listOfPackets : reference to string vector of packet data/messages
-- RETURNS: int : bytes read from socket
-- NOTES:
-- As with all functions involved with receiving & listening, this should be called on a background thread.
-- Should only be called if select has confirmed there's data to be read from client socket.
-- Reads and appends packet data as string to listOfPackets to be broadcasted.
-- If no bytes are read, client has disconnected, and it's removed from clientList array.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION BroadcastMessages
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: void BroadcastMessages(ClientInfo* clientList, vector<string>& listOfPackets)
--				clientList : pointer to array of of ClientInfo structs, stores clients' socket descriptors & IPs
--				listOfPackets : reference to string vector of packet data/messages
-- RETURNS: void
-- NOTES:
-- This should be called on a background thread, after reading from all read-ready sockets.
-- Should only be called after there're no more data waiting on any client sockets (already read from them).
-- Loops through listOfPackets and sends each packet as string message to every connected client except the author.  
-- If a client in the list is the author (should only have 1 author), print out the message for user to view,
-- and to the chat log if it's opened.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION HandleCommand
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: bool HandleCommand()
--
-- RETURNS: bool : continue calling this function if true : return to main if false
--
-- NOTES:
-- Call this function in a loop to continuously get input from user. This function should be run on the main thread. 
-- User input could be a command (start with /), or a message (anything else).
-- Note that server ignores any messages user enters, and commands are not printed to chatlog if one is opened.
-- Accepted commands:
--		/help or /h : prints a help message 
--		/disonnect or /d : returns false so the loop calling this function will break
--		/clients : a server-only command. Prints out all connected clients' IPs
--		/exit : not actually valid command. but user may think was it was, so tells them to call /disconnect instead
-- After handling a line of input and user hasn't entered /disconnect, return true.
----------------------------------------------------------------------------------------------------------------------*/
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
			{
				int clientCount = 0;
				for(int i = 0; i < FD_SETSIZE; ++i)
				{
					if (connectedList[i].FileDesc != -1 )
					{
						cout << "- " << connectedList[i].IpAddress << endl;
						clientCount++;
					}
				}
				if (clientCount == 0)
					cout << "No clients connected" << endl;
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION GetFileDescIndex
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: int GetFileDescIndex(const int fileDesc, ClientInfo* clientList)
--
-- RETURNS: bool : continue calling this function if true : return to main if false
--
-- NOTES:
-- Loops through array of ClientInfo objects, and returns the index of the IP associated with a socket, or -1 if
-- socket not found.
--
-- Allows program to be ran on localhost (client and host with same IP address and shared set of socket descriptors).
-- This function was created to fix a bug running a client & server on localhost. Original a socket/fileDesc can be 
-- converted to corresponding index in clientList by fileDesc-4. But since a localhost client would takes up additional FileDescs.
-- fileDesc-4=clientList index is no longer reliable on Server.  */
int Server::GetFileDescIndex(const int fileDesc, ClientInfo* clientList)
{
	for (int i = 0; i < FD_SETSIZE; ++i)
	{
		if (clientList[i].FileDesc == fileDesc)
			return i;
	}
	return -1;
}
