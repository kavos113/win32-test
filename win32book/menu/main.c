#include <windows.h>

#include "resource.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitHwnd(HINSTANCE hInstance, int nCmdShow);

const TCHAR CLASS_NAME[] = TEXT("Sample Window Class");
HWND hParent;
HINSTANCE hInst;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    BOOL ret;
    HACCEL hAccel;
    
    if (!MyRegisterClass(hInstance))
    {
        return FALSE;
    }
    
    if (!InitHwnd(hInstance, nCmdShow))
    {
        return FALSE;
    }
    
    hAccel = LoadAccelerators(hInstance, TEXT("MYACCELERATORS"));
    
    hInst = hInstance;
    
    while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        // error
        if (ret == -1)
        {
            return -1;
        }
        
        if (!TranslateAccelerator(hParent, hAccel, &msg))
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
    
    hParent = hwnd;
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    return TRUE;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int id;
    HMENU hmenu, hsubmenu;
    POINT pt;
    
    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_ITEM1:
            MessageBox(hwnd, TEXT("Item 1"), TEXT("Menu"), MB_OK);
            break;
        case IDM_ITEM2:
            MessageBox(hwnd, TEXT("Item 2"), TEXT("Menu"), MB_OK);
            break;
        case IDM_ITEM3:
            MessageBox(hwnd, TEXT("Item 3"), TEXT("Menu"), MB_OK);
            break;
        case IDM_OPTION1:
            MessageBox(hwnd, TEXT("Option 1"), TEXT("Options"), MB_OK);
            break;
        case IDM_OPTION2:
            MessageBox(hwnd, TEXT("Option 2"), TEXT("Options"), MB_OK);
            break;
        case IDM_OPTION3:
            MessageBox(hwnd, TEXT("Option 3"), TEXT("Options"), MB_OK);
            break;
        case IDM_CLOSE:
            id = MessageBox(hwnd, TEXT("Really Quit?"), TEXT("Confirm"), MB_YESNO | MB_ICONQUESTION);
            if (id == IDYES)
            {
                DestroyWindow(hwnd);
            }
            break;
        }
        return 0;
        
    case WM_RBUTTONDOWN:
        hmenu = LoadMenu(hInst, TEXT("MYPOPUP"));
        hsubmenu = GetSubMenu(hmenu, 0);
        
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);
        ClientToScreen(hwnd, &pt);
        
        TrackPopupMenu(hsubmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
        
        DestroyMenu(hmenu);
        return 0;
        
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}