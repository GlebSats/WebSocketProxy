#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

class PSERVER
{
public:
	PSERVER();
	~PSERVER();
	void initSockets();
	void createLisSocket();
	void transSockAddr(in_addr* ip_addr);
	void startServer();

private:
	void stopServer();
private:
	SOCKET lis_socket;
	SOCKET client_socket;
	WSADATA wsData;
	sockaddr_in servInfo;
	sockaddr_in clientInfo;
	int errState;
};

