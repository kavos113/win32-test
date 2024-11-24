#include <windows.h>

#include "Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    Application app;
    RECT wr = { 0, 0, 1280, 720 };

    app.Init(wr);
    app.Run();

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    app.Cleanup();

    return 0;
}