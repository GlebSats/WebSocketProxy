#include "ClientP.h"
#include "ClientException.h"

CLIENT::~CLIENT()
{
	stopClient();
}

void CLIENT::startClient()
{
	try
	{
		initSockets();
		createClientSocket();
		connectToServ();
	}
	catch (const ClientException& ex)
	{
		std::cout << ex.GetErrorType() << ex.GetErrorCode() << std::endl;
	}
}


void CLIENT::initSockets()
{
	errState = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (errState != 0) {
		throw ClientException("Initialization version error: ", WSAGetLastError());
	}
}

void CLIENT::createClientSocket()
{
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket == INVALID_SOCKET) {
		throw ClientException("Socket initialization error: ", WSAGetLastError());
	}
}

void CLIENT::transSockAddr(in_addr* ip_addr)
{
	errState = inet_pton(AF_INET, "127.0.0.1", ip_addr);
	if (errState <= 0) {
		throw ClientException("Socket address translation error");
	}
}

void CLIENT::connectToServ()
{
	in_addr ip_addr;
	try
	{
		transSockAddr(&ip_addr);
	}
	catch (const std::exception&)
	{
		throw;
	}

	sockaddr_in servInfo;
	ZeroMemory(&servInfo, sizeof(servInfo));

	servInfo.sin_family = AF_INET;
	servInfo.sin_port = htons(4444);
	servInfo.sin_addr = ip_addr;

	errState = connect(client_socket, (sockaddr*)&servInfo, sizeof(servInfo));
	if (errState != 0) {
		throw ClientException("Connection failed: ", WSAGetLastError());
	}
	std::cout << "Connection successful" << std::endl;
}

void CLIENT::stopClient()
{
	closesocket(client_socket);
	WSACleanup();
}
