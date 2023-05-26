#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

class PSERVER
{
public:
	PSERVER():client_counter(0), is_listen(false){}
	~PSERVER();
	void startServer();

private:
	void initSockets();
	void createLisSocket();
	void transSockAddr(in_addr* ip_addr);
	void bindSocket();
	bool listenState();
	void acceptConnection();
	void stopServer();
private:
	SOCKET lis_socket;
	SOCKET client_socket;
	WSADATA wsData;
	int errState;
	int client_counter;
	bool is_listen;
};

