#ifndef WIN32_TEST_BASE_WINDOW_H
#define WIN32_TEST_BASE_WINDOW_H

#include <windows.h>

template <class DERIVED_TYPE>
class BaseWindow
{
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        DERIVED_TYPE *pThis = NULL;
        
        if (uMsg == WM_CREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (DERIVED_TYPE*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
            
            pThis->m_hwnd = hwnd;
        }
        else
        {
            pThis = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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
    
    ATOM RegisterNewClassEx()
    {
        WNDCLASSEX wcex = {0};
        
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = DERIVED_TYPE::WindowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(LONG_PTR);
        wcex.hInstance = GetModuleHandle(nullptr);
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName = nullptr;
        wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.lpszClassName = ClassName();
        
        return RegisterClassEx(&wcex);
    }
    
    BOOL Create(
        PCWSTR lpWindowName,
        DWORD dwStyle,
        DWORD dwExStyle = 0,
        int x = CW_USEDEFAULT,
        int y = CW_USEDEFAULT,
        int nWidth = 600,
        int nHeight = 400,
        HWND hWndParent = 0,
        HMENU hMenu = 0
    )
    {
        
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
