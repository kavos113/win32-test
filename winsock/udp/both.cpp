#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_PORT 27017

#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>

int sendvalue(char* ipaddr);
int recieve();

int main(int argc, char** argv)
{
    // initialize winsock
    WSADATA wsaData;

    int r = WSAStartup(MAKEWORD(2, 2), &wsaData); // both use winsock version 2
    if (r != 0)
    {
        std::cerr << "WSAStartup failed with error: " << r << std::endl;
        return 1;
    }

    std::thread t(recieve);
    t.join();
}

int sendvalue(char* ipaddr)
{
    auto serverSocket = INVALID_SOCKET;
    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DEFAULT_PORT);
    int r = inet_pton(AF_INET, ipaddr, &serverAddr.sin_addr);
    if (r == 0)
    {
        std::cerr << "inet_pton failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    r = sendto(serverSocket, "Hello, world!", 13, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (r == SOCKET_ERROR)
    {
        std::cerr << "sendto failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}

int recieve()
{
    auto clientSocket = INVALID_SOCKET;
    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DEFAULT_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    int r = bind(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (r == SOCKET_ERROR)
    {
        std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // blocking code
    u_long nonblock = 0;
    r = ioctlsocket(clientSocket, FIONBIO, &nonblock);
    if (r == SOCKET_ERROR)
    {
        std::cerr << "ioctlsocket failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    char recvbuf[512];
    r = recv(clientSocket, recvbuf, 512, 0);
    if (r == SOCKET_ERROR)
    {
        std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Received: " << recvbuf << std::endl;

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}