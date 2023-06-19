#include "PSERVER.h"
#include "ServException.h"

#define BUFFER_SIZE 1024

PSERVER::~PSERVER() {
	stopServer();
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
	std::cout << "Server in listening state..." << std::endl;
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
	std::cout << "Client: " << ipStr << " has been connected" << std::endl;
}

void PSERVER::connectToWebServ()
{
	errState = connect(web_socket, webSockInfo->ai_addr, webSockInfo->ai_addrlen);
	if (errState != 0) {
		throw ServException("Connection to Web Server failed: ", WSAGetLastError());
	}
	std::cout << "Connection to Web Server successful" << std::endl;
}

void PSERVER::sockCommunication()
{
	char bufToClient[BUFFER_SIZE];
	char bufToServer[BUFFER_SIZE];
	ZeroMemory(&bufToClient, sizeof(bufToClient));
	ZeroMemory(&bufToServer, sizeof(bufToServer));
	int dataForClient = 0;
	int dataForServer = 0;
	int indexForClient = 0;
	int indexForServer = 0;

	while (true) {
		fd_set sendset;
		FD_ZERO(&sendset);
		FD_SET(client_socket, &sendset);
		FD_SET(server_socket, &sendset);
		fd_set recset;
		FD_ZERO(&recset);
		FD_SET(client_socket, &recset);
		FD_SET(server_socket, &recset);
		timeval timeout{ 0, 0 };
		int readySock = select(0, &sendset, nullptr, /*&recset,*/ nullptr, &timeout);
		if (readySock == SOCKET_ERROR) {
			closeConnection();
			throw ServException("Select function error: ", WSAGetLastError());
		}
		if (FD_ISSET(server_socket, &sendset)) {
			if (dataForClient == 0) {
				int rec_data = recv(server_socket, bufToClient, BUFFER_SIZE, 0);
				if (rec_data == SOCKET_ERROR) {
					closeConnection();
					throw ServException("Connection with the server has been severed: ", WSAGetLastError());
				}
				dataForClient = rec_data;
				indexForClient = 0;
			}
		}
		if (FD_ISSET(client_socket, &sendset)) {
			if (dataForServer == 0) {
				int rec_data = recv(client_socket, bufToServer, BUFFER_SIZE, 0);
				if (rec_data == SOCKET_ERROR) {
					closeConnection();
					throw ServException("Connection with the client has been severed: ", WSAGetLastError());
				}
				dataForServer = rec_data;
				indexForServer = 0;
			}
		}
		if (FD_ISSET(server_socket, &recset)) {
			if (dataForServer > 0) {
				int send_data = send(server_socket, bufToServer + indexForServer, dataForServer, 0);
				if (send_data == SOCKET_ERROR) {
					closeConnection();
					throw ServException("Connection with the server has been severed: ", WSAGetLastError());
				}
				dataForServer -= send_data;
				indexForServer += send_data;
			}
		}
		if (FD_ISSET(client_socket, &recset)) {
			if (dataForClient > 0) {
				int send_data = send(client_socket, bufToClient + indexForClient, dataForClient, 0);
				if (send_data == SOCKET_ERROR) {
					closeConnection();
					throw ServException("Connection with the client has been severed: ", WSAGetLastError());
				}
				dataForClient -= send_data;
				indexForClient += send_data;
			}
		}
	}
}

void PSERVER::closeConnection()
{
	shutdown(web_socket, SD_BOTH);
	shutdown(client_socket, SD_BOTH);
	closesocket(web_socket);
	closesocket(client_socket);
}

void PSERVER::startServer()
{
	try
	{
		initSockets();
		createSockInfo("127.0.0.1", "4444", &lisSockInfo);
		createNewSocket(lis_socket, lisSockInfo);
		bindSocket();
		listenState();
		while (true) {
			try
			{
				acceptConnection();
				createSockInfo("127.0.0.1", "1883", &webSockInfo);
				createNewSocket(web_socket, webSockInfo);
				connectToWebServ();
				sockCommunication();
			}
			catch (const ServException& ex)
			{
				std::cout << ex.GetErrorType() << ex.GetErrorCode() << std::endl;
			}
		}
	}
	catch (const ServException& ex)
	{
		std::cout << ex.GetErrorType() << ex.GetErrorCode() << std::endl;
	}
}

void PSERVER::stopServer()
{
	freeaddrinfo(webSockInfo);
	freeaddrinfo(lisSockInfo);
	if (web_socket != INVALID_SOCKET) {
		closesocket(web_socket);
	}
	if (client_socket != INVALID_SOCKET) {
		closesocket(client_socket);
	}
	if (lis_socket != INVALID_SOCKET) {
		closesocket(lis_socket);
	}
	WSACleanup();
}

