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
	void startServer();

private:
	void stopServer();
private:
	SOCKET lisSocket;
	WSADATA wsData;
	sockaddr_in servInfo;
	int errState;
};

