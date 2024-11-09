#define ID_TOOLBAR 100

#include <windows.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#include "resource.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitHwnd(HINSTANCE hInstance, int nCmdShow);
HWND MyCreateToolbar(HWND hwnd);

const TCHAR CLASS_NAME[] = TEXT("Sample Window Class");
int type = 0;
HINSTANCE hInst;

TBBUTTON tbb[] = {
    {0, IDM_FIRST, TBSTATE_ENABLED, TBSTYLE_BUTTON},
    {1, IDM_SECOND, TBSTATE_ENABLED, TBSTYLE_BUTTON},
    {2, IDM_THIRD, TBSTATE_ENABLED, TBSTYLE_BUTTON},
};

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
    INITCOMMONCONTROLSEX icex;
    static HWND hToolbar;
    LONG lStyle;
    MENUITEMINFO mii;
    static HMENU hMenu;
    
    switch (uMsg)
    {
    case WM_CREATE:
        hMenu = GetMenu(hwnd);
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = ICC_BAR_CLASSES;
        InitCommonControlsEx(&icex);
        hToolbar = MyCreateToolbar(hwnd);
        break;
        
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_FIRST:
            MessageBox(hwnd, TEXT("First"), TEXT("Toolbar"), MB_OK);
            break;
        case IDM_SECOND:
            MessageBox(hwnd, TEXT("Second"), TEXT("Toolbar"), MB_OK);
            break;
        case IDM_THIRD:
            MessageBox(hwnd, TEXT("Third"), TEXT("Toolbar"), MB_OK);
            break;
        
        case IDM_OLD:
            lStyle = GetWindowLongPtr(hToolbar, GWL_STYLE);
            lStyle = lStyle & ~TBSTYLE_FLAT & ~TBSTYLE_TRANSPARENT;
            SetWindowLongPtr(hToolbar, GWL_STYLE, lStyle);
            InvalidateRect(hToolbar, NULL, TRUE);
            type = 0;
            break;
            
        case IDM_FLAT:
            lStyle = GetWindowLongPtr(hToolbar, GWL_STYLE);
            lStyle = lStyle | TBSTYLE_FLAT;
            lStyle = lStyle & ~TBSTYLE_TRANSPARENT;
            SetWindowLongPtr(hToolbar, GWL_STYLE, lStyle);
            InvalidateRect(hToolbar, NULL, TRUE);
            type = 1;
            break;
            
        case IDM_TRANSPARENT:
            lStyle = GetWindowLongPtr(hToolbar, GWL_STYLE);
            lStyle = lStyle | TBSTYLE_TRANSPARENT | TBSTYLE_FLAT;
            SetWindowLongPtr(hToolbar, GWL_STYLE, lStyle);
            InvalidateRect(hToolbar, NULL, TRUE);
            type = 2;
            break;
        }
        break;
    
    case WM_SIZE:
        SendMessage(hToolbar, TB_AUTOSIZE, 0, 0);
        break;
        
    case WM_INITMENU:
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_STATE;
        
        switch (type)
        {
        case 0:
            mii.fState = MFS_CHECKED;
            SetMenuItemInfo(hMenu, IDM_OLD, FALSE, &mii);
            mii.fState = MFS_UNCHECKED;
            SetMenuItemInfo(hMenu, IDM_FLAT, FALSE, &mii);
            SetMenuItemInfo(hMenu, IDM_TRANSPARENT, FALSE, &mii);
            break;
        case 1:
            mii.fState = MFS_UNCHECKED;
            SetMenuItemInfo(hMenu, IDM_OLD, FALSE, &mii);
            mii.fState = MFS_CHECKED;
            SetMenuItemInfo(hMenu, IDM_FLAT, FALSE, &mii);
            SetMenuItemInfo(hMenu, IDM_TRANSPARENT, FALSE, &mii);
            break;
        case 2:
            mii.fState = MFS_UNCHECKED;
            SetMenuItemInfo(hMenu, IDM_OLD, FALSE, &mii);
            SetMenuItemInfo(hMenu, IDM_FLAT, FALSE, &mii);
            mii.fState = MFS_CHECKED;
            SetMenuItemInfo(hMenu, IDM_TRANSPARENT, FALSE, &mii);
            break;
        }
        
        DrawMenuBar(hwnd);
        break;
        
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
        
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}

HWND MyCreateToolbar(HWND hwnd)
{
    HWND hwndTB;
    
    hwndTB = CreateToolbarEx(
        hwnd,
        WS_CHILD | WS_VISIBLE,
        ID_TOOLBAR,
        3,
        hInst,
        IDR_TOOLBAR,
        tbb,
        3,
        0,
        0,
        0,
        0,
        sizeof(TBBUTTON)
    );
    
    return hwndTB;
}