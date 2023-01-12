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

void handleRequest(SocketState* sockets, int index);
httpMethods resolveMethods(string request);
string crackLanguage(string& wantedFile);
string createResponse(SocketState* sockets, int index);
void findFirstBackslashzeroindex(int index, SocketState* sockets, int& lenOfResponded);

void createBaseMessage(int statusCode, stringstream& message);
string statusCode_description(int statusCode);
bool fileExists(const string& fileName);
void executeDELETErequest(int index, SocketState* sockets);
string extractPOSTMANbody(stringstream& Buff);
void readFile(ifstream& File, string& fileName, stringstream& message);
stringstream createMessage(int size, string message);