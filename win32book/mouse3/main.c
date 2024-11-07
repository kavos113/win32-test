#include <windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitHwnd(HINSTANCE hInstance, int nCmdShow);
BOOL MyAdjustWindow(HWND hwnd, int width, int height);

const TCHAR CLASS_NAME[] = TEXT("Sample Window Class");
HINSTANCE hInstance;

int WINAPI WinMain(HINSTANCE hCurInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
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

BOOL MyAdjustWindow(HWND hwnd, int width, int height)
{
    RECT rc;
    int x, y, w, h, winx, winy;
    
    w = GetSystemMetrics(SM_CXSCREEN);
    h = GetSystemMetrics(SM_CYSCREEN);
    
    rc.left = 0;
    rc.top = 0;
    rc.right = width;
    rc.bottom = height;
    
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    winx = rc.right - rc.left;
    winy = rc.bottom - rc.top;
    x = (w - winx) / 2;
    y = (h - winy) / 2;
    
    MoveWindow(hwnd, x, y, winx, winy, FALSE);
    
    return TRUE;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int id;
    int rcwidth = 80;
    int rcheight = 40;
    int moucex, moucey;
    static int customwidth = 0;
    static int customheight = 0;
    HBITMAP hBitmap;
    static HBITMAP hCustomBitmap;
    static RECT rc1, rc2;
    PAINTSTRUCT ps;
    static BOOL bDrag, bDragCustom;
    static POINT ptStart;
    BITMAP bm;
    static HDC hdcmem, hdccustom;
    HDC hdc;
    
    switch (uMsg)
    {
    case WM_CREATE:
        MyAdjustWindow(hwnd, 800, 600);
        
        hdcmem = CreateCompatibleDC(NULL);
        hdc = GetDC(hwnd);
        hBitmap = CreateCompatibleBitmap(hdc, 600, 400);
        SelectObject(hdcmem, hBitmap);
        PatBlt(hdcmem, 0, 0, 600, 400, WHITENESS);
        rc1.left = 10;
        rc1.top = 10;
        rc1.right = rc1.left + rcwidth;
        rc1.bottom = rc1.top + rcheight;
        Rectangle(hdcmem, rc1.left, rc1.top, rc1.right, rc1.bottom);
        
        hCustomBitmap = (HBITMAP) LoadImage(NULL, TEXT("MYBMP"), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
        GetObject(hCustomBitmap, sizeof(BITMAP), &bm);
        customwidth = bm.bmWidth;
        customheight = bm.bmHeight;
        hdccustom = CreateCompatibleDC(NULL);
        SelectObject(hdccustom, hCustomBitmap);
        rc2.left = 100;
        rc2.top = 100;
        rc2.right = rc2.left + customwidth;
        rc2.bottom = rc2.top + customheight;
        BitBlt(hdcmem, rc2.left, rc2.top, customwidth, customheight, hdccustom, 0, 0, SRCCOPY);
        
        DeleteObject(hBitmap);
        ReleaseDC(hwnd, hdc);
        
        return 0;
        
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        
        BitBlt(hdc, 0, 0, 600, 400, hdcmem, 0, 0, SRCCOPY);
        
        EndPaint(hwnd, &ps);
        return 0;
        
    case WM_LBUTTONDOWN:
        moucex = LOWORD(lParam);
        moucey = HIWORD(lParam);
        
        if (moucex > rc1.left && moucex < rc1.right && moucey > rc1.top && moucey < rc1.bottom)
        {
            bDrag = TRUE;
        }
        
        if (moucex > rc2.left && moucex < rc2.right && moucey > rc2.top && moucey < rc2.bottom)
        {
            bDragCustom = TRUE;
        }
        
        if (!bDrag && !bDragCustom)
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        
        ptStart.x = moucex;
        ptStart.y = moucey;
        SetCapture(hwnd);
        
        return 0;
        
    case WM_MOUSEMOVE:
        if (!bDrag && !bDragCustom)
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        
        moucex = LOWORD(lParam);
        moucey = HIWORD(lParam);
        
        if (bDrag)
        {
            rc1.left += moucex - ptStart.x;
            rc1.top += moucey - ptStart.y;
            rc1.right = rc1.left + rcwidth;
            rc1.bottom = rc1.top + rcheight;
        }
        
        if (bDragCustom)
        {
            rc2.left += moucex - ptStart.x;
            rc2.top += moucey - ptStart.y;
            rc2.right = rc2.left + customwidth;
            rc2.bottom = rc2.top + customheight;
        }
        
        PatBlt(hdcmem, 0, 0, 600, 400, WHITENESS);
        Rectangle(hdcmem, rc1.left, rc1.top, rc1.right, rc1.bottom);
        BitBlt(hdcmem, rc2.left, rc2.top, customwidth, customheight, hdccustom, 0, 0, SRCCOPY);
        
        InvalidateRect(hwnd, NULL, FALSE);
        ptStart.x = moucex;
        ptStart.y = moucey;
        
        return 0;
        
    case WM_LBUTTONUP:
        if (!bDrag && !bDragCustom)
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        
        if (bDrag)
        {
            bDrag = FALSE;
        }
        
        if (bDragCustom)
        {
            bDragCustom = FALSE;
        }
        
        ReleaseCapture();
        
        return 0;
    
    case WM_CLOSE:
        id = MessageBox(hwnd, TEXT("Really Quit?"), TEXT("Confirm"), MB_YESNO | MB_ICONQUESTION);
        
        if (id == IDYES)
        {
            DestroyWindow(hwnd);
        }
        
        return 0;
        
    case WM_DESTROY:
        if (bDrag || bDragCustom)
        {
            ReleaseCapture();
        }
        
        DeleteObject(hCustomBitmap);
        DeleteDC(hdccustom);
        DeleteDC(hdcmem);
        PostQuitMessage(0);
        return 0;
        
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}