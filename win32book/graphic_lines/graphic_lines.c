#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitHwnd(HINSTANCE hInstance, int nCmdShow);

const TCHAR CLASS_NAME[] = TEXT("Sample Window Class");

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    BOOL ret;
    
    if (!MyRegisterClass(hInstance))
    {
        return FALSE;
    }
    
    if (!InitHwnd(hInstance, nCmdShow))
    {
        return FALSE;
    }
    
    while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        // error
        if (ret == -1)
        {
            return -1;
        }
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wc;
    
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = (HICON) LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    wc.hCursor = (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_NAME;
    wc.hIconSm = (HICON) LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    
    return RegisterClassEx(&wc);
}

BOOL InitHwnd(HINSTANCE hInstance, int nCmdShow)
{
    HWND hwnd;
    
    hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        TEXT("Hello, Windows Lines"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );
    
    if (hwnd == NULL)
    {
        return FALSE;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    return TRUE;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    HPEN pen1, pen2, pen3, pen4;
    
    switch (uMsg)
    {
    
    /* ---------------------------------------------------------- */
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        
        pen1 = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
        SelectObject(hdc, pen1);
        MoveToEx(hdc, 100, 100, NULL);
        LineTo(hdc, 600, 100);
        
        pen2 = CreatePen(PS_DASH, 0, RGB(0, 255, 0));
        SelectObject(hdc, pen2);
        MoveToEx(hdc, 100, 200, NULL);
        LineTo(hdc, 600, 200);
        
        pen3 = CreatePen(PS_DOT, 0, RGB(0, 0, 255));
        SelectObject(hdc, pen3);
        MoveToEx(hdc, 100, 300, NULL);
        LineTo(hdc, 600, 300);
        
        pen4 = CreatePen(PS_DASHDOT, 0, RGB(255, 0, 255));
        SelectObject(hdc, pen4);
        MoveToEx(hdc, 100, 400, NULL);
        LineTo(hdc, 600, 400);
        
        EndPaint(hwnd, &ps);
        return 0;
    /* ---------------------------------------------------------- */
    
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}