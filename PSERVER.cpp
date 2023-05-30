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

void PSERVER::createNewSocket(SOCKET& new_socket)
{
	new_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (new_socket == INVALID_SOCKET) {
		throw ServException("Socket initialization error: ", WSAGetLastError());
	}

}

void PSERVER::transSockAddr(const char* ip_string, in_addr* ip_addr)
{
	errState = inet_pton(AF_INET, ip_string, ip_addr);
	if (errState <= 0) {
		throw ServException("Socket address translation error");
	}
}

void PSERVER::bindSocket()
{
	in_addr ip_addr;
	try
	{
		transSockAddr("127.0.0.1", &ip_addr);
	}
	catch (const ServException& ex)
	{
		throw;
	}

	sockaddr_in servInfo;
	ZeroMemory(&servInfo, sizeof(servInfo));
	servInfo.sin_family = AF_INET;
	servInfo.sin_port = htons(4444);
	servInfo.sin_addr = ip_addr;

	errState = bind(lis_socket, (sockaddr*)&servInfo, sizeof(servInfo));
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
	sockaddr_in clientInfo;
	int clientInfo_size = sizeof(clientInfo);
	ZeroMemory(&clientInfo, clientInfo_size);

	client_socket = accept(lis_socket, (sockaddr*)&clientInfo, &clientInfo_size);
	if (client_socket == INVALID_SOCKET) {
		throw ServException("Client connection error: ", WSAGetLastError());
	}
	std::cout << "Client has been connected" << std::endl;
}

void PSERVER::connectToWebServ()
{
	in_addr ip_addr;
	try
	{
		transSockAddr("127.0.0.1", &ip_addr);
	}
	catch (const std::exception&)
	{
		throw;
	}

	sockaddr_in servInfo;
	ZeroMemory(&servInfo, sizeof(servInfo));

	servInfo.sin_family = AF_INET;
	servInfo.sin_port = htons(4445);
	servInfo.sin_addr = ip_addr;

	errState = connect(web_socket, (sockaddr*)&servInfo, sizeof(servInfo));
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
		createNewSocket(lis_socket);
		bindSocket();
		listenState();
		while (true) {
			try
			{
				acceptConnection();
				createNewSocket(web_socket);
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

