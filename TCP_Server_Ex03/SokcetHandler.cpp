#include "SokcetHandler.h"
#include <sstream>

using namespace std;

bool addSocket(SOCKET id, int what, SocketState* sockets, int& socketsCount) {
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
		message = "Server: Error at accept(): " + WSAGetLastError() ;
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
		cout << "Server: Error at recv(): " << WSAGetLastError() << endl;
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
		cout << "Server: Recieved: " << bytesRecv << " bytes of \"" << &sockets[index].buffer[len] << "\" message.\n";

		sockets[index].len += bytesRecv;

		handleRequest(sockets, index);

		if (sockets[index].len > 0)
		{
			sockets[index].send = SEND;

			int lenResponded;
			findFirstBackslashzeroindex(index, sockets, lenResponded);
			memcpy(sockets[index].buffer, &sockets[index].buffer[lenResponded], sockets[index].len - lenResponded);
			sockets[index].len -= lenResponded;
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
		string response = createResponse(sockets, index);
		cout << response;
		bytesSent = send(msgSocket, response.c_str(), response.size(), 0);
		if (SOCKET_ERROR == bytesSent)
		{
			cout << "Server: Error at send(): " << WSAGetLastError() << endl;
			return;
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

void selectCheck(int& nfd, fd_set& waitRecv, fd_set& waitSend)
{
	string message;
	nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
	if (nfd == SOCKET_ERROR)
	{
		message = "Server: Error at select(): " + WSAGetLastError();
		WSACleanup();
		throw message;	
	}
}

void handleRequest(SocketState* sockets, int index)
{
	std::filesystem::path p = "C:\\temp\\";
	filesystem::current_path(p);
	string request;
	stringstream Buff(sockets[index].buffer);

	Buff >> request;
	cout << "Client Request: " << request << endl;
	int method = resolveMethods(request);
	switch (method)
	{
	case httpMethods::Get:case httpMethods::Head:
	{
		if (method == httpMethods::Get)
			sockets[index].request = Get;
		else
			sockets[index].request = Head;

		Buff >> sockets[index].wantedFile;
		sockets[index].wantedFile = sockets[index].wantedFile.substr(1, sockets[index].wantedFile.length() - 1); // Removes Backslash
		string fileName = crackLanguage(sockets[index].wantedFile);
		sockets[index].wantedFile = fileName;
		if (fileExists(sockets[index].wantedFile))
			sockets[index].statusCode = 200;
		else
			sockets[index].statusCode = 404;
		break;
	}
	case httpMethods::Post:
	{
		string postBody = extractPOSTMANbody(Buff);
		sockets[index].request = Post;
		sockets[index].statusCode = 200;
		cout << "POST request:\n";
		cout << postBody;
		break;
	}
	case httpMethods::Delete:
	{
		sockets[index].request = Delete;
		Buff >> sockets[index].wantedFile;
		sockets[index].wantedFile = sockets[index].wantedFile.substr(1, sockets[index].wantedFile.length() - 1);
		executeDELETErequest(index, sockets);
		break;
	}
	case httpMethods::Options:
		sockets[index].request = Options;
		sockets[index].statusCode = 204;
		break;
	case httpMethods::Trace:
		sockets[index].request = Trace;
		sockets[index].statusCode = 200;
		break;
	case httpMethods::Put:
		sockets[index].request = Put;
		Buff >> sockets[index].wantedFile;
		sockets[index].wantedFile = sockets[index].wantedFile.substr(1, sockets[index].wantedFile.length() - 1);
		(fileExists(sockets[index].wantedFile)) ? sockets[index].statusCode = 200 : sockets[index].statusCode = 201;
		break;
	default: // ERROR
		sockets[index].request = Error;
		sockets[index].statusCode = 501;
		break;
	}
}

httpMethods resolveMethods(string request)
{
	if (request == "GET")
		return httpMethods::Get;
	if (request == "HEAD")
		return httpMethods::Head;
	if (request == "POST")
		return httpMethods::Post;
	if (request == "DELETE")
		return httpMethods::Delete;
	if (request == "OPTIONS")
		return httpMethods::Options;
	if (request == "PUT")
		return httpMethods::Put;
	if (request == "TRACE")
		return httpMethods::Trace;
	return httpMethods::Error;
}

void createBaseMessage(int statusCode, stringstream& message)
{
	time_t currTime;
	time(&currTime);
	message << "HTTP/1.1 " << statusCode << " " << statusCode_description(statusCode) << "\n";
	message << "Date: " << ctime(&currTime); // ctime automatically adds \n
	message << "Server: HTTP Web Server\n";
	message << "Content-Type: text/html\n";
	message << "Connection: keep-alive\n";
}

string statusCode_description(int statusCode)
{
	if (statusCode == 200) return "OK";
	if (statusCode == 201) return "Created";
	if (statusCode == 204) return "No Content";
	if (statusCode == 404) return "Not Found";
	else return "Not Implemented";
}

string crackLanguage(string& wantedFile)
{
	string lang;
	stringstream fileName, finish;
	int len = wantedFile.size(), i = 0;
	bool sendDefault = false;

	while (wantedFile[i] != '.' && i < len)
	{
		fileName << wantedFile[i];
		i++;
	}
	while (wantedFile[i] != '?' && i < len) // .html
	{
		finish << wantedFile[i];
		i++;
	}
	if (i < len)
	{
		stringstream queryParam;
		while (wantedFile[i] != '=' && i < len)
		{
			if (wantedFile[i] != '?')
				queryParam << wantedFile[i];
			i++;
		}
		queryParam << wantedFile[i];
		i++;
		if (queryParam.str() != "lang=")// illigal query parameter
		{
			sendDefault = true;
		}

		if (!sendDefault)
		{
			lang = wantedFile[i];
			i++;
			while (i < len)
				lang += wantedFile[i++];
			fileName << "_" << lang << finish.str();
		}
	}
	else
	{
		sendDefault = true;
	}
		
	if (sendDefault)
	{
		string def = fileName.str();
		def += finish.str();
		if (fileExists(def))
		{
			return def;
		}	
		fileName << "_en"; // default - English
		fileName << finish.str();
	}
	return fileName.str();
}

string createResponse(SocketState* sockets, int index)
{
	stringstream fullMessage, messageBody;
	ifstream file;
	createBaseMessage(sockets[index].statusCode, fullMessage);
	cout << fullMessage.str();

	switch (sockets[index].request)
	{
	case Get:
		if (sockets[index].statusCode == 404)
		{
			fullMessage << "Content-Length:" << 13 << "\n";
			fullMessage << "\n";
			fullMessage << "404 NOT FOUND";
		}
		else // statusCode = 200
		{
			readFile(file, sockets[index].wantedFile, messageBody);
			fullMessage << "Content-Length: " << messageBody.str().size() << "\n";
			fullMessage << "\n";
			fullMessage << messageBody.str();

		}
		break;

	case Head:
		if (sockets[index].statusCode == 404)
		{
			fullMessage << "Content-Length: " << 13 << "\n";
		}
		else // statusCode = 200
		{
			readFile(file, sockets[index].wantedFile, messageBody);
			fullMessage << "Content-Length: " << messageBody.str().size() << "\n";
		}
		fullMessage << "\n";
		break;

	case Post: {
		string postResponse = "Sent POST response";
		fullMessage << "Content-Length: " << postResponse.size() << "\n";
		fullMessage << "\n";
		fullMessage << postResponse; }
		break;

	case Delete:
		if (sockets[index].statusCode == 200)
			messageBody << "File Deleted Successfully";
		else // statusCode = 404
			messageBody << "404 NOT FOUND";

		fullMessage << "Content-Length: " << messageBody.str().size() << "\n";
		fullMessage << "\n";
		fullMessage << messageBody.str();
		break;

	case Options:
		fullMessage << "Allow: GET, POST, PUT, OPTIONS, DELETE, TRACE, HEAD\n";
		fullMessage << "Accept-Language: he, en, fr\n";
		fullMessage << "\n";
		break;

	case Trace:
		fullMessage << "Content-Length: " << strlen(sockets[index].buffer) << "\n";
		fullMessage << "\n";
		fullMessage << sockets[index].buffer;
		break;

	case Put: {
		ofstream output;
		stringstream Buff(sockets[index].buffer);
		string response;
		output.open(sockets[index].wantedFile, std::ofstream::out | std::ofstream::trunc);
		output << extractPOSTMANbody(Buff);
		output.close();

		if (sockets[index].statusCode == 200)
			response = "File Updated Successfully";
		else // statusCode = 201
			response = "File Created and Updated Successfully";

		fullMessage << "Content-Length: " << response.size() << "\n";
		fullMessage << "\n";
		fullMessage << response; }
		break;

	default://Error
		string errorMessage = "Error. This Server Does Not Support This Kind Of Requests.";
		errorMessage += "Please Check 'OPTIONS'.";
		fullMessage << "Content-Length: " << errorMessage.size() << "\n";
		fullMessage << "\n";
		fullMessage << errorMessage;
		break;
	}
	cout << fullMessage.str();

	return fullMessage.str();
}

bool fileExists(const string& fileName)
{
	ifstream file(fileName.c_str());
	return file.good();
}

void executeDELETErequest(int index, SocketState* sockets)
{
	int status;
	status = remove(sockets[index].wantedFile.c_str());
	if (status == 0)
		sockets[index].statusCode = 200;
	else
		sockets[index].statusCode = 404;
}

string extractPOSTMANbody(stringstream& Buff)
{
	string header, body;
	stringstream post;

	while (getline(Buff, header))
		if (header == "\n" || header == "\r")
			break;
	while (getline(Buff, body))
	{
		if (body == "\n" || body == "\r")
			post << "\n";
		else
			post << body << endl;
	}
	return post.str();
}

void readFile(ifstream& File, string& fileName, stringstream& message)
{
	File.open(fileName);
	string temp;
	while (File.good())
	{
		getline(File, temp);
		if (temp.empty())
			message << "\n";
		else
			message << temp << endl;
	}
}

void findFirstBackslashzeroindex(int index, SocketState* sockets,int& lenOfResponded)
{
	int i = 0;
	int bufferSize = (sizeof(sockets[index].buffer)) / sizeof((sockets[index].buffer[0]));
	for (i = 0; i < bufferSize; i++)
	{
		if (sockets[index].buffer[i] == '\0')
			break;
	}
	lenOfResponded = i;
}