#include "SharedUtils.h"
/* note, all funcs here are pseudo-static. dont belong in any class but used by all*/

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

/* connect by default */
/* only print this after connected */
/* mode: "Client" or "Server". Should be enum but thats harder to read */
void PrintWelcomeMessage(const string mode, const string ip)
{
	cout << "*******************************************************************************" << endl;
	cout << "* Welcome to chat room console. " << endl;
	cout << "* Current status: connected as "<< mode << ". Room host ip:" + ip << endl;
	cout << "* Enter /help to display this message again." << endl;
	cout << "* Enter /disconnect to return to start and choose a different mode" << endl;
	if (mode == "CLIENT")
		cout << "* Enter anything else to send it to other clients." << endl;
	if (mode == "SERVER")
		cout << "* Note, entering messages as server does nothing." << endl;
	cout << "* A chat log is saved in the current directory." << endl;
	cout << "*******************************************************************************" << endl;
}

void PrintHelpMessage()
{
	cout << "* Enter /help to display this message again." << endl;
	cout << "* Enter /disconnect to return to start and choose a different mode" << endl;
	cout << "* Enter anything else to send it to other clients." << endl;
	cout << "* A chat log is saved in the current directory." << endl;
	cout << "* Note, help msgs and cmds are not printed to chat log." << endl;
}

void Die(const string message)
{
    perror (message.c_str());
    exit (EXIT_FAILURE);
}

void SendPacket(int socketFileDesc, string msg)
{
	//call packetize here
	// send (sd, sbuf, BUFLEN, 0);
	send(socketFileDesc, msg.c_str(), BUFLEN, 0);
}

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
			// printf("%s: %s\n", tmp->ifa_name, inet_ntoa(pAddr->sin_addr));
			idkwhichIptoKeep = string(inet_ntoa(pAddr->sin_addr));
		}
		tmp = tmp->ifa_next;
	}
	freeifaddrs(addrs);
	return idkwhichIptoKeep;
}