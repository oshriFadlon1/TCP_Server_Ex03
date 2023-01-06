#pragma once
#include "Socket.h"

void stratupWSA(WSAData& wsaData);
void conncetSocket(SOCKET& listenSocket);
void sockaddrInInitialize(sockaddr_in& server);
void BindSocket(SOCKET& listenSocket, sockaddr_in& serverService);
void listenCheck(SOCKET& listenSocket);
void CloseServer(SOCKET& socket);
void selectCheck(int& nfd, fd_set& waitRecv, fd_set& waitSend);

