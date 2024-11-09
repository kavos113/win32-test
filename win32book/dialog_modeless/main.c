#include <windows.h>
#include <stdio.h>

#include "resource.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitHwnd(HINSTANCE hInstance, int nCmdShow);

const TCHAR CLASS_NAME[] = TEXT("Sample Window Class");
HINSTANCE hInst;
HWND hDlg;
TCHAR szText[256];
HWND hMain;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    BOOL ret;
    
    hInst = hInstance;
    
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
        
        if (!hDlg || !IsDialogMessage(hDlg, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
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
    wc.lpszMenuName = TEXT("MYMENU");
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
        1000,
        600,
        800,
        600,
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
    static HMENU hMenu;
    TCHAR szBuf[256];
    HDC hdc;
    PAINTSTRUCT ps;
    
    switch (uMsg)
    {
    case WM_CREATE:
        hMenu = GetMenu(hwnd);
        return 0;
        
    case WM_INITMENU:
        if (IsWindow(hDlg))
        {
            EnableMenuItem(hMenu, IDM_DIALOG, MF_GRAYED);
        }
        else
        {
            EnableMenuItem(hMenu, IDM_DIALOG, MF_ENABLED);
        }
        DrawMenuBar(hwnd);
        return 0;
        
    case WM_PAINT:
        if (lstrcmp(szText, TEXT("")) == 0)
        {
            lstrcpy(szBuf, TEXT("No Input"));
        }
        else
        {
            wsprintf(szBuf, TEXT("Input: %s"), szText);
        }
        
        hdc = BeginPaint(hwnd, &ps);
        TextOut(hdc, 10, 10, szBuf, lstrlen(szBuf));
        EndPaint(hwnd, &ps);
        return 0;
        
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case IDM_DIALOG:
            hDlg = CreateDialog(hInst, TEXT("MYDIALOG"), hwnd, DialogProc);
            ShowWindow(hDlg, SW_NORMAL);
            break;
        case IDM_ABOUT:
            MessageBox(hwnd, TEXT("Hello, Windows!"), TEXT("About"), MB_OK);
            break;
        }
        return 0;
    case WM_DESTROY:
        if (IsWindow(hDlg))
        {
            DestroyWindow(hDlg);
        }
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hParent;
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        hParent = (HWND) lParam;
        return TRUE;
        
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            GetDlgItemText(hwnd, IDC_EDIT, szText, 256);
            InvalidateRect(hParent, NULL, TRUE);
            EndDialog(hwnd, 0);
            hDlg = NULL;
            break;
        case IDCANCEL:
            EndDialog(hwnd, 0);
            hDlg = NULL;
            break;
        }
        return TRUE;
    }
    return FALSE;
}