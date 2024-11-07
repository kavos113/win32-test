#include <windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitHwnd(HINSTANCE hInstance, int nCmdShow);
HINSTANCE hInstance;

const TCHAR CLASS_NAME[] = TEXT("Sample Window Class");

int WINAPI WinMain(HINSTANCE hCurInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    MSG msg;
    BOOL ret;
    hInstance = hCurInstance;
    
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

ATOM MyRegisterClass(HINSTANCE hInst)
{
    WNDCLASSEX wc;
    
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = (HICON) LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    wc.hCursor = (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_NAME;
    wc.hIconSm = (HICON) LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    
    return RegisterClassEx(&wc);
}

BOOL InitHwnd(HINSTANCE hInst, int nCmdShow)
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
        hInst,
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
    HDC hdc, hdcMem;
    PAINTSTRUCT ps;
    HBITMAP hBitmap;
    BITMAP bm;
    int w, h;
    
    switch (uMsg)
    {
    
    /* ---------------------------------------------------------- */
    /* メモリデバイスコンテキストにWM_CREATEで作っておいてWM_PAINTの時は転送するだけにしてもよい */
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        hBitmap = LoadBitmap(hInstance, TEXT("MYBMP"));
        
        GetObject(hBitmap, sizeof(BITMAP), &bm);
        w = bm.bmWidth;
        h = bm.bmHeight;
        
        hdcMem = CreateCompatibleDC(hdc);
        SelectObject(hdcMem, hBitmap);
        
        BitBlt(hdc,
               0,
               0,
               w,
               h,
               hdcMem,
               0,
               0,
               NOTSRCCOPY
               );
        
        StretchBlt(hdc,
                   500,
                   500,
                   w * 2,
                   h * 2,
                   hdcMem,
                   100,
                   100,
                   w,
                   h,
                   SRCCOPY
                   );
        
        DeleteDC(hdcMem);
        DeleteObject(hBitmap);
        
        EndPaint(hwnd, &ps);
        return 0;
    /* ---------------------------------------------------------- */
    
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}