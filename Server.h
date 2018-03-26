#ifndef SERVER_H
#define SERVER_H
#include "SharedUtils.h"

class Server
{
    public:
        Server();
        virtual ~Server();

	private:
		void SelectLoop(int);
};

#endif // SERVER_H
