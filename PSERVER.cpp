#include "PSERVER.h"
#include "ServException.h"

PSERVER::PSERVER() {
	try
	{
		initSockets();
		createLisSocket();
		startServer();
	}
	catch (const ServException& ex)
	{
		std::cout << ex.GetErrorType() << ex.GetErrorCode() << std::endl;
	}
}

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
	errState = inet_pton(AF_INET, "192.168.1.104", ip_addr);
	if (true) {
		throw ServException("Socket address translation error");
	}
}

void PSERVER::startServer()
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
	ZeroMemory(&servInfo, sizeof(servInfo));
	servInfo.sin_family = AF_INET;
	servInfo.sin_port = htons(4444);
	servInfo.sin_addr = ip_addr;

	errState = bind(lis_socket, (sockaddr*)&servInfo, sizeof(servInfo));
	if (errState != 0) {
		std::cout << "Binding error: ";
		std::cout << WSAGetLastError() << std::endl;
		return;
	}

	errState = listen(lis_socket, SOMAXCONN);
	if (errState != 0) {
		std::cout << "Listening error: ";
		std::cout << WSAGetLastError() << std::endl;
		return;
	}
	else {
		std::cout << "Server in listening state" << std::endl;
	}
}

void PSERVER::stopServer()
{
	closesocket(lis_socket);
	WSACleanup();
}


