#include"TCP_Server.h"

int main()
{
	struct SocketState sockets[MAX_SOCKETS] = { 0 };
	int socketsCount = 0;

<<<<<<< HEAD
const int HTTP_PORT = 42069;
const int MAX_SOCKETS = 60;
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;
const int SEND_TIME = 1;
const int SEND_SECONDS = 2;

bool addSocket(SOCKET id, int what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
void sendMessage(int index);

struct SocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;


void main()
{
	// Initialize Winsock (Windows Sockets).

	// Create a WSADATA object called wsaData.
	// The WSADATA structure contains information about the Windows 
	// Sockets implementation.
=======
>>>>>>> origin/master
	WSAData wsaData;
	stratupWSA(wsaData);

	SOCKET listenSocket;
	conncetSocket(listenSocket);


	sockaddr_in serverService;
<<<<<<< HEAD
	// Address family (must be AF_INET - Internet address family).
	serverService.sin_family = AF_INET;
	// IP address. The sin_addr is a union (s_addr is a unsigned long 
	// (4 bytes) data type).
	// inet_addr (Iternet address) is used to convert a string (char *) 
	// into unsigned long.
	// The IP address is INADDR_ANY to accept connections on all interfaces.
	serverService.sin_addr.s_addr = INADDR_ANY;
	// IP Port. The htons (host to network - short) function converts an
	// unsigned short from host to TCP/IP network byte order 
	// (which is big-endian).
	serverService.sin_port = htons(HTTP_PORT);
=======
	sockaddrInInitialize(serverService);
>>>>>>> origin/master

	BindSocket(listenSocket, serverService);

	addSocket(listenSocket, LISTEN,sockets,socketsCount);

	startServer(sockets, socketsCount);

	CloseServer(listenSocket);
	return 0;
}




