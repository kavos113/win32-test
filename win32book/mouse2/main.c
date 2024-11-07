#define BMPWIDTH 300
#define BMPHEIGHT 100

#include <windows.h>

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
        CW_USEDEFAULT, CW_USEDEFAULT, BMPWIDTH, BMPHEIGHT,
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
    int id;
    HDC hdc;
    static HDC hdcMem;
    HBITMAP hBitmap;
    PAINTSTRUCT ps;
    
    TCHAR buf[64];
    const TCHAR format[] = TEXT("Mouse Position: (%d, %d), Capture: %s");
    static BOOL isCapture = FALSE;
    POINTS pts;
    
    switch (uMsg)
    {
    case WM_CREATE:
        hdc = GetDC(hwnd);
        hdcMem = CreateCompatibleDC(hdc);
        hBitmap = CreateCompatibleBitmap(hdc, BMPWIDTH, BMPHEIGHT);
        SelectObject(hdcMem, hBitmap);
        PatBlt(hdcMem, 0, 0, BMPWIDTH, BMPHEIGHT, WHITENESS);
        
        wsprintf(buf, format, 0, 0, isCapture ? TEXT("Yes") : TEXT("No"));
        TextOut(hdcMem, 10, 10, buf, lstrlen(buf));
        
        ReleaseDC(hwnd, hdc);
        DeleteObject(hBitmap);
        return 0;
        
    case WM_LBUTTONDOWN:
        if (!isCapture)
        {
            isCapture = TRUE;
            SetCapture(hwnd);
        }
        pts.x = LOWORD(lParam);
        pts.y = HIWORD(lParam);
        
        wsprintf(buf, format, pts.x, pts.y, isCapture ? TEXT("Yes") : TEXT("No"));
        PatBlt(hdcMem, 0, 0, BMPWIDTH, BMPHEIGHT, WHITENESS);
        TextOut(hdcMem, 10, 10, buf, lstrlen(buf));
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
        
    case WM_LBUTTONUP:
        if (isCapture)
        {
            isCapture = FALSE;
            ReleaseCapture();
        }
        pts.x = LOWORD(lParam);
        pts.y = HIWORD(lParam);
        
        wsprintf(buf, format, pts.x, pts.y, isCapture ? TEXT("Yes") : TEXT("No"));
        PatBlt(hdcMem, 0, 0, BMPWIDTH, BMPHEIGHT, WHITENESS);
        TextOut(hdcMem, 10, 10, buf, lstrlen(buf));
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
        
    case WM_MOUSEMOVE:
        pts.x = LOWORD(lParam);
        pts.y = HIWORD(lParam);
        
        wsprintf(buf, format, pts.x, pts.y, isCapture ? TEXT("Yes") : TEXT("No"));
        PatBlt(hdcMem, 0, 0, BMPWIDTH, BMPHEIGHT, WHITENESS);
        TextOut(hdcMem, 10, 10, buf, lstrlen(buf));
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
        
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        BitBlt(hdc, 0, 0, BMPWIDTH, BMPHEIGHT, hdcMem, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);
        return 0;
        
    case WM_CLOSE:
        id = MessageBox(hwnd, TEXT("Really Quit?"), TEXT("Quit"), MB_YESNO | MB_ICONQUESTION);
        if (id == IDYES)
        {
            DestroyWindow(hwnd);
        }
        return 0;
        
    case WM_DESTROY:
        DeleteDC(hdcMem);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}