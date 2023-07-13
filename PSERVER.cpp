#include "PSERVER.h"
#include "ServException.h"
#include "writeLog.h"
#include <string>

#define BUFFER_SIZE 5

PSERVER::~PSERVER() {
	stopServer();
	writeLog("Proxy server was stopped");
}

void PSERVER::serverInitialization()
{
	initSockets();
	createSockInfo("127.0.0.1", listeningPort, &lisSockInfo);
	createNewSocket(lis_socket, lisSockInfo);
	bindSocket();
	listenState();
}

void PSERVER::serverHandler()
{
	acceptConnection();
	createSockInfo(serverIP, serverPort, &webSockInfo);
	createNewSocket(server_socket, webSockInfo);
	connectToWebServ();
	sockCommunication();
}


void PSERVER::initSockets()
{
	errState = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (errState != 0) {
		throw ServException("Initialization version error: ", WSAGetLastError());
	}
}

void PSERVER::createSockInfo(const char* ip, const char* port, addrinfo** sockInfo)
{
	addrinfo addrInfo;
	ZeroMemory(&addrInfo, sizeof(addrInfo));
	addrInfo.ai_family = AF_INET;
	addrInfo.ai_socktype = SOCK_STREAM;
	addrInfo.ai_protocol = IPPROTO_TCP;
	errState = getaddrinfo(ip, port, &addrInfo, sockInfo);
	if (errState != 0) {
		throw ServException("Error getting address information: ", WSAGetLastError());
	}
}

void PSERVER::createNewSocket(SOCKET& new_socket, addrinfo* sockInfo)
{
	new_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (new_socket == INVALID_SOCKET) {
		throw ServException("Socket initialization error: ", WSAGetLastError());
	}

}

void PSERVER::bindSocket()
{
	errState = bind(lis_socket, lisSockInfo->ai_addr, lisSockInfo->ai_addrlen);
	if (errState != 0) {
		throw ServException("Binding error: ", WSAGetLastError());
	}
}

void PSERVER::listenState()
{
	errState = listen(lis_socket, SOMAXCONN);
	if (errState != 0) {
		throw ServException("Listening error: ", WSAGetLastError());
	}
	writeLog("Server in listening state...");
}

void PSERVER::acceptConnection()
{
	char ipStr[INET_ADDRSTRLEN];
	sockaddr_in clientSockInfo;
	ZeroMemory(&clientSockInfo, sizeof(clientSockInfo));
	int clientSize = sizeof(clientSockInfo);
	client_socket = accept(lis_socket, (sockaddr*)&clientSockInfo, &clientSize);
	if (client_socket == INVALID_SOCKET) {
		throw ServException("Client connection error: ", WSAGetLastError());
	}
	inet_ntop(clientSockInfo.sin_family, &(clientSockInfo.sin_addr), ipStr, INET_ADDRSTRLEN);
	std::string strIP(ipStr);
	writeLog("Client: " + strIP + " has been connected");
}

void PSERVER::connectToWebServ()
{
	errState = connect(server_socket, webSockInfo->ai_addr, webSockInfo->ai_addrlen);
	if (errState != 0) {
		throw ServException("Connection to Web Server failed: ", WSAGetLastError());
	}
	writeLog("Connection to Server successful");
}

