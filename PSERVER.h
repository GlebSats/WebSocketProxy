#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstring>

class PSERVER
{
public:
	PSERVER() :lis_socket(-1), client_socket(-1), server_socket(-1), errState(0), bufToClientHasData(WSA_INVALID_EVENT),
		bufToServHasData(WSA_INVALID_EVENT), clientReadySend(WSA_INVALID_EVENT), serverReadySend(WSA_INVALID_EVENT)
	{
	}
	~PSERVER();
	void serverInitialization();
	void serverHandler();

private:
	void initSockets();
	void createSockInfo(const char* ip, const char* port, addrinfo** sockInfo);
	void createNewSocket(SOCKET& new_socket, addrinfo* sockInfo);
	void bindSocket();
	void listenState();
	void acceptConnection();
	void connectToWebServ();
	void sockCommunication();
	void createSocketEvents();
	void closeConnection();
	void stopServer();
public:
	HANDLE serviceStopEvent;
private:
	addrinfo* lisSockInfo;
	addrinfo* webSockInfo;
	SOCKET lis_socket;
	SOCKET client_socket;
	SOCKET server_socket;
	WSAEVENT bufToClientHasData;
	WSAEVENT bufToServHasData;
	WSAEVENT clientReadySend;
	WSAEVENT serverReadySend;
	WSADATA wsData;
	int errState;
};
