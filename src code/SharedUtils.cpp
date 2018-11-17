#include "SharedUtils.h"
/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: SharedUtils.cpp - Shared utilities functions
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
--		string PromptForString(const string);
--		int PromptForInteger(const string);
--		string PromptForChatlog();
--		void PrintWelcomeMessage(const string, const bool, const string, const string, const int);
--		void PrintHelpMessage();
--		string GetCurrentIP();
--		void Die(const string message);
--		void SendPacket(const int, const string);
--		string GetTimeString();
--		string GetIpFromPacket(const string);
--		string GetIpedPacket(const string, const string);
--		string GetTimestampedPacket(const string);
--		string GetEncryptedString(const string);
--		string GetDecryptedString(const string);
--	
-- NOTES:
-- To avoid redundancy the shared pseudo-static functions used by both Server and Client (and even main) are stored here.
----------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION PromptForInteger
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: int PromptForInteger(const string msg)
--					const string msg : msg to display to user
--
-- RETURNS: int : a non-zero (could be +ve or -ve) int parsed from user input
--
-- NOTES:
-- Displays a msg to a user and waits for a string input. Then parses an int from it using atoi (doesn't throw 
-- exceptions and returns 0 if no numbers found). Continue prompting with same msg until non-zero int parsed.
-- Then returns that parsed int.
----------------------------------------------------------------------------------------------------------------------*/
int PromptForInteger(const string msg)
{
	int intInput = 0;
	do
	{
		intInput = atoi(PromptForString(msg).c_str());
	}
	while (intInput == 0);
	return intInput;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION PromptForChatlog
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: string PromptForChatlog()
--
-- RETURNS: string : either a filename(open this file as chatlog), or an empty string(user wants no chatlog)
--
-- NOTES:
-- Prompts user to make a selection, whether they want to print chat log to a file or not (Y/N) (case-insensitive).
-- Continue prompting until either string begining with either Y or N is entered.
-- If user entered N, return a empty string. Should not open keep any chat logs.
-- If user enters Y, prompts them for a file name. 
-- If they entered a file name, return & use as chatlog file name.
-- If they entered a '.' instead, return & use "ChatLog.txt" as chatlog file name.
----------------------------------------------------------------------------------------------------------------------*/
string PromptForChatlog()
{
	while(true)
	{
		string input = PromptForString("(Do you want to save a chatlog? [y/n]):");
		if (tolower(input[0]) == 'y')
		{
			string fileName = PromptForString("(Enter a filename, or . for ChatLog.txt:)");
			if (fileName[0] == '.')
				return "ChatLog.txt";
			else
				return fileName;
		}
		if (tolower(input[0]) == 'n')
		{	
			return "";
		}
		cout << "* Please make sure first letter entered is y or n *" << endl;
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION PromptForString
--
-- DATE: Mar 4 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: string PromptForString(const string msg)
--					const string msg : msg to display to user
--
-- RETURNS: string : a non-empty string read from user
--
-- NOTES:
-- Displays a msg to a user and waits for a string input. If user enters an empty string,
-- continue prompting same msg until non-empty string read in. Then returns that string from user.
-- This function is borrowed from my assignment 2.
----------------------------------------------------------------------------------------------------------------------*/
string PromptForString(const string msg)
{
	while (true)
	{
		cout << msg;
		cout.flush();
		string input;
		cin >> input;
		cin.ignore();
		if (input != "")
			return input;
		cout << endl;
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION PrintWelcomeMessage
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: void PrintWelcomeMessage(const string mode, const bool chatlogOn, const string userIp, const string serverIp, const int port)
--					mode : either CLIENT or SERVER, current mode of program
--					chatlogOn : whether user has enabled printing chat log to a file or not
--					userIp : IP of current user, may not be clientIP as user could be either server or client
--					serverIp : IP of the server hosting the chat room
--					port : port server is running chat room on
--
-- RETURNS: void
--
-- NOTES:
-- Should only call this after client has connected/server has began listening for connections.
-- Prints out a client/server-specific welcome string with help messages with instructions on how to use the program.
-- Also prints out the current statuses of the program through the passed-in arguments.
----------------------------------------------------------------------------------------------------------------------*/
void PrintWelcomeMessage(const string mode, const bool chatlogOn, const string userIp, const string serverIp, const int port)
{
	cout << "*******************************************************************************" << endl;
	cout << "* Welcome to chat room console. " << endl;
	cout << "* Current status: " << endl;
	cout << "* 	connected as "<< mode << ", user ip: "<< userIp << endl;
	cout << "*	Room hosted at:" + serverIp << " on port:" << port << endl;
	if (chatlogOn)
		cout << "*	A chat log is saved in the current directory." << endl;
	if (!chatlogOn)	
		cout << "*	Chat log is not being saved." << endl;
	cout << "* Enter /help to display this message again." << endl;
	cout << "* Enter /disconnect to return to start and choose a different mode" << endl;
	if (mode == "CLIENT")
		cout << "* Enter anything else to send it to other clients." << endl;
	if (mode == "SERVER")
		cout << "* Note, entering messages as server does nothing." << endl;
	cout << "*******************************************************************************" << endl;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION PrintHelpMessage
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: void PrintHelpMessage()
--
-- RETURNS: void
--
-- NOTES:
-- Call this function when user enters /help or /h command.
-- Prints out a help message suited to both client and server.
----------------------------------------------------------------------------------------------------------------------*/
void PrintHelpMessage()
{
	cout << "* Enter /help to display this message again." << endl;
	cout << "* Enter /disconnect to return to start and choose a different mode" << endl;
	cout << "* Enter anything else to send it to other clients." << endl;
	cout << "* Note, help msgs and cmds are not printed to chat log." << endl;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION Die
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: void Die(const string message)
--
-- RETURNS: void
--
-- NOTES:
-- Only call this if errno is set.
-- Call perror with to print error message and exits program.	
----------------------------------------------------------------------------------------------------------------------*/
void Die(const string message)
{
    perror (message.c_str());
    exit (EXIT_FAILURE);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION SendPacket
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: void SendPacket(const int socketFileDesc, const string packet)
--					socketFileDesc : socket file descriptor for destination
--					packet : msg string to be sent (should include timestamp & author IP if possible)
-- RETURNS: void
--
-- NOTES:
-- Wrapper function for API send call.
-- Call this function with a string to send it.
-- String does not need to follow format <ip><timestamp>msg to be succesfully sent & received by server & other clients
-- but it would look nicer.
----------------------------------------------------------------------------------------------------------------------*/
void SendPacket(const int socketFileDesc, const string packet)
{
	send(socketFileDesc, packet.c_str(), BUFLEN, 0);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION GetCurrentIP
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Stack Overflow user brm 
--
-- PROGRAMMER: Stack Overflow user brm 
-- WRAPPED IT IN A STRING: Alex Xia
-- INTERFACE: string GetCurrentIP()
-- 
-- RETURNS: string : IPv4 address of user
--
-- NOTES:
-- Gets a list of user ip addresses and returns the last one in the list.

-- Special thanks to brm for providing a solution I needed on stack overflow. Somehow the last/only IP in the list
-- of IPs returned is the correct one
-- source: https://stackoverflow.com/questions/20800319/how-do-i-get-my-ip-address-in-c-on-linux 
----------------------------------------------------------------------------------------------------------------------*/
string GetCurrentIP()
{
	struct ifaddrs *addrs, *tmp;;
	getifaddrs(&addrs);
	tmp = addrs;
	string idkwhichIptoKeep;
	while (tmp) 
	{
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
		{
			struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
			idkwhichIptoKeep = string(inet_ntoa(pAddr->sin_addr));
		}
		tmp = tmp->ifa_next;
	}
	freeifaddrs(addrs);
	return idkwhichIptoKeep;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION GetTimeString
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Stack Overflow user DiB 
--
-- PROGRAMMER: Stack Overflow user DiB
-- WRAPPED IT IN A STRING: Alex Xia
-- INTERFACE: string GGetTimeString()
-- 
-- RETURNS: string : IPv4 address of user
--
-- NOTES:
-- Returns the current time when calling this function in HH:MM:SS format.

-- Special thanks to DiB for providing the solution I needed.
-- source: https://stackoverflow.com/questions/16357999/current-date-and-time-as-string 
----------------------------------------------------------------------------------------------------------------------*/
string GetTimeString()
{
	char buffer[64];
	memset(buffer, 0, sizeof(buffer));
	time_t rawtime;
	time(&rawtime);
	const auto timeinfo = localtime(&rawtime);
	strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo); //additional info %d-%m-%Y 
	return string(buffer);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION GetIpFromPacket
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: string GetIpFromPacket(const string packet)
-- 					packet : packet string in format <ip><timestamp>msg
--
-- RETURNS: string : the IPv4 address string from a formatted packet
--
-- NOTES:
-- Takes a formatted packet string and extracts the IPv4 address from it as a substring
-- eg. extracts 198.98.62.21 from <198.98.62.21><10:58:44>hello world
----------------------------------------------------------------------------------------------------------------------*/
string GetIpFromPacket(const string packet)
{
	int startIndex = packet.find("<") + 1;
	int endIndex = packet.find(">");
	return packet.substr(startIndex, endIndex - startIndex);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION GetIpedPacket
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: string GetIpedPacket(const string ip, const string timestampedPacket)
-- 					ip : IPv4 address to append to packet message
--					timestampedPacket : packet in format <timestamp>msg
--
-- RETURNS: string : packet with IP info appended to it
--
-- NOTES:
-- Appends an Ip address to a timestamped packet message
--
-- eg. takes <10:58:44>hello world and returns <198.98.62.21><10:58:44>hello world
----------------------------------------------------------------------------------------------------------------------*/
string GetIpedPacket(const string ip, const string timestampedPacket)
{
	return "<" +  ip + ">" + timestampedPacket;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION GetTimestampedPacket
--
-- DATE: Mar 28 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: string GetTimestampedPacket(const string msg)
--					msg : unformatted packet msg
--
-- RETURNS: string : packet with timestamp appended to it
--
-- NOTES:
-- Appends a timestamp to an unformatted packet msg
--
-- eg. takes hello world and returns <10:58:44>hello world
----------------------------------------------------------------------------------------------------------------------*/
string GetTimestampedPacket(const string msg)
{
	return "<" +  GetTimeString() + ">: " + msg;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION GetEncryptedString
--
-- DATE: Apr 2 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: string GetEncryptedString(const string msg)
--					msg : msg to be encrypted
--
-- RETURNS: string : encrypted string msg
--
-- NOTES:
-- Uses very basic Caesar cipher (offset = +1) to encrypt msg.  
-- Used by both server & client before sending any message.
-- eg. takes hello world and returns ifmmp!xpsme
----------------------------------------------------------------------------------------------------------------------*/
string GetEncryptedString(const string msg)
{
	string encryptedMsg = msg;
	for(int i=0; encryptedMsg[i] != '\0'; ++i ) 
		encryptedMsg[i]++;
	return encryptedMsg;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION GetDecryptedString
--
-- DATE: Apr 2 2018
--
-- DESIGNER: Alex Xia
--
-- PROGRAMMER: Alex Xia
--
-- INTERFACE: string GetDecryptedString(const string encryptedMsg)
--					msg : msg to be encrypted
--
-- RETURNS: string : encrypted string msg
--
-- NOTES:
-- Uses very basic Caesar cipher (offset = -1) to decrypt a msg.  
-- Used by both server & client after receiving any message.
-- eg. takes ifmmp!xpsme and returns hello world 
----------------------------------------------------------------------------------------------------------------------*/
string GetDecryptedString(const string encryptedMsg)
{
	string decryptedMsg = encryptedMsg;
	for(int i=0; decryptedMsg[i] != '\0'; ++i ) 
		decryptedMsg[i]--;
	return decryptedMsg; 	
}