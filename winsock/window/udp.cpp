#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_PORT 27017

#include <windows.h>
#include <WinSock2.h>
#include <iostream>
#include <shellapi.h>

#include "BaseWindow.h"
#include "MainWindow.h"

int receive();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lCmdLine, int nCmdShow)
{
    int numArgs;
    LPWSTR* args = CommandLineToArgvW(GetCommandLineW(), &numArgs);

    if (numArgs != 2)
    {
        std::cerr << "Usage: udp.exe server/client" << std::endl;
        return 0;
    }

    std::unique_ptr<MainWindow> window;
    if (wcscmp(args[1], L"server") == 0)
    {
        window = std::make_unique<MainWindow>(true);
    }
    else if (wcscmp(args[1], L"client") == 0)
    {
        window = std::make_unique<MainWindow>(false);
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

    ShowWindow(window->Window(), nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

int receive()
{
    // initialize winsock
    WSADATA wsaData;

    int r = WSAStartup(MAKEWORD(2, 2), &wsaData); // both use winsock version 2
    if (r != 0)
    {
        std::cerr << "WSAStartup failed with error: " << r << std::endl;
        return 1;
    }

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
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

    r = bind(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
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