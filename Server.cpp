#include "Server.h"
#ifndef BACKLOG
#define BACKLOG 5
#endif

Server::Server()
{
	int port = PromptForInteger("Enter a new port:");
	// create socket
	int listenSocket;
	if ((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		Die("Failed to create listenSocket");

	// set socket flag SO_REUSEADDR so port can be resused after exit
	if (setsockopt (listenSocket, SOL_SOCKET, SO_REUSEADDR, &port, sizeof(port)) == -1)
		Die("Failed to set listenSocket to flag SO_REUSEADDR");

	// Bind an address to the socket

	struct sockaddr_in serverInetAddress;
	memset(&serverInetAddress, 0, sizeof(struct sockaddr_in));
	serverInetAddress.sin_family = AF_INET;
	serverInetAddress.sin_port = htons(port);
	serverInetAddress.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client

	// tcp calls for bind b4 accept
	if (bind(listenSocket, (struct sockaddr *)&serverInetAddress, sizeof(serverInetAddress)) == -1)
		Die("Failed to bind listenSocket to port:" + port);

	listen(listenSocket, BACKLOG);
	int lastAllocIndex	= listenSocket;
   	lastAllocIndex = -1;

   	// runs 4096 times
	// for (i = 0; i < FD_SETSIZE; i++)
	// 	client[i] = -1;             // -1 indicates available entry

    fd_set allset;
 	FD_ZERO(&allset);
   	FD_SET(listenSocket, &allset);

   	return;
	// while (TRUE);
}

Server::~Server()
{
    //dtor
}
