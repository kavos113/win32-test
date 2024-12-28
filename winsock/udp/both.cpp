#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_PORT 28020

#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>

int sendvalue();
int recieve();

auto sock = INVALID_SOCKET;
sockaddr_in serverAddr;

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

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DEFAULT_PORT);
    r = inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);
    if (r == 0)
    {
        std::cerr << "inet_pton failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(DEFAULT_PORT);
    clientAddr.sin_addr.s_addr = INADDR_ANY;

    r = bind(sock, (sockaddr*)&clientAddr, sizeof(clientAddr));
    if (r == SOCKET_ERROR)
    {
        std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Listening..." << std::endl;

    std::thread t(recieve);

    for (int i = 0; i < 30; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        std::cout << "Sending value..." << std::endl;
        sendvalue();
        std::cout << "Done" << std::endl;
    }


    t.join();

    closesocket(sock);
    WSACleanup();
}

int sendvalue()
{
    int r = sendto(sock, "Hello, world!", 13, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (r == SOCKET_ERROR)
    {
        std::cerr << "sendto failed with error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    return 0;
}

int recieve()
{
    while (true)
    {
        char recvbuf[16];
        int r = recv(sock, recvbuf, 16, 0);
        if (r == SOCKET_ERROR)
        {
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        std::cout << "Received: " << recvbuf << std::endl;
    }

    return 0;
}