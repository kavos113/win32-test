#include <windows.h>
#include <windowsx.h>

#include "resource.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitHwnd(HINSTANCE hInstance, int nCmdShow);

const TCHAR CLASS_NAME[] = TEXT("Sample Window Class");
HINSTANCE hInst;
TCHAR szText[6][64];
int gender, country, married, pet, address, pos = 50;

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
    int n;
    HDC hdc;
    PAINTSTRUCT ps;
    
    switch (uMsg)
    {
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        for (n = 0; n < 6; n++)
        {
            TextOut(hdc, 10, 10 + 20 * n, szText[n], lstrlen(szText[n]));
        }
        EndPaint(hwnd, &ps);
        return 0;
        
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case IDM_DIALOG:
            DialogBox(hInst, TEXT("MYDIALOG"), hwnd, DialogProc);
            break;
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

INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hRadio1, hRadio2, hRadio3, hRadio4;
    static HWND hCheck1, hCombo1, hList1, hEdit1, hScroll1;
    TCHAR buf[64];
    TCHAR list[5][16] = {
        TEXT("no pet"),
        TEXT("dog"),
        TEXT("cat"),
        TEXT("bird"),
        TEXT("lion")
    };
    TCHAR combo[6][16] = {
        TEXT("Japan"),
        TEXT("COCOS"),
        TEXT("Mashroom Kingdom"),
        TEXT("Grate Britain"),
        TEXT("Gabon"),
        TEXT("Azerbaijan")
    };
    
    static HWND hMain;
    
    int id;
    int n;
    BOOL success;
    
    switch (uMsg)
    {
    case WM_HSCROLL:
        if (lParam != (LPARAM) hScroll1)
        {
            return FALSE;
        }
        
        switch (LOWORD(wParam))
        {
        case SB_LEFT:
            pos = 0;
            break;
            
        case SB_RIGHT:
            pos = 100;
            break;
            
        case SB_LINELEFT:
            pos--;
            if (pos < 0)
            {
                pos = 0;
            }
            break;
            
        case SB_LINERIGHT:
            pos++;
            if (pos > 100)
            {
                pos = 100;
            }
            break;
            
        case SB_PAGELEFT:
            pos -= 10;
            if (pos < 0)
            {
                pos = 0;
            }
            break;
            
        case SB_PAGERIGHT:
            pos += 10;
            if (pos > 100)
            {
                pos = 100;
            }
            break;
            
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            pos = HIWORD(wParam);
            break;
        }
        
        ScrollBar_SetPos(hScroll1, pos, TRUE);
        
        wsprintf(buf, TEXT("%03d"), pos);
        Edit_SetText(hEdit1, buf);
        wsprintf(szText[5], TEXT("pos = %03d"), pos);
        
        InvalidateRect(hMain, NULL, FALSE);
        return TRUE;
        
    case WM_INITDIALOG:
        hMain = GetParent(hwnd);
        
        hRadio1 = GetDlgItem(hwnd, IDC_RADIO1);
        hRadio2 = GetDlgItem(hwnd, IDC_RADIO2);
        hRadio3 = GetDlgItem(hwnd, IDC_RADIO3);
        hRadio4 = GetDlgItem(hwnd, IDC_RADIO4);
        hCheck1 = GetDlgItem(hwnd, IDC_CHECK1);
        hCombo1 = GetDlgItem(hwnd, IDC_COMBO1);
        hList1 = GetDlgItem(hwnd, IDC_LIST1);
        hEdit1 = GetDlgItem(hwnd, IDC_EDIT1);
        hScroll1 = GetDlgItem(hwnd, IDC_SCROLLBAR1);
        
        ScrollBar_SetRange(hScroll1, 0, 100, TRUE);
        ScrollBar_SetPos(hScroll1, pos, TRUE);
        
        if (gender == 0)
        {
            Button_SetCheck(hRadio1, BST_CHECKED);
        }
        else
        {
            Button_SetCheck(hRadio2, BST_CHECKED);
        }
        
        if (country == 0)
        {
            Button_SetCheck(hRadio3, BST_CHECKED);
        }
        else
        {
            Button_SetCheck(hRadio4, BST_CHECKED);
        }
        
        if (married == 1)
        {
            Button_SetCheck(hCheck1, BST_CHECKED);
        }
        else
        {
            Button_SetCheck(hCheck1, BST_UNCHECKED);
        }
        
        for (n = 0; n < 6; n++)
        {
            ComboBox_AddString(hCombo1, combo[n]);
        }
        
        for (n = 0; n < 5; n++)
        {
            ListBox_AddString(hList1, list[n]);
        }
        
        ComboBox_SetCurSel(hCombo1, address);
        ListBox_SetCurSel(hList1, pet);
        wsprintf(buf, TEXT("%03d"), pos);
        Edit_SetText(hEdit1, buf);
        return TRUE;
        
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
            
        case IDOK:
            if (gender == 0)
            {
                lstrcpy(szText[0], TEXT("selected 'Male'"));
            }
            else
            {
                lstrcpy(szText[0], TEXT("selected 'Female'"));
            }
            
            if (country == 0)
            {
                lstrcpy(szText[1], TEXT("selected 'Japan'"));
            }
            else
            {
                lstrcpy(szText[1], TEXT("selected 'Other'"));
            }
            
            if (married == 1)
            {
                lstrcpy(szText[2], TEXT("selected 'Married'"));
            }
            else
            {
                lstrcpy(szText[2], TEXT("selected 'Single'"));
            }
            
            wsprintf(szText[3], TEXT("selected '%s'"), combo[ComboBox_GetCurSel(hCombo1)]);
            wsprintf(szText[4], TEXT("selected '%s'"), list[ListBox_GetCurSel(hList1)]);
            wsprintf(szText[5], TEXT("pos = %03d"), pos);
            
            InvalidateRect(hMain, NULL, FALSE);
            EndDialog(hwnd, IDOK);
            return TRUE;
            
        case IDC_RADIO1:
            gender = 0;
            lstrcpy(szText[0], TEXT("selected 'Male'"));
            InvalidateRect(hMain, NULL, FALSE);
            return TRUE;
            
        case IDC_RADIO2:
            gender = 1;
            lstrcpy(szText[0], TEXT("selected 'Female'"));
            InvalidateRect(hMain, NULL, FALSE);
            return TRUE;
            
        case IDC_RADIO3:
            country = 0;
            lstrcpy(szText[1], TEXT("selected 'Japan'"));
            InvalidateRect(hMain, NULL, FALSE);
            return TRUE;
            
        case IDC_RADIO4:
            country = 1;
            lstrcpy(szText[1], TEXT("selected 'Other'"));
            InvalidateRect(hMain, NULL, FALSE);
            return TRUE;
            
        case IDC_CHECK1:
            if (Button_GetCheck(hCheck1) == BST_CHECKED)
            {
                married = 1;
                lstrcpy(szText[2], TEXT("selected 'Married'"));
            }
            else
            {
                married = 0;
                lstrcpy(szText[2], TEXT("selected 'Single'"));
            }
            InvalidateRect(hMain, NULL, FALSE);
            return TRUE;
            
        case IDC_COMBO1:
            id = ComboBox_GetCurSel(hCombo1);
            wsprintf(szText[3], TEXT("selected '%s'"), combo[id]);
            InvalidateRect(hMain, NULL, FALSE);
            return TRUE;
            
        case IDC_LIST1:
            id = ListBox_GetCurSel(hList1);
            wsprintf(szText[4], TEXT("selected '%s'"), list[id]);
            InvalidateRect(hMain, NULL, FALSE);
            return TRUE;
            
        case IDC_EDIT1:
            pos = GetDlgItemInt(hwnd, IDC_EDIT1, &success, FALSE);
            if (pos > 100)
            {
                pos = 100;
            }
            if (pos < 0)
            {
                pos = 0;
            }
            
            ScrollBar_SetPos(hScroll1, pos, TRUE);
            wsprintf(szText[5], TEXT("pos = %03d"), pos);
            InvalidateRect(hMain, NULL, FALSE);
            return TRUE;
        }
        
    default:
        return FALSE;
    }
    
    return FALSE;
}