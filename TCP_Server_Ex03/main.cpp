#include"TCP_Server.h"

int main()
{
	struct SocketState sockets[MAX_SOCKETS] = { 0 };
	int socketsCount = 0;

	WSAData wsaData;
	stratupWSA(wsaData);

	SOCKET listenSocket;
	conncetSocket(listenSocket);


	sockaddr_in serverService;
	sockaddrInInitialize(serverService);

	BindSocket(listenSocket, serverService);

	addSocket(listenSocket, LISTEN,sockets,socketsCount);

	startServer(sockets, socketsCount);

	CloseServer(listenSocket);
	return 0;
}




