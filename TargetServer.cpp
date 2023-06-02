#include "TSERVER.h"
#include "ServException.h"

#define BUFFER_SIZE 4

TSERVER::~TSERVER() {
	stopServer();
}


void TSERVER::initSockets()
{
	errState = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (errState != 0) {
		throw ServException("Initialization version error: ", WSAGetLastError());
	}
}

void TSERVER::createNewSocket(SOCKET& new_socket)
{
	new_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (new_socket == INVALID_SOCKET) {
		throw ServException("Socket initialization error: ", WSAGetLastError());
	}

}

void TSERVER::transSockAddr(const char* ip_string, in_addr* ip_addr)
{
	errState = inet_pton(AF_INET, ip_string, ip_addr);
	if (errState <= 0) {
		throw ServException("Socket address translation error");
	}
}

void TSERVER::bindSocket()
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
	servInfo.sin_port = htons(4445);
	servInfo.sin_addr = ip_addr;

	errState = bind(lis_socket, (sockaddr*)&servInfo, sizeof(servInfo));
	if (errState != 0) {
		throw ServException("Binding error: ", WSAGetLastError());
	}
}

void TSERVER::listenState()
{
	errState = listen(lis_socket, SOMAXCONN);
	if (errState != 0) {
		throw ServException("Listening error: ", WSAGetLastError());
	}
	std::cout << "Server in listening state..." << std::endl;
}

void TSERVER::acceptConnection()
{
	sockaddr_in clientInfo;
	int clientInfo_size = sizeof(clientInfo);
	ZeroMemory(&clientInfo, clientInfo_size);

	client_socket = accept(lis_socket, (sockaddr*)&clientInfo, &clientInfo_size);
	if (client_socket == INVALID_SOCKET) {
		closesocket(client_socket);
		throw ServException("Client connection error: ", WSAGetLastError());
	}
	std::cout << "Client has been connected" << std::endl;
}

void TSERVER::connectToWebServ()
{
	in_addr ip_addr;
	try
	{
		transSockAddr("91.121.93.94", &ip_addr);
	}
	catch (const std::exception&)
	{
		throw;
	}

	sockaddr_in servInfo;
	ZeroMemory(&servInfo, sizeof(servInfo));

	servInfo.sin_family = AF_INET;
	servInfo.sin_port = htons(8080);
	servInfo.sin_addr = ip_addr;

	errState = connect(web_socket, (sockaddr*)&servInfo, sizeof(servInfo));
	if (errState != 0) {
		throw ServException("Connection to Web Server failed: ", WSAGetLastError());
	}
	std::cout << "Connection to Web Server successful" << std::endl;
}

void TSERVER::sockCommunication()
{
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	int packet_size = 0;
	while (true) {
		do
		{
			packet_size = recv(client_socket, buffer, BUFFER_SIZE, 0);
			int i = 0;
			for (int i = 0; i < BUFFER_SIZE; i++) {
				if (buffer[i] == '\0') {
					break;
				}
				std::cout << buffer[i];
			}
			memset(buffer, 0, BUFFER_SIZE);
		} while (packet_size == BUFFER_SIZE);
		std::cout << std::endl;
	}
}

void TSERVER::startServer()
{
	try
	{
		initSockets();
		createNewSocket(lis_socket);
		bindSocket();
		listenState();

		acceptConnection();
		//test
		//createNewSocket(web_socket);
		//connectToWebServ();
		sockCommunication();
	}
	catch (const ServException& ex)
	{
		std::cout << ex.GetErrorType() << ex.GetErrorCode() << std::endl;
	}
}

void TSERVER::stopServer()
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
