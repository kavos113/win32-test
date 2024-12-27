#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_PORT "27017"

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

    addrinfo *result = nullptr;
    addrinfo *ptr = nullptr;
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    r = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (r != 0)
    {
        std::cerr << "getaddrinfo failed with error: " << r << std::endl;
        WSACleanup();
        return 1;
    }

    // create socket
    SOCKET connectSocket = INVALID_SOCKET;
    connectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (connectSocket == INVALID_SOCKET)
    {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // connect to server
    for (ptr = result; ptr != nullptr; ptr = ptr->ai_next)
    {
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET)
        {
            std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
            freeaddrinfo(result);
            WSACleanup();
            return 1;
        }

        // retry
        r = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (r == SOCKET_ERROR)
        {
            std::cerr << "connect failed with error: " << WSAGetLastError() << std::endl;
            closesocket(connectSocket);
            connectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(result);

    if (connectSocket == INVALID_SOCKET)
    {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // send and receive data
    const char* message = "Hello, world!";
    int recvbuflen = 512;
    char recvbuf[512];

    r = send(connectSocket, message, (int)strlen(message), 0);
    if (r == SOCKET_ERROR)
    {
        std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Sent: " << message << std::endl;

    r = shutdown(connectSocket, SD_SEND);
    if (r == SOCKET_ERROR)
    {
        std::cerr << "shutdown failed with error: " << WSAGetLastError() << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    do
    {
        r = recv(connectSocket, recvbuf, recvbuflen, 0);
        if (r > 0)
        {
            recvbuf[r] = '\0';
            std::cout << "Received: " << recvbuf << std::endl;
        }
        else if (r == 0)
        {
            std::cout << "Connection closed" << std::endl;
        }
        else
        {
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
            closesocket(connectSocket);
            WSACleanup();
        }
    } while (r > 0);

    // shutdown winsock
    closesocket(connectSocket);
    WSACleanup();

    return 0;
}