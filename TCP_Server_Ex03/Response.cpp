#include "Response.h"

void hendleRequest(SocketState* sockets, int index)
{
	std::filesystem::path p = "C:\\temp\\";
	filesystem::current_path(p);
	string request;
	istringstream Buff(sockets[index].buffer);
	//istringstream Buff(&sockets[index].buffer[sockets[index].len]);
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
		string fileName = crackWantedLanguage(sockets[index].wantedFile);
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
	default: // _ERROR
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