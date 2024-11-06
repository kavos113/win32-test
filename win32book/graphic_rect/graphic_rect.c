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
        TEXT("Hello, Windows!"),
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
    HPEN hPen, hOldPen;
    HBRUSH hBrush, hOldBrush;
    
    switch (uMsg)
    {
    
    /* ---------------------------------------------------------- */
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        
        hPen = CreatePen(PS_SOLID, 0, RGB(255, 0, 0));
        hOldPen = (HPEN) SelectObject(hdc, hPen);
        
        hBrush = CreateSolidBrush(RGB(0, 255, 0));
        hOldBrush = (HBRUSH) SelectObject(hdc, hBrush);
        
        Rectangle(hdc, 10, 10, 100, 100);
        DeleteObject(hBrush);
        
        hBrush = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 0, 255));
        SelectObject(hdc, hBrush);
        Rectangle(hdc, 110, 10, 200, 100);
        DeleteObject(hBrush);
        
        hBrush = CreateHatchBrush(HS_CROSS, RGB(255, 0, 255));
        SelectObject(hdc, hBrush);
        Rectangle(hdc, 210, 10, 300, 100);
        DeleteObject(hBrush);
        
        hBrush = CreateHatchBrush(HS_DIAGCROSS, RGB(0, 255, 255));
        SelectObject(hdc, hBrush);
        Rectangle(hdc, 310, 10, 400, 100);
        DeleteObject(hBrush);
        
        hBrush = CreateHatchBrush(HS_FDIAGONAL, RGB(255, 255, 0));
        SelectObject(hdc, hBrush);
        Rectangle(hdc, 410, 10, 500, 100);
        DeleteObject(hBrush);
        
        hBrush = CreateHatchBrush(HS_HORIZONTAL, RGB(255, 0, 0));
        SelectObject(hdc, hBrush);
        Rectangle(hdc, 510, 10, 600, 100);
        DeleteObject(hBrush);
        
        hBrush = CreateHatchBrush(HS_VERTICAL, RGB(0, 255, 0));
        SelectObject(hdc, hBrush);
        Rectangle(hdc, 610, 10, 700, 100);
        DeleteObject(hBrush);
        
        hBrush = (HBRUSH) GetStockObject(NULL_BRUSH);
        SelectObject(hdc, hBrush);
        Rectangle(hdc, 650, 10, 800, 100);
        DeleteObject(hBrush);
        
        hBrush = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, hBrush);
        RoundRect(hdc, 10, 110, 100, 200, 20, 20);
        DeleteObject(hBrush);
        
        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        
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