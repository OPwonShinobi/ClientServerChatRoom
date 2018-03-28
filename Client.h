#ifndef CLIENT_H
#define CLIENT_H
#include "SharedUtils.h"

class Client
{
public:
	Client();
	virtual ~Client();

private:
	void SelectLoop(const int);
	bool HandleNewline(const int);
	void ReadServerMessage(const int);

	bool keepSelecting = true;
};

#endif // CLIENT_H
