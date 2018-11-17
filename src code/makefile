CXX = g++
CPPFLAGS = -Wall -g -g3 -pthread
RM=rm -f

chatroom: main.o Server.o Client.o SharedUtils.o
	$(CXX) $(CPPFLAGS) -o chatroom main.o Server.o Client.o SharedUtils.o

main.o: Server.o Client.o SharedUtils.o
	$(CXX) $(CPPFLAGS) -c main.cpp

Server.o: Server.cpp Server.h
	$(CXX) $(CPPFLAGS) -c Server.cpp

Client.o: Client.cpp Client.h
	$(CXX) $(CPPFLAGS) -c Client.cpp

SharedUtils.o: SharedUtils.cpp SharedUtils.h
	$(CXX) $(CPPFLAGS) -c SharedUtils.cpp

all: chatroom;

clean:
	$(RM) *.o *.exe;

clean_o:
	$(RM) *.o;
