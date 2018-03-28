#include "Client.h"

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

	// Connecting to the server
	if (connect (serverSocket, (struct sockaddr *)&serverInetAddress, sizeof(serverInetAddress)) == -1)
		Die("Failed to connect to server");
	
	fcntl(serverSocket, F_SETFL, O_NONBLOCK);
	PrintWelcomeMessage("CLIENT", serverHostent->h_name);
	
	thread clientThread(&Client::SelectLoop, this, serverSocket);
	while(this->HandleNewline(serverSocket))
	
	keepSelecting = false;
	clientThread.join();
	close(serverSocket);
}

void Client::SelectLoop(const int serverSocket)
{
	fd_set serverSocketSet; //server socket is only socket in this set
	while (keepSelecting)
	{
	 	FD_ZERO(&serverSocketSet);
	   	FD_SET(serverSocket, &serverSocketSet);
		struct timeval timeout;
		timeout.tv_sec = 2;             
		timeout.tv_usec = 0;

		select(serverSocket + 1, &serverSocketSet, NULL, NULL, &timeout);

		// stuff to read from server
		if (FD_ISSET(serverSocket, &serverSocketSet)) 
			this->ReadServerMessage(serverSocket);
	}
}

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
		SendPacket(serverSocket, line);
	}
	return true;
}

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
		//    v this should be in the packet already
		cout << "<otherClientIP>:" << recvBuffer << endl;
	}
	else 
	{
		free(recvBuffer);
		Die("Server port has closed. Client disconnecting...");
	}
	//string constructor is deep copy, this is fine
	free(recvBuffer);
}

Client::~Client()
{
    keepSelecting = false;
}
