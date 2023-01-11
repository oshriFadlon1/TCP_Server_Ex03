#pragma once
#include"Socket.h"
#include <filesystem>

void hendleRequest(SocketState* sockets, int index);
httpMethods resolveMethods(string request);
