#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitHwnd(HINSTANCE hInstance, int nCmdShow);
void DrawRect(HWND hwnd, POINTS topleftpoint, POINTS bottomrightpoint);

const TCHAR CLASS_NAME[] = TEXT("Sample Window Class");
POINTS topleft, bottomright, oldButtomright;

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
    TCHAR buf[32];
    HDC hdc;
    HBRUSH hBrush;
    static BOOL bDraw = FALSE;
    
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
        bDraw = TRUE;
        topleft = MAKEPOINTS(lParam);
        oldButtomright = topleft;
        DrawRect(hwnd, topleft, oldButtomright);
        return 0;
        
    case WM_MOUSEMOVE:
        if (bDraw)
        {
            bottomright = MAKEPOINTS(lParam);
            DrawRect(hwnd, topleft, oldButtomright);
            DrawRect(hwnd, topleft, bottomright);
            oldButtomright = bottomright;
        }
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        return 0;
        
    case WM_LBUTTONUP:
        if (bDraw)
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            DrawRect(hwnd, topleft, oldButtomright);
            
            bDraw = FALSE;
            
            wsprintf(buf, TEXT("RECT: (%d, %d) - (%d, %d)"),
                     topleft.x, topleft.y, bottomright.x, bottomright.y);
            SetWindowText(hwnd, buf);
            
            hdc = GetDC(hwnd);
            hBrush = (HBRUSH) GetStockObject(NULL_BRUSH);
            SelectObject(hdc, hBrush);
            Ellipse(hdc, topleft.x, topleft.y, bottomright.x, bottomright.y);
            ReleaseDC(hwnd, hdc);
        }
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        return 0;
        
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}

void DrawRect(HWND hwnd, POINTS topleftpoint, POINTS bottomrightpoint)
{
    HDC hdc;
    HPEN hPen, hOldPen;
    
    hdc = GetDC(hwnd);
    SetROP2(hdc, R2_NOT);
    
    hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    hOldPen = (HPEN) SelectObject(hdc, hPen);
    
    MoveToEx(hdc, topleftpoint.x, topleftpoint.y, NULL);
    LineTo(hdc, bottomrightpoint.x, topleftpoint.y);
    LineTo(hdc, bottomrightpoint.x, bottomrightpoint.y);
    LineTo(hdc, topleftpoint.x, bottomrightpoint.y);
    LineTo(hdc, topleftpoint.x, topleftpoint.y);
    
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    ReleaseDC(hwnd, hdc);
}