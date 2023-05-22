#include "PSERVER.h"

PSERVER::PSERVER() {
	initSockets();
}

PSERVER::~PSERVER() {
	stopServer();
}

void PSERVER::initSockets()
{
	errState = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (errState != 0) {
		std::cout << "Initialization version error: ";
		std::cout << WSAGetLastError() << std::endl;
		return;
	}
}

void PSERVER::startServer()
{
	lisSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (lisSocket == INVALID_SOCKET) {
		std::cout << "Socket initialization error: ";
		std::cout << WSAGetLastError() << std::endl;
		return;
	}

	in_addr ip_addr;
	errState = inet_pton(AF_INET, "192.168.1.104", &ip_addr);
	if (errState <= 0) {
		std::cout << "Socket address creation error" << std::endl;
		return;
	}

	ZeroMemory(&servInfo, sizeof(servInfo));
	servInfo.sin_family = AF_INET;
	servInfo.sin_port = htons(4444);
	servInfo.sin_addr = ip_addr;

	errState = bind(lisSocket, (sockaddr*)&servInfo, sizeof(servInfo));
	if (errState != 0) {
		std::cout << "Binding error: ";
		std::cout << WSAGetLastError() << std::endl;
		return;
	}

	errState = listen(lisSocket, SOMAXCONN);
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
	closesocket(lisSocket);
	WSACleanup();
}


