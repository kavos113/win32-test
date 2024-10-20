#ifndef WIN32_TEST_BASE_WINDOW_H
#define WIN32_TEST_BASE_WINDOW_H

#include <windows.h>

class BaseWindow
{
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        BaseWindow *pThis = NULL;
        
        if (uMsg == WM_CREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (BaseWindow *)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
            
            pThis->m_hwnd = hwnd;
        }
        else
        {
            pThis = (BaseWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }
        
        if (pThis)
        {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
    
    BaseWindow() : m_hwnd(NULL) {}
    
    BOOL Create(
        PCWSTR lpWindowName,
        DWORD dwStyle,
        DWORD dwExStyle = 0,
        int x = CW_USEDEFAULT,
        int y = CW_USEDEFAULT,
        int nWidth = CW_USEDEFAULT,
        int nHeight = CW_USEDEFAULT,
        HWND hWndParent = 0,
        HMENU hMenu = 0
    )
    {
        WNDCLASS wc = {0};
        
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = ClassName();
        
        RegisterClass(&wc);
        
        m_hwnd = CreateWindowEx(
            dwExStyle,
            ClassName(),
            lpWindowName,
            dwStyle,
            x, y, nWidth, nHeight,
            hWndParent,
            hMenu,
            GetModuleHandle(NULL),
            this
        );
        
        return (m_hwnd ? TRUE : FALSE);
    }
    
    HWND Window() const
    {
        return m_hwnd;
    }

protected:
    
    virtual PCWSTR ClassName() const = 0;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    
    HWND m_hwnd;
};


#endif //WIN32_TEST_BASE_WINDOW_H
