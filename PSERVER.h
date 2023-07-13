#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstring>

class PSERVER
{
public:
	PSERVER(const char* listeningPort, const char* serverIP, const char* serverPort);
	~PSERVER();
	void serverInitialization();
	void serverHandler();
	void stopServer();

private:
	void initSockets(); // Function initiates use of the Winsock DLL
	void createSockInfo(const char* ip, const char* port, addrinfo** sockInfo); // Create addrinfo and translate host name to address
	void createNewSocket(SOCKET& new_socket, addrinfo* sockInfo); // Create socket with addrinfo parameters
	void bindSocket(); // Function associates a local address with a socket
	void listenState(); // Set socket to listen state
	void acceptConnection();
	void connectToWebServ();
	void sockCommunication();
	void createSocketEvents();
	void closeConnection();
	PSERVER(const PSERVER&) = delete; // Copy not allowed
	void operator=(const PSERVER&) = delete; // Assignment not allowed
public:
	HANDLE* serviceStopEvent;
private:
	const char* listeningPort;
	const char* serverIP;
	const char* serverPort;
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
