#include "Client.h"

Client::Client()
{
    string serverIp = PromptForString("Enter a server ip:");
    int serverPort = PromptForInteger("Enter a server port:");
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

	printf("Connected: Server Name: %s\n", serverHostent->h_name);
	close(serverSocket);
}

Client::~Client()
{
    //dtor
}
