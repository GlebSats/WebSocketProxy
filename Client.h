#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

class CLIENT
{
public:
	CLIENT() {}
	~CLIENT();
	void startClient();

private:
	void initSockets();
	void createClientSocket();
	void transSockAddr(in_addr* ip_addr);
	void connectToServ();
	void stopClient();
private:
	SOCKET client_socket;
	WSADATA wsData;
	int errState;
};
