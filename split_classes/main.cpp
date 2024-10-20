#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include "base_window.h"


class MainWindow : public BaseWindow
{
public:
    PCWSTR ClassName() const
    {
        return L"This is Sample Class";
    }
    
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    MainWindow win;
    
    if (!win.Create(L"Sample Window", WS_OVERLAPPEDWINDOW))
    {
        return 0;
    }
    
    ShowWindow(win.Window(), nCmdShow);
    
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OutputDebugString(L"HandleMessage\n");
    
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(m_hwnd, &ps);
        
        FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));
        
        EndPaint(m_hwnd, &ps);
    }
        return 0;
    
    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
    
    return TRUE;
}