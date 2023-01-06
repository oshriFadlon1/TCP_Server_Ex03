#include "TCP_Server.h"

void stratupWSA(WSAData& wsaData)
{
	string message = "Server: Error at WSAStartup()\n";
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		throw message;
	}
}

void conncetSocket(SOCKET& listenSocket)
{
	string message;
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == listenSocket)
	{
		message = "Server: Error at socket(): " + WSAGetLastError();

		WSACleanup();

		throw message;
	}
}

void sockaddrInInitialize(sockaddr_in& server)
{
	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(TIME_PORT);
}

void BindSocket(SOCKET& listenSocket, sockaddr_in& serverService)
{
	string message;

	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		message = "Time Server: Error at bind(): " + WSAGetLastError();
		closesocket(listenSocket);
		WSACleanup();
		throw message;
	}
}

void listenCheck(SOCKET& listenSocket)
{
	string message;
	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		message = "Time Server: Error at listen(): " + WSAGetLastError();
		closesocket(listenSocket);
		WSACleanup();
		throw message;
	}
}

void CloseServer(SOCKET& socket)
{
	cout << "Time Server: Closing Connection.\n";
	closesocket(socket);
	WSACleanup();
}

void selectCheck(int& nfd, fd_set& waitRecv, fd_set& waitSend)
{
	string message;
	nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
	if (nfd == SOCKET_ERROR)
	{
		message = "Time Server: Error at select(): " + WSAGetLastError();
		WSACleanup();
		throw message;
	}
}

