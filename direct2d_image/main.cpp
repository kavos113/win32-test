#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "windowscodecs")

#include "main_window.h"



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    MainWindow window;
    
    if (!window.Create(L"Direct2D sample", WS_OVERLAPPEDWINDOW))
    {
        return 0;
    }
    window.Init();
    
    ShowWindow(window.Window(), nCmdShow);
    
    // run message loop
    
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}

