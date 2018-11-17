# ClientServerChatRoom
Console-based LAN Chat Room using the Berkeley Socket C++ API 
This application can toggle between acting as server or client mode. 

The Berkeley API is cross-platform. In light of this, the program has been tested on Windows 7-10, Ubuntu, and Fedora machines.

How to run
----------
A windows executable (compiled in cygwin) is included. Instructions on compiling on Unix systems is included in the user manual PDF. 

Run the executable and select a mode, either S for server, or C for client.
Then enter commands as prompted on screen. Available commands and details can be found in the user manual. 

Remember to have a server instance running before trying to connect any client instances.

#### Bug/Disclaimer:
Although it's meant to be run across a local network, the application still works running both clients and server on a single machine. This works fine with a single client. However, as any client that connects has the same IP address, they'll be treated as a single client regardless of how many connects. The server will still treat them as separate clients. But to the clients, any messages sent will be treated as their own message, and ignored.     
