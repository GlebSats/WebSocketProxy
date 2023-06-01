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
	client_socket = accept(lis_socket, (sockaddr*) & clientSockInfo, &clientSize);
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
	char buffer[BUFFER_SIZE] = {};
	int packet_size = 0;

	fd_set sendset;
	FD_ZERO(&sendset);
	FD_SET(client_socket, &sendset);
	FD_SET(web_socket, &sendset);

	while (true) {

		int readySock = select(0, &sendset, nullptr, nullptr, nullptr);
		if (readySock == SOCKET_ERROR) {
			throw ServException("Select runtime error: ", WSAGetLastError());
			shutdown(web_socket, SD_RECEIVE);
			shutdown(client_socket, SD_SEND);
		}
		if (FD_ISSET(web_socket, &sendset)) {
			packet_size = recv(web_socket, buffer, BUFFER_SIZE, 0);
			if (packet_size == SOCKET_ERROR) {
				shutdown(web_socket, SD_RECEIVE);
				shutdown(client_socket, SD_SEND);
				throw ServException("Connection with the client has been severed: ", WSAGetLastError());
			}
			packet_size = send(client_socket, buffer, packet_size, 0);
			if (packet_size == SOCKET_ERROR) {
				shutdown(web_socket, SD_RECEIVE);
				shutdown(client_socket, SD_SEND);
				throw ServException("Connection with the server has been severed: ", WSAGetLastError());
			}
		}
		if (FD_ISSET(client_socket, &sendset)) {
			packet_size = recv(client_socket, buffer, BUFFER_SIZE, 0);
			if (packet_size == SOCKET_ERROR) {
				shutdown(web_socket, SD_RECEIVE);
				shutdown(client_socket, SD_SEND);
				throw ServException("Connection with the client has been severed: ", WSAGetLastError());
			}
			packet_size = send(web_socket, buffer, packet_size, 0);
			if (packet_size == SOCKET_ERROR) {
				shutdown(web_socket, SD_RECEIVE);
				shutdown(client_socket, SD_SEND);
				throw ServException("Connection with the server has been severed: ", WSAGetLastError());
			}
		}
	}
}

void PSERVER::startServer()
{
	try
	{
		initSockets();
		//
		createSockInfo("127.0.0.1", "4444", &lisSockInfo);
		//
		createNewSocket(lis_socket, lisSockInfo);
		bindSocket();
		listenState();
		while (true) {
			try
			{
				acceptConnection();
				//
				createSockInfo("127.0.0.1", "4445", &webSockInfo);
				//
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

