#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_PORT "27017"

#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

int main()
{
    // initialize winsock
    WSADATA wsaData;

    int r = WSAStartup(MAKEWORD(2, 2), &wsaData); // both use winsock version 2
    if (r != 0)
    {
        std::cerr << "WSAStartup failed with error: " << r << std::endl;
        return 1;
    }

    addrinfo *result = nullptr;
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    r = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);
    if (r != 0)
    {
        std::cerr << "getaddrinfo failed with error: " << r << std::endl;
        WSACleanup();
        return 1;
    }

    // create socket
    SOCKET listenSocket = INVALID_SOCKET;
    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET)
    {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // bind socket
    r = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (r == SOCKET_ERROR)
    {
        std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    std::cout << "Listening on port " << DEFAULT_PORT << std::endl;

    // listen for connections
    r = listen(listenSocket, SOMAXCONN);
    if (r == SOCKET_ERROR)
    {
        std::cerr << "listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // accept connection
    SOCKET clientSocket = INVALID_SOCKET;
    clientSocket = accept(listenSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "accept failed with error: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    closesocket(listenSocket);

    // send and receive data
    char recvbuf[512];
    int recvbuflen = 512;

    do
    {
        r = recv(clientSocket, recvbuf, recvbuflen, 0);
        if (r > 0)
        {
            recvbuf[r] = '\0';
            std::cout << "Received: " << recvbuf << std::endl;

            int sendr = send(clientSocket, recvbuf, r, 0);
            if (sendr == SOCKET_ERROR)
            {
                std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
                closesocket(clientSocket);
                WSACleanup();
                return 1;
            }
        }
        else if (r == 0)
        {
            std::cout << "Connection closed" << std::endl;
        }
        else
        {
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
    } while (r > 0);

    // shutdown server
    r = shutdown(clientSocket, SD_SEND);
    if (r == SOCKET_ERROR)
    {
        std::cerr << "shutdown failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}