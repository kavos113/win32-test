#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_PORT 27018

#include <windows.h>
#include <WinSock2.h>
#include <iostream>
#include <shellapi.h>
#include <thread>
#include <WS2tcpip.h>

#include "BaseWindow.h"
#include "MainWindow.h"

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
        std::cout << "Usage: udp.exe server/client <ipaddr>" << std::endl;
        return 0;
    }

    // initialize winsock
    WSADATA wsaData;

    int r = WSAStartup(MAKEWORD(2, 2), &wsaData); // both use winsock version 2
    if (r != 0)
    {
        std::cout << "WSAStartup failed with error: " << r << std::endl;
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
        std::cout << "Usage: udp.exe server/client" << std::endl;
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
        std::cout << "Failed to create window" << std::endl;
        return 1;
    }
    window->SetUp();

    std::thread t1(&MainWindow::Listen, window.get());

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
