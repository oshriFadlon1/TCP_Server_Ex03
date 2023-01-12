#pragma once
#include "SokcetHandler.h"

void stratupWSA(WSAData& wsaData);
void conncetSocket(SOCKET& listenSocket);
void sockaddrInInitialize(sockaddr_in& server);
void BindSocket(SOCKET& listenSocket, sockaddr_in& serverService);
void listenCheck(SOCKET& listenSocket);
void CloseServer(SOCKET& socket);
void startServer(SocketState* sockets, int& socketsCount);


