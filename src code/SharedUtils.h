#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#define BUFLEN 255

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
string PromptForChatlog();

void PrintWelcomeMessage(const string, const bool, const string, const string, const int);
void PrintHelpMessage();

string GetCurrentIP(); // may need to remove
void Die(const string message);
void SendPacket(const int, const string);
string GetTimeString();
string GetIpFromPacket(const string);
string GetIpedPacket(const string, const string);
string GetTimestampedPacket(const string);

string GetEncryptedString(const string);
string GetDecryptedString(const string);
#endif
