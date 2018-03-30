#include "Client.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Client.cpp - Client mode class
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
--		bool HandleNewline(const int);
--		void ReadServerMessage(const int);
--
-- Instance vars:
--		bool keepSelecting = true; - controls background thread
--		ofstream chatLogFile; - file to print chat log to
--
-- NOTES:
-- This class controls everything that happens in program running in Client mode. It reads user input in a loop on the main
-- thread, and handles it depending either as a command or a message. 
-- Commands start with a '/', and they affect execution. Below are all supported client commands
-- 		/disconnect or /d : disconnect from server & return to main to choose a different mode (or exit program there)
--		/help or /h : prints message listing all client (and server) commands
--		/exit : an exit option (E) is already in main loop. Will instead print msg to tell user to call /disconnect
-- Commands are neither sent to server, nor printed to chatlog file if that option is enabled(see below). 	
-- 
-- Messages from other client instances connected to the same server will be continously read & printed to stdout from a
-- background thread. User can enter commands or messages at anytime.
--
-- On entering client mode, user will directly connect to a server via TCP; user must enter the Ip and port of a valid server
-- or the connection times out and the program dies. If connected successfully, user is then given the option (Y/N) to print 
-- the chatlog (message-only) to a file, and to enter a file name if they choose yes.
--   
-- Note that packet messsages from client to server only contain the message and a timestamp. The server appends the Ip 
-- and sends it back. This is due to only the server can easily get client ip address, and a getCurrentIP function couldn't be
-- implemented until later into development.  
----------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION Client constructor
--
-- DATE: Mar 29 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: Client::Client()
--
-- RETURNS: N/A
--
-- NOTES:
-- Call this function to run a client once. Handles the client code run on the main thread. 
-- Creates a socket to connect / listen to server packets.
-- Only print welcome message and prompt user for chatlog options if connection succeed. If not, exits program.
-- Then starts a background thread running a select loop to listen for packets, while main thread listens from user input.
-- If user disconnects at any point, the background thread is stopped, joined, and program returns to main.
----------------------------------------------------------------------------------------------------------------------*/
Client::Client()
{
    string serverIp = PromptForString("(Enter a server ip): ");
    int serverPort = PromptForInteger("(Enter a server port): ");
    int serverSocket;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		Die("Failed to create socket to server");

	struct hostent *serverHostent;
	//if ((serverHostent = gethostbyaddr(serverIp.c_str(), sizeof(struct in_addr), AF_INET)) == NULL)
	if ((serverHostent = gethostbyname(serverIp.c_str())) == NULL)
		Die("Failed to get host, unknown server address");
		
	struct sockaddr_in serverInetAddress;
	memset(&serverInetAddress, 0, sizeof(struct sockaddr_in));
	serverInetAddress.sin_family = AF_INET;
	serverInetAddress.sin_port = htons(serverPort);
	serverInetAddress.sin_addr.s_addr = inet_addr(serverIp.c_str());
	// memcpy(serverHostent->h_addr, (char *)&serverInetAddress.sin_addr, serverHostent->h_length); 
	// ^ connect error: Address family not supported by protocol

	// Connecting to the server (this blocks)
	if ( connect (serverSocket, (struct sockaddr *)&serverInetAddress, sizeof(serverInetAddress)) < 0)
		Die("Failed to connect to server");
	fcntl(serverSocket, F_SETFL, O_NONBLOCK);
	
	string fileName = PromptForChatlog();
	if (fileName != "")
		chatLogFile.open(fileName);
		
	//PrintWelcomeMessage("CLIENT", serverHostent->h_name, chatLogFile.is_open());
	PrintWelcomeMessage("CLIENT", chatLogFile.is_open(), GetCurrentIP(), serverHostent->h_name, serverPort);
	thread clientThread(&Client::SelectLoop, this, serverSocket);
	while(this->HandleNewline(serverSocket));
	
	keepSelecting = false;
	clientThread.join();
	close(serverSocket);
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
-- INTERFACE: void Client::SelectLoop(const int serverSocket)
--				serverSocket : file descriptor for socket connected to server
-- RETURNS: N/A
--
-- NOTES:
-- The client version of a select loop. This function should be run on a background thread. 
-- Uses the select API call with a 2 second timeout to poll server socket for packets from server. 
-- Then calls ReadServerMessage to recv and print the packet data from server.
-- Main thread can stop this function by setting keepSelecting to false.
----------------------------------------------------------------------------------------------------------------------*/
void Client::SelectLoop(const int serverSocket)
{
	fd_set serverSocketSet; //server socket is only socket in this set
	fd_set tempSocketSet;
	FD_ZERO(&serverSocketSet);
	FD_SET(serverSocket, &serverSocketSet);
	while (keepSelecting)
	{
	 	tempSocketSet = serverSocketSet;
		struct timeval timeout;
		timeout.tv_sec = 2;             
		timeout.tv_usec = 0;

		select(serverSocket + 1, &tempSocketSet, NULL, NULL, &timeout);

		// stuff to read from server
		if (FD_ISSET(serverSocket, &tempSocketSet)) 
			this->ReadServerMessage(serverSocket);
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION HandleNewline
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: bool HandleNewline(const int serverSocket)
--				serverSocket : file descriptor for socket connected to server
-- RETURNS: bool : continue calling this function if true : return to main if false
--
-- NOTES:
-- Call this function in a loop to continuously get input from user. This function should be run on the main thread. 
-- User input could be a command (start with /), or a message (anything else).
-- Only messages are sent to server.
-- Accepted commands:
--		/help or /h : prints a help message 
--		/disonnect or /d : returns false so the loop calling this function will break
--		/exit : not actually valid command. but user may think was it was, so tells them to call /disconnect instead
-- Any messages sent are also printed to a chat log, if file opened.
-- After handling a line of input and user hasn't entered /disconnect, return true.
----------------------------------------------------------------------------------------------------------------------*/
bool Client::HandleNewline(const int serverSocket)
{
	string line;
	cout << "(Me): ";
	cout.flush();
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
		else
		{
			cout << "* Unsupported cmd '" << cmd << "'. Use /help to see all commands." << endl;
		}
	}
	else
	{
		string noIpInfoPacket = GetTimestampedPacket(line);
		SendPacket(serverSocket, noIpInfoPacket);
		if (chatLogFile.is_open())
			chatLogFile << "<Me>" << noIpInfoPacket << endl;
	}
	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION ReadServerMessage
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: void ReadServerMessage(const int serverSocket)
--				serverSocket : file descriptor for socket connected to server
-- RETURNS: void
--
-- NOTES:
-- Call this function to receive a packet from server. This function should be run on a background thread. 
-- Should only be called if select has confirmed there's data to be read from server socket.
-- If select found server socket to be read ready but no bytes are actually read,
-- server has disconnected, and program will exit.
----------------------------------------------------------------------------------------------------------------------*/
void Client::ReadServerMessage(const int serverSocket)
{
	int packetBytesRemaining = BUFLEN; //used for setting read size
	int bytesJustRead = BUFLEN; // used for ++bufferTail
	int bytesTotalRead = 0; // if nothing read ->bytesJustRead=0, if everything read->bytesJustRead=0
	char* recvBuffer = (char*)calloc(BUFLEN, sizeof(char)); 
	char* bufferTail = recvBuffer;
	while ((bytesJustRead = read(serverSocket, bufferTail, packetBytesRemaining)) > 0)
	{
		bufferTail += bytesJustRead;
		bytesTotalRead += bytesJustRead;
		packetBytesRemaining -= bytesJustRead;
	} 
	if (bytesTotalRead > 0)
	{
		// packet should be this format: <198.98.62.21><10:58:44>hello world
		cout << recvBuffer << endl;
		if (chatLogFile.is_open())
			chatLogFile << recvBuffer << endl;
	}
	else 
	{
		free(recvBuffer);
		Die("Server port has closed. Client disconnecting...");
	}
	//string constructor is deep copy, this is fine
	free(recvBuffer);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION Client destructor
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: Client::~Client()
--				
-- RETURNS: N/A
--
-- NOTES:
-- Stops background thread by setting a boolean controlling its loop to false.
-- Just a safeguard; boolean should already be set to false in client constructor.
----------------------------------------------------------------------------------------------------------------------*/
Client::~Client()
{
    keepSelecting = false;
}
