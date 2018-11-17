#include "Client.h"
#include "Server.h"
/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: main.cpp - Program entry point
--
-- PROGRAM: Linux Console Chat Room
--
-- DATE: Mar 25, 2018
--
-- DESIGNER: Alex Xia, Juliana French
--
-- PROGRAMMER: Alex Xia
--
-- NOTES:
-- This program is a Linux console application for running a TCP client & multiplexed server chat room. It uses the select
-- function to multiplex messages coming in from different clients and sends them to each other. 
-- On starting the program, user will be prompted to select a mode [S/C] (case-insensitive), S-Server, C-Client, or E to exit
-- program. Below is a brief summary of what happens in each mode. More details on how client/server works are found in 
-- Client/Server.cpp
--
-- detailed instructions on commands will be printed to stdout in either mode.
-- In Client mode:
--   	- User will be prompted for a ip & port to connect to
--		- Only works if a server is already running at that ip & port
--		- Reading from server will be done on a background thread
--		- User can enter commands at any time to stdin, or messages to be sent
--
-- In Server mode:
--   	- User will be prompted for a port to run on
--		- Reading from clients & making connections will be done on a background thread
--		- Any packets from clients will be printed to stdout before being broadcasted to other clients 
--		- User can enter commands at any time to stdin but messages won't be sent
--
-- Note. Main function is borrowed from my 4981 assignment 2. Also program runs just fine on windows using cygwin.
----------------------------------------------------------------------------------------------------------------------*/
int main(void)
{
	while(true)
	{
		string input = PromptForString("* Server or client mode [S/C], or E to exit: ");
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