void PSERVER::sockCommunication() {

	WSANETWORKEVENTS clientEvents;
	WSANETWORKEVENTS serverEvents;
	char bufToClient[BUFFER_SIZE];
	char bufToServer[BUFFER_SIZE];
	ZeroMemory(&bufToClient, sizeof(bufToClient));
	ZeroMemory(&bufToServer, sizeof(bufToServer));
	int dataForClient = 0;
	int dataForServer = 0;
	int indexForClient = 0;
	int indexForServer = 0;

	createSocketEvents();
	HANDLE eventArr[5] = { *serviceStopEvent, clientReadySend, serverReadySend, bufToServHasData, bufToClientHasData };

	while (true) {

		int eventResult = WSAWaitForMultipleEvents(5, eventArr, FALSE, INFINITE, FALSE);
		if (eventResult == WSA_WAIT_FAILED) {
			closeConnection();
			throw ServException("Error while waiting for events: ", WSAGetLastError());
		}

		if (eventResult == WSA_WAIT_EVENT_0) {
			closeConnection();
			throw ServException("Connection has been severed: ", WSAGetLastError());
		}

		errState = WSAEnumNetworkEvents(client_socket, clientReadySend, &clientEvents);
		if (errState == SOCKET_ERROR) {
			closeConnection();
			throw ServException("Error while getting information about events: ", WSAGetLastError());
		}

		errState = WSAEnumNetworkEvents(server_socket, serverReadySend, &serverEvents);
		if (errState == SOCKET_ERROR) {
			closeConnection();
			throw ServException("Error while getting information about events: ", WSAGetLastError());
		}

		if (clientEvents.lNetworkEvents & FD_CLOSE) { // poslat zbýtek dat?
			closeConnection();
			throw ServException("Connection with the client has been severed: ", WSAGetLastError());
		}

		if (serverEvents.lNetworkEvents & FD_CLOSE) { // poslat zbýtek dat?
			closeConnection();
			throw ServException("Connection with the server has been severed: ", WSAGetLastError());
		}

		if ((clientEvents.lNetworkEvents & FD_READ) && (dataForServer == 0)) {
			int rec_data = recv(client_socket, bufToServer, BUFFER_SIZE, 0);
			if (rec_data == SOCKET_ERROR) {
				closeConnection();
				throw ServException("Connection with the client has been severed: ", WSAGetLastError());
			}
			dataForServer = rec_data;
			indexForServer = 0;
		}

		if ((serverEvents.lNetworkEvents & FD_READ) && (dataForClient == 0)) {
			int rec_data = recv(server_socket, bufToClient, BUFFER_SIZE, 0);
			if (rec_data == SOCKET_ERROR) {
				closeConnection();
				throw ServException("Connection with the server has been severed: ", WSAGetLastError());
			}
			dataForClient = rec_data;
			indexForClient = 0;
		}

		if (dataForServer != 0) {
			int send_data = send(server_socket, bufToServer + indexForServer, dataForServer, 0);
			if (send_data == SOCKET_ERROR) {
				if (WSAGetLastError() != WSAEWOULDBLOCK) {
					closeConnection();
					throw ServException("Connection with the server has been severed: ", WSAGetLastError());
				}
			}
			else {
				dataForServer -= send_data;
				indexForServer += send_data;
			}
		}

		if (dataForClient != 0) {
			int send_data = send(client_socket, bufToClient + indexForClient, dataForClient, 0);
			if (send_data == SOCKET_ERROR) {
				if (WSAGetLastError() != WSAEWOULDBLOCK) {
					closeConnection();
					throw ServException("Connection with the client has been severed: ", WSAGetLastError());
				}
			}
			else {
				dataForClient -= send_data;
				indexForClient += send_data;
			}
		}

		if (dataForClient != 0) {
			WSASetEvent(bufToClientHasData);
		}
		else {
			WSAResetEvent(bufToClientHasData);
		}

		if (dataForServer != 0) {
			WSASetEvent(bufToServHasData);
		}
		else {
			WSAResetEvent(bufToServHasData);
		}
	}
}

void PSERVER::createSocketEvents() {

	bufToClientHasData = WSACreateEvent();
	if (bufToClientHasData == WSA_INVALID_EVENT) {
		closeConnection();
		throw ServException("Create WSA Event failed: ", WSAGetLastError());
	}

	bufToServHasData = WSACreateEvent();
	if (bufToServHasData == WSA_INVALID_EVENT) {
		closeConnection();
		throw ServException("Create WSA Event failed: ", WSAGetLastError());
	}

	clientReadySend = WSACreateEvent();
	if (clientReadySend == WSA_INVALID_EVENT) {
		closeConnection();
		throw ServException("Create WSA Event failed: ", WSAGetLastError());
	}

	serverReadySend = WSACreateEvent();
	if (serverReadySend == WSA_INVALID_EVENT) {
		closeConnection();
		throw ServException("Create WSA Event failed: ", WSAGetLastError());
	}

	if (WSAEventSelect(client_socket, clientReadySend, FD_READ | FD_CLOSE) != 0) {
		closeConnection();
		throw ServException("WSAEventSelect function failed: ", WSAGetLastError());
	}

	if (WSAEventSelect(server_socket, serverReadySend, FD_READ | FD_CLOSE) != 0) {
		closeConnection();
		throw ServException("WSAEventSelect function failed: ", WSAGetLastError());
	}

}

void PSERVER::closeConnection()
{
	WSACloseEvent(bufToClientHasData);
	WSACloseEvent(bufToServHasData);
	WSACloseEvent(clientReadySend);
	WSACloseEvent(serverReadySend);
	shutdown(server_socket, SD_BOTH);
	shutdown(client_socket, SD_BOTH);
	closesocket(server_socket);
	closesocket(client_socket);
}

void PSERVER::stopServer()
{
	freeaddrinfo(webSockInfo);
	freeaddrinfo(lisSockInfo);
	if (server_socket != INVALID_SOCKET) {
		closesocket(server_socket);
	}
	if (client_socket != INVALID_SOCKET) {
		closesocket(client_socket);
	}
	if (lis_socket != INVALID_SOCKET) {
		closesocket(lis_socket);
	}
	WSACleanup();
}

