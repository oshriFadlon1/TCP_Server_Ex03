#include "Socket.h"

bool addSocket(SOCKET id, int what, SocketState* sockets, int& socketsCount)
{
	string message;
	unsigned long flag = 1;
	if (ioctlsocket(id, FIONBIO, &flag) != 0)
	{
		
		message = "Time Server: Error at ioctlsocket(): " + WSAGetLastError() ;
		throw message;
	}

	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].len = 0;
			socketsCount++;
			return (true);
		}
	}
	return (false);
}

void removeSocket(int index, SocketState* sockets, int& socketsCount)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	socketsCount--;
}

void acceptConnection(int index, SocketState* sockets, int& socketsCount)
{
	string message;
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;		// Address of sending partner
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);

	if (INVALID_SOCKET == msgSocket)
	{
		message = "Time Server: Error at accept(): " + WSAGetLastError() ;
		throw message;
	}
	//cout << "Time Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	if (addSocket(msgSocket, RECEIVE, sockets, socketsCount) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
}

void receiveMessage(int index, SocketState* sockets, int& socketsCount)
{
	SOCKET msgSocket = sockets[index].id;

	int len = sockets[index].len;
	sockets[index].startTime = clock();
	int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len, 0);

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Time Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(index, sockets, socketsCount);
		return;
	}
	if (bytesRecv == 0)
	{
		closesocket(msgSocket);
		removeSocket(index, sockets, socketsCount);
		return;
	}
	else
	{
		sockets[index].buffer[len + bytesRecv] = '\0'; //add the null-terminating to make it a string
		cout << "Time Server: Recieved: " << bytesRecv << " bytes of \"" << &sockets[index].buffer[len] << "\" message.\n";

		sockets[index].len += bytesRecv;

		// to do hendle the request
		// with switch 

		if (sockets[index].len > 0)
		{
			sockets[index].send = SEND;
			// find the \0 in the socket buffer
			int lenOfResponded;
			memcpy(sockets[index].buffer, &sockets[index].buffer[lenOfResponded], sockets[index].len - lenOfResponded);
			sockets[index].len -= lenOfResponded;
		}
	}

}

void sendMessage(int index, SocketState* sockets, int& socketsCount)
{
	int bytesSent = 0;

	SOCKET msgSocket = sockets[index].id;
	sockets[index].endTime = clock();

	double timePassed = (double)(sockets[index].endTime - sockets[index].startTime) / CLOCKS_PER_SEC;

	if (timePassed <= 120)
	{
		// switch create the response 
		string response = createResponse(index, sockets);

		bytesSent = send(msgSocket, response.c_str(), response.size(), 0);
		if (SOCKET_ERROR == bytesSent)
		{
			cout << "Time Server: Error at send(): " << WSAGetLastError() << endl;
			return;
			// throw
		}
		if (sockets[index].len > 0)
		{
			sockets[index].send = SEND;
		}
		else
		{
			sockets[index].send = IDLE;
		}	
	}
	else // it over 2 minutes
	{
		closesocket(sockets[index].id);
		removeSocket(index, sockets, socketsCount);
		cout << "\nClient connection closed... Socket: " << sockets[index].id;
	}
}

void startServer(SocketState* sockets, int& socketsCount)
{
	while (true)
	{

		fd_set waitRecv;
		FD_ZERO(&waitRecv);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
				FD_SET(sockets[i].id, &waitRecv);
		}

		fd_set waitSend;
		FD_ZERO(&waitSend);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].send == SEND)
				FD_SET(sockets[i].id, &waitSend);
		}

		int nfd;
		selectCheck(nfd, waitRecv, waitSend);

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitRecv))
			{
				nfd--;
				switch (sockets[i].recv)
				{
				case LISTEN:
					acceptConnection(i, sockets, socketsCount);
					break;

				case RECEIVE:
					receiveMessage(i, sockets, socketsCount);
					break;
				}
			}
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitSend))
			{
				nfd--;
				switch (sockets[i].send)
				{
				case SEND:
					sendMessage(i, sockets, socketsCount);
					break;
				}
			}
		}
	}
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