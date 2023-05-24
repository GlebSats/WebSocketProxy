#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

class PSERVER
{
public:
	PSERVER(){}
	~PSERVER();
	void startServer();

private:
	void initSockets();
	void createLisSocket();
	void transSockAddr(in_addr* ip_addr);
	void bindSocket();
	void listenState();
	void acceptConnection();
	void stopServer();
private:
	SOCKET lis_socket;
	WSADATA wsData;
	int errState;
};

