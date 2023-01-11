#include"Utility.h"

typedef struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving?
	int	send;			// Sending?
	char buffer[2048];
	int len;
	int request;		// GET, POST, ...
	string wantedFile;
	int statusCode;
	clock_t startTime = NULL, endTime = NULL;
}SocketState;

bool addSocket(SOCKET id, int what, SocketState* sockets, int& socketsCount);
void removeSocket(int index, SocketState* sockets, int& socketsCount);
void acceptConnection(int index, SocketState* sockets, int& socketsCount);
void receiveMessage(int index, SocketState* sockets, int& socketsCount);
void sendMessage(int index, SocketState* sockets, int& socketsCount);
void selectCheck(int& nfd, fd_set& waitRecv, fd_set& waitSend);
void startServer(SocketState* sockets, int& socketsCount);

