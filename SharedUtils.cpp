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

string PromptForChatlog()
{
	while(true)
	{
		string input = PromptForString("(Do you want to save a chatlog? [y/n]):");
		if (tolower(input[0]) == 'y')
		{
			string fileName = PromptForString("(Enter a filename, or space for default:)");
			if (fileName.find_first_not_of(' ') == string::npos)
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
void PrintWelcomeMessage(const string mode, const string ip, const bool chatlogOn)
{
	cout << "*******************************************************************************" << endl;
	cout << "* Welcome to chat room console. " << endl;
	cout << "* Current status: connected as "<< mode << ". Room host ip:" + ip << endl;
	cout << "* Session is being logged: " << (chatlogOn?"TRUE":"FALSE") << endl;
	cout << "* Enter /help to display this message again." << endl;
	cout << "* Enter /disconnect to return to start and choose a different mode" << endl;
	if (mode == "CLIENT")
		cout << "* Enter anything else to send it to other clients." << endl;
	if (mode == "SERVER")
		cout << "* Note, entering messages as server does nothing." << endl;
	if (chatlogOn)
		cout << "* A chat log is saved in the current directory." << endl;
	cout << "*******************************************************************************" << endl;
}

void PrintHelpMessage()
{
	cout << "* Enter /help to display this message again." << endl;
	cout << "* Enter /disconnect to return to start and choose a different mode" << endl;
	cout << "* Enter anything else to send it to other clients." << endl;
	cout << "* Note, help msgs and cmds are not printed to chat log." << endl;
}

void Die(const string message)
{
    perror (message.c_str());
    exit (EXIT_FAILURE);
}

void SendPacket(const int socketFileDesc, const string packet)
{
	// send (sd, sbuf, BUFLEN, 0);
	send(socketFileDesc, packet.c_str(), BUFLEN, 0);
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

/* https://stackoverflow.com/questions/16357999/current-date-and-time-as-string 
   thx to DiB */
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

/* get ip wrapped by <> */
/* eg. get 198.98.62.21 from <198.98.62.21>*/
string GetIpFromPacket(const string packet)
{
	int startIndex = packet.find("<") + 1;
	int endIndex = packet.find(">");
	return packet.substr(startIndex, endIndex - startIndex);
}

/* finish packet would look like <198.98.62.21><10:58:44>hello world */
string GetIpedPacket(const string ip, const string timestampedPacket)
{
	return "<" +  ip + ">" + timestampedPacket;
}

string GetTimestampedPacket(const string msg)
{
	return "<" +  GetTimeString() + ">: " + msg;
}