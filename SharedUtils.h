#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#ifndef BUFLEN
#define BUFLEN 255
#endif

#include <cstring>
#include <cstdlib>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <ifaddrs.h>

#include <iostream>
#include <fstream>
#include <thread>
#include <vector>

using namespace std;

string PromptForString(const string);
int PromptForInteger(const string);
void PrintWelcomeMessage(const string, const string);
void PrintHelpMessage();

string GetCurrentIP(); // may need to remove
void Die(const string message);
void SendPacket(int, string);
#endif
