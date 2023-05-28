#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

class PSERVER
{
public:
	PSERVER() :lis_socket(-1), client_socket(-1), web_socket(-1), errState(0){}
	~PSERVER();
	void startServer();

private:
	void initSockets();
	void createNewSocket(SOCKET& new_socket);
	void transSockAddr(const char* ip_string, in_addr* ip_addr);
	void bindSocket();
	void listenState();
	void acceptConnection();
	void connectToWebServ();
	void sockCommunication();
	void stopServer();
private:
	SOCKET lis_socket;
	SOCKET client_socket;
	SOCKET web_socket;
	WSADATA wsData;
	int errState;
};
