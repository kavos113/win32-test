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
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rc;
    LPCTSTR text = TEXT("Hello, Windows!\tsssssss");
    DRAWTEXTPARAMS dtp;
    
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
        
    /* ----------------------------------------------- */
    case WM_PAINT:
        {
            GetClientRect(hwnd, &rc);
            
            dtp.cbSize = sizeof(DRAWTEXTPARAMS);
            dtp.iLeftMargin = 20;
            dtp.iRightMargin = 20;
            dtp.iTabLength = 4;
            
            hdc = BeginPaint(hwnd, &ps);
            
            /* TextOut(hdc, 0, 0, text, lstrlen(text)); */

            /*
            DrawText(hdc,
                     text,
                     -1,
                     &rc,
                     DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            */
            
            SetTextColor(hdc, RGB(0, 128, 128));
            DrawTextEx(hdc,
                       text,
                       -1,
                       &rc,
                       DT_SINGLELINE | DT_CENTER | DT_VCENTER,
                       &dtp);
            
            EndPaint(hwnd, &ps);
        }
        return 0;
    /* ----------------------------------------------- */
    
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}