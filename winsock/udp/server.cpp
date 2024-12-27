#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_PORT 27017

#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

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
    r = inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);
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