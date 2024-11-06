#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitHwnd(HINSTANCE hInstance, int nCmdShow);
HFONT MyCreateFont(int height, DWORD dwCharset, LPCTSTR lpFaceName);

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

/* ---------------------------------------------------------- */
HFONT MyCreateFont(int height, DWORD dwCharset, LPCTSTR lpFaceName)
{
    HFONT hFont;
    
    hFont = CreateFont(
        height,
        0,
        0,
        0,
        FW_DONTCARE,
        FALSE, // italic
        FALSE, // underline
        TRUE,  // strikeout
        dwCharset,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        lpFaceName
    );
    
    return hFont;
}
/* ---------------------------------------------------------- */

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rc;
    LPCTSTR text = TEXT("Hello, Windows!");
    LPCTSTR text2 = TEXT("Hello, Windows?");
    DRAWTEXTPARAMS dtp;
    static HFONT hFont1, hFont2, hFont3;
    
    switch (uMsg)
    {
    
    /* ---------------------------------------------------------- */
    case WM_CREATE:
        hFont1 = MyCreateFont(20, ANSI_CHARSET, TEXT("Times New Roman"));
        hFont2 = MyCreateFont(20, ANSI_CHARSET, TEXT("Arial"));
        hFont3 = MyCreateFont(20, ANSI_CHARSET, TEXT("Courier New"));
        return 0;
    case WM_DESTROY:
        DeleteObject(hFont1);
        DeleteObject(hFont2);
        DeleteObject(hFont3);
        PostQuitMessage(0);
        return 0;
        
    case WM_PAINT:
    {
        GetClientRect(hwnd, &rc);
        rc.bottom = rc.top + 100;
        rc.right = rc.left + 200;
        
        dtp.cbSize = sizeof(DRAWTEXTPARAMS);
        dtp.iLeftMargin = 20;
        dtp.iRightMargin = 20;
        dtp.iTabLength = 4;
        
        hdc = BeginPaint(hwnd, &ps);
        
        SetTextColor(hdc, RGB(0, 128, 128));
        SelectObject(hdc, hFont1);
        DrawTextEx(hdc,
                   text,
                   -1,
                   &rc,
                   DT_SINGLELINE | DT_CENTER | DT_VCENTER,
                   &dtp);
        
        rc.top += 100;
        rc.bottom += 100;
        
        SetTextColor(hdc, RGB(128, 0, 128));
        SelectObject(hdc, hFont2);
        DrawTextEx(hdc,
                   text2,
                   -1,
                   &rc,
                   DT_SINGLELINE | DT_CENTER | DT_VCENTER,
                   &dtp);
        
        rc.top += 100;
        rc.bottom += 100;
        
        SetTextColor(hdc, RGB(128, 128, 0));
        SelectObject(hdc, hFont3);
        DrawTextEx(hdc,
                   text,
                   -1,
                   &rc,
                   DT_SINGLELINE | DT_CENTER | DT_VCENTER,
                   &dtp);
        
        EndPaint(hwnd, &ps);
    }
        return 0;
    /* ---------------------------------------------------------- */
    
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}