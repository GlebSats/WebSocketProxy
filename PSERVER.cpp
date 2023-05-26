#include "PSERVER.h"
#include "ServException.h"


PSERVER::~PSERVER() {
	stopServer();
}


void PSERVER::initSockets()
{
	errState = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (errState != 0) {
		throw ServException("Initialization version error: ", WSAGetLastError());
	}
}

void PSERVER::createLisSocket()
{
	lis_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (lis_socket == INVALID_SOCKET) {
		throw ServException("Socket initialization error: ", WSAGetLastError());
	}

}

void PSERVER::transSockAddr(in_addr* ip_addr)
{
	errState = inet_pton(AF_INET, "127.0.0.1", ip_addr);
	if (errState <= 0) {
		throw ServException("Socket address translation error");
	}
}

void PSERVER::bindSocket()
{
	in_addr ip_addr;
	try
	{
		transSockAddr(&ip_addr);
	}
	catch (const ServException& ex)
	{
		throw;
	}

	sockaddr_in servInfo;
	ZeroMemory(&servInfo, sizeof(servInfo));
	servInfo.sin_family = AF_INET;
	servInfo.sin_port = htons(4444);
	servInfo.sin_addr = ip_addr;

	errState = bind(lis_socket, (sockaddr*)&servInfo, sizeof(servInfo));
	if (errState != 0) {
		throw ServException("Binding error: ", WSAGetLastError());
	}
}

bool PSERVER::listenState()
{
	errState = listen(lis_socket, SOMAXCONN);
	if (errState != 0) {
		throw ServException("Listening error: ", WSAGetLastError());
	}
	std::cout << "Server in listening state..." << std::endl;
	return true;
}

void PSERVER::acceptConnection()
{
	sockaddr_in clientInfo;
	int clientInfo_size = sizeof(clientInfo);
	ZeroMemory(&clientInfo, clientInfo_size);

	SOCKET client_socket = accept(lis_socket, (sockaddr*)&clientInfo, &clientInfo_size);
	if (client_socket == INVALID_SOCKET) {
		closesocket(client_socket);
		throw ServException("Client connection error: ", WSAGetLastError());
	}
	client_counter++;
	std::cout << "Client has been connected" << std::endl;
	std::cout << "Number of connected clients: " << client_counter << std::endl;
}
}

void PSERVER::startServer()
{
	try
	{
		initSockets();
		createLisSocket();
		bindSocket();
		listenState();

		acceptConnection();
	}
	catch (const ServException& ex)
	{
		std::cout << ex.GetErrorType() << ex.GetErrorCode() << std::endl;
	}
}

void PSERVER::stopServer()
{
	if (client_counter > 0) {
		closesocket(client_socket);
	}
	if (is_listen) {
		closesocket(lis_socket);
	}
	WSACleanup();
}


