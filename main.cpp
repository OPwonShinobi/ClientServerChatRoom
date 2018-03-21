#include "Client.h"
#include "Server.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: main.cpp - Program entry point
--
-- PROGRAM: System V Message Queue Console Network 
--
-- DATE: Mar 4, 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- NOTES:
-- This program is a Linux console application which implements the System V IPC message queue to send text files by
-- priority from a server instance to a client instance.
-- On starting the program, user will be prompted to select a mode [S/C] (not case-sensitive), S-Server or C-Client.
-- Below is a brief summary of what happens in each mode; more details on how client/server works are found in Client/Server.cpp 
--
-- In Client mode:
--   	- User will be prompted for a IPC key to work with (any random number will do). Both the client and the server
--		must use the same key to connect. 
--		- Then the client will be prompted for a priority[1/2/3]. The priority will be used by the server to determine how
--		quickly data is sent back. 
--		- The client is then prompted to enter a filename (a full path to the file would also work). That filename, along with
--		the client's type and priority, is sent as a msg to the server. If the server can open the file, its contents will be sent 		
--		back; if not, an error msg is sent.
--		- After all msgs meant for this client is received, client exits and the program will prompt user to run again,
--		in client or server mode.
--
-- In Server mode:
--   	- User will be prompted for a IPC key to work with (any random number will do). Both the client and the server
--		must use the same key to connect. 
--		- User is then prompted to enter the max clients to receive from. The server will then begin listen for msges from clients. 
--		After that number of clients are handled, server stops listening, closes the queue and the program will prompt user to run again, in client or 
--		server mode.
--		- For every new client that connects, server tries to open the file that client specifies, 
--		then sends back relevent data according to client's type & priority.   	
--		
-- If a user did not enter necessary information correctly, an error msg will be printed. Or if the send/receive
-- was not successful, the program will exit. 		
--
-- Note. Program runs just fine on windows using cygwin if in admin mode and cygserver is running.
----------------------------------------------------------------------------------------------------------------------*/
int main(void)
{
	while(true)
	{
		string input = PromptForInput("* Server or client mode [S/C], or E to exit:");
		if (tolower(input[0]) == 's')
		{
			Server server;
			continue;
		}
		if (tolower(input[0]) == 'c')
		{
			Client client;
			continue;
		}
		if (tolower(input[0]) == 'e' )
		{
			break;
		}
		cout << "* Please make sure first letter entered is s or c *" << endl;
	}	
}