#include <windows.h>
#include <time.h>


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitHwnd(HINSTANCE hInstance, int nCmdShow);
int TypeStart(HWND hwnd);

const TCHAR CLASS_NAME[] = TEXT("Sample Window Class");
TCHAR problem[32], input[32], check[32];
int iProblem;
DWORD dwStart, dwEnd;
BOOL bStart = FALSE;
BOOL bCorrect = TRUE;

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
        TEXT("Type Speed Test"),
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
    static HMENU hMenu;
    MMTIME mmtime;
    
    switch (uMsg)
    {
    case WM_CREATE:
        srand((unsigned) time(NULL));
        hMenu = GetMenu(hwnd);
        return 0;
        
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        
        TextOut(hdc, 10, 10, problem, lstrlen(problem));
        TextOut(hdc, 10, 50, input, lstrlen(input));
        if (bCorrect)
        {
            SetTextColor(hdc, RGB(0, 0, 0));
        }
        else
        {
            SetTextColor(hdc, RGB(255, 0, 0));
        }
        
        TextOut(hdc, 10, 90, check, lstrlen(check));
        
        EndPaint(hwnd, &ps);
        return 0;
        
    case WM_CHAR:
        if (wParam == VK_SPACE && !bStart)
        {
            bStart = TRUE;
            TypeStart(hwnd);
            return 0;
        }
        
        if (bStart == FALSE)
        {
            return 0;
        }
        
        if (wParam == VK_ESCAPE)
        {
            lstrcpy(problem, TEXT(""));
            lstrcpy(input, TEXT(""));
            lstrcpy(check, TEXT(""));
            InvalidateRect(hwnd, NULL, TRUE);
            bStart = FALSE;
            
            return 0;
        }
        
        wsprintf(input, TEXT("YOUR INPUT: %c"), (TCHAR) wParam);
        if (iProblem == wParam)
        {
            bCorrect = TRUE;
            
            mmtime.wType = TIME_MS;
            timeGetSystemTime(&mmtime, sizeof(MMTIME));
            dwEnd = mmtime.u.ms;
            
            wsprintf(check, TEXT("CORRECT! %d ms"), dwEnd - dwStart);
            
            TypeStart(hwnd);
        }
        else
        {
            bCorrect = FALSE;
            wsprintf(check, TEXT("WRONG!"));
        }
        
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
        
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
        
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int TypeStart(HWND hwnd)
{
    MMTIME mmtime;
    
    iProblem = rand() % 26 + 'a';
    wsprintf(problem, TEXT("TYPE: %c"), (TCHAR) iProblem);
    
    mmtime.wType = TIME_MS;
    timeGetSystemTime(&mmtime, sizeof(MMTIME));
    dwStart = mmtime.u.ms;
    
    InvalidateRect(hwnd, NULL, TRUE);
    return 0;
}