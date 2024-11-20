#ifndef WIN32_TEST_BASEWINDOW_H
#define WIN32_TEST_BASEWINDOW_H

#include <windows.h>

template<class WINDOW_TYPE>
class BaseWindow
{
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        WINDOW_TYPE *pThis = NULL;
        
        if (uMsg == WM_CREATE)
        {
            auto* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            pThis = reinterpret_cast<WINDOW_TYPE*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            
            pThis->m_hwnd = hwnd;
        }
        else
        {
            pThis = reinterpret_cast<WINDOW_TYPE*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
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
    
protected:
    
    virtual PCWSTR ClassName() const = 0;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    
    HWND m_hwnd;
    
};


#endif //WIN32_TEST_BASEWINDOW_H
