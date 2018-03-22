#include "SharedUtils.h"

string PromptForInput(const string msg)
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
