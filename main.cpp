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
-- This program is a Linux console application which <@TODO>
-- On starting the program, user will be prompted to select a mode [S/C] (not case-sensitive), S-Server or C-Client.
-- Below is a brief summary of what happens in each mode; more details on how client/server works are found in Client/Server.cpp
--
-- In Client mode:
--   	- User will be prompted for a <@TODO>
--
-- In Server mode:
--   	- User will be prompted for a <@TODO>
--
-- If a user did not enter necessary information correctly, an error msg will be printed. Or if the send/receive
-- was not successful, the program will exit.
--
-- Note. Program runs just fine on windows using cygwin.
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
