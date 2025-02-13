#ifndef UNICODE
#define UNICODE
#endif

#include <thread>
#include <windows.h>

#define WM_CUSTOM_MESSAGE (WM_USER + 1)

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void Post(HWND hwnd)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    PostMessage(hwnd, WM_CUSTOM_MESSAGE, 0, 0);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"sample window class";
    
    WNDCLASS wc = {};
    
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"window test",
        WS_OVERLAPPEDWINDOW,
        
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,

        nullptr,
        nullptr,
        hInstance,
        nullptr
    );
    if (hwnd == nullptr)
    {
        return 1;
    }
    
    ShowWindow(hwnd, nCmdShow);

    std::thread t(Post, hwnd);
    
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    t.join();
    
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            FillRect(hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>((COLOR_WINDOW + 1)));
            
            EndPaint(hwnd, &ps);
        }
        return 0;

    case WM_CUSTOM_MESSAGE:
    {
        MessageBox(hwnd, L"Custom message", L"Custom message", MB_OK);
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}