#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_PORT 27017

#include <windows.h>
#include <WinSock2.h>
#include <iostream>
#include <shellapi.h>
#include <thread>
#include <WS2tcpip.h>

#include "BaseWindow.h"
#include "MainWindow.h"

int receive();
int send(const char* ipaddr);

std::string wcharToString(const wchar_t* wstr)
{
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (sizeNeeded == 0)
    {
        throw std::runtime_error("WideCharToMultiByte failed");
    }

    std::string str(sizeNeeded - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], sizeNeeded, nullptr, nullptr);

    return str;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lCmdLine, int nCmdShow)
{
    int numArgs;
    LPWSTR* args = CommandLineToArgvW(GetCommandLineW(), &numArgs);

    if (numArgs != 3)
    {
        std::cerr << "Usage: udp.exe server/client <ipaddr>" << std::endl;
        return 0;
    }

    // initialize winsock
    WSADATA wsaData;

    int r = WSAStartup(MAKEWORD(2, 2), &wsaData); // both use winsock version 2
    if (r != 0)
    {
        std::cerr << "WSAStartup failed with error: " << r << std::endl;
        return 1;
    }

    std::unique_ptr<MainWindow> window;
    if (wcscmp(args[1], L"server") == 0)
    {
        window = std::make_unique<MainWindow>(true, wcharToString(args[2]));
    }
    else if (wcscmp(args[1], L"client") == 0)
    {
        window = std::make_unique<MainWindow>(false, wcharToString(args[2]));
    }
    else
    {
        std::cerr << "Usage: udp.exe server/client" << std::endl;
        return 0;
    }

    HRESULT hr = window->Create(
        "Winsock sample",
        WS_OVERLAPPEDWINDOW,
        0,
        100,
        100,
        800,
        600
        );
    if (FAILED(hr))
    {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }
    window->SetUp();

    std::thread t1(receive);

    ShowWindow(window->Window(), nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    t1.join();

    return 0;
}

int receive()
{
    auto clientSocket = INVALID_SOCKET;
    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DEFAULT_PORT);
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

    int r = bind(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (r == SOCKET_ERROR)
    {
        std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Listening on port " << DEFAULT_PORT << std::endl;

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

    while (true)
    {
        char recvbuf[sizeof(float) * 2];
        r = recv(clientSocket, recvbuf, sizeof(float) * 2, 0);
        if (r == SOCKET_ERROR)
        {
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        float val[2];
        memcpy(val, recvbuf, sizeof(float) * 2);

        std::cout << "Received: " << val[0] << ", " << val[1] << std::endl;
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

int send(const char* ipaddr)
{
    auto serverSocket = INVALID_SOCKET;
    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
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