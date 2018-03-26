#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#include <iostream>
#include <fstream>
#include <thread>


using namespace std;

string PromptForString(const string msg);
int PromptForInteger(const string msg);
void PrintWelcomeMessage(const string mode);

void Die(const string message);
#endif
