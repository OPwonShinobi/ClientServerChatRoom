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
		if (input != "")
			return input;
		cout << endl;
	}
}

/* connect by default */
/* only print this after connected */
/* mode: "Client" or "Server". Should be enum but thats harder to read */
void PrintWelcomeMessage(const string mode)
{
	cout << "* Welcome to chat room console. *" << endl;
	cout << "* Current status: connected as "<< mode << endl;
	cout << "* Enter /help to display this message again." << endl;
	cout << "* Enter /disconnect to return to start and choose a different mode" << endl;
	cout << "* Enter anything else to send it to other clients." << endl;
	cout << "* A chat log is saved in the current directory." << endl;
}

void Die(const string message)
{
    perror (message.c_str());
    exit (EXIT_FAILURE);
}
