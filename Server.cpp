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

	struct sockaddr_in serverInetAddress;
	memset(&serverInetAddress, 0, sizeof(struct sockaddr_in));
	serverInetAddress.sin_family = AF_INET;
	serverInetAddress.sin_port = htons(port);
	serverInetAddress.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client

	// Bind an address to the socket
	// tcp calls for bind b4 accept
	if (bind(listenSocket, (struct sockaddr *)&serverInetAddress, sizeof(serverInetAddress)) == -1)
		Die("Failed to bind listenSocket to port:" + port);

	listen(listenSocket, BACKLOG);

   	// return;
   	this->SelectLoop(listenSocket);
	// while (TRUE);
}

void Server::SelectLoop(int listenSocket)
{
	int lastAllocFileDesc = listenSocket;
   	lastAllocFileDesc = -1;

   	// runs 4096 times
   	int tooLong = FD_SETSIZE;
	// for (int i = 0; i < FD_SETSIZE; i++)
		// clientList[i] = -1;             // -1 indicates available entry
	int clientList[] = { -1};

    fd_set allFileDescSet;
    fd_set readFileDescSet;
 	FD_ZERO(&allFileDescSet);
   	FD_SET(listenSocket, &allFileDescSet);
	while (true)
	{
   		readFileDescSet = allFileDescSet;               // structure assignment
		struct timeval timeout;
		timeout.tv_sec = 2;             /* 2 second timeout */
		timeout.tv_usec = 0;

		int readReadyCount = select(lastAllocFileDesc + 1, &readFileDescSet, NULL, NULL, &timeout);

		if (FD_ISSET(listenSocket, &readFileDescSet)) // new client connection
		{
			struct sockaddr_in clientInetAddress;
			int clientSocket; 
			int tempLen = sizeof clientInetAddress; //not used ever again
			if ((clientSocket = accept(listenSocket, (struct sockaddr *) &clientInetAddress, &tempLen) ) == -1)
				Die("Failed to accept:");

		    printf(" Remote Address:  %s\n", inet_ntoa(clientInetAddress.sin_addr));
		    close(clientSocket);
		    return;
		    // for (int i = 0; i < FD_SETSIZE; i++)
		 //    for (int i = 0; i < 1; i++)
			// 	if (clientList[i] < 0)
			// 	{
			// 		clientList[i] = clientSocket;	// save descriptor
			// 		break;
			// 	}
			// 	if (i == FD_SETSIZE)
			// 	{
			// 		Die("Too many clients\n");
			// 	}

			// 	FD_SET (clientSocket, &allFileDescSet);     // add new descriptor to set
			// 	if (clientSocket > lastAllocFileDesc)
			// 		lastAllocFileDesc = clientSocket;	// for select

			// 	if (i > maxi)
			// 		maxi = i;	// new max index in clientList[] array

			// 	if (--readReadySubset <= 0)
			// 		continue;	// no more readable descriptors
			// }

			// for (i = 0; i <= maxi; i++)	// check all clients for data
			// {
			// 	if ((sockfd = clientList[i]) < 0)
			// 		continue;

			// 	if (FD_ISSET(sockfd, &readFileDescSet))
			// 	{
			// 		// bp = buf;
			// 		bytes_to_read = BUFLEN;
			// 		// while ((n = read(sockfd, &buf, bytes_to_read)) > 0)
			// 		// {
			// 		n = read(sockfd, &buf, bytes_to_read);
			// 			// bp += n;
			// 			// bytes_to_read -= n;
			// 		// }
			// 		write(sockfd, &buf, BUFLEN);   // echo to client

			// 		if (n == 0) // connection closed by client
			// 		{
			// 			printf(" Remote Address:  %s closed connection\n", inet_ntoa(clientInetAddress.sin_addr));
			// 			close(sockfd);
			// 			FD_CLR(sockfd, &allFileDescSet);
			// 			clientList[i] = -1;
			// 		}
						            				
			// 		if (--readReadySubset <= 0)
			// 			break;        // no more readable descriptors
			// 	} //endif
			// } // endfor
   		} // endif	
   	}
}

Server::~Server()
{
    //dtor
}
