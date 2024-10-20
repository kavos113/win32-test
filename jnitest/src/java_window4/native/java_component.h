#ifndef WIN32_TEST_JAVA_COMPONENT_H
#define WIN32_TEST_JAVA_COMPONENT_H

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <d2d1.h>
#include <iostream>
#pragma comment(lib, "d2d1")

#include "java_window4_java_Component.h"
#include "util.h"

template <class DERIVED_TYPE>
class JavaComponent
{
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        DERIVED_TYPE *pThis = NULL;
        
        if (uMsg == WM_CREATE)
        {
            CREATESTRUCT *pCreate = (CREATESTRUCT *)lParam;
            pThis = (DERIVED_TYPE *)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
            
            pThis->m_hwnd = hwnd;
        }
        else
        {
            pThis = (DERIVED_TYPE *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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
    
    JavaComponent() : m_hwnd(nullptr) {}
    
    ATOM RegisterNewClass()
    {
        WNDCLASS wc = {};
        
        wc.lpfnWndProc = WindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = class_name();
        
        return RegisterClass(&wc);
    }
    
    BOOL Create(
        PCWSTR lpWindowName,
        DWORD dwStyle,
        DWORD dwExStyle = 0,
        int x = CW_USEDEFAULT,
        int y = CW_USEDEFAULT,
        int nWidth = CW_USEDEFAULT,
        int nHeight = CW_USEDEFAULT,
        HWND hWndParent = nullptr,
        HMENU hMenu = nullptr
    )
    {
        m_hwnd = CreateWindowEx(
            dwExStyle,
            class_name(),
            lpWindowName,
            dwStyle,
            x, y,
            nWidth, nHeight,
            hWndParent,
            hMenu,
            GetModuleHandle(nullptr),
            this
        );
        
        return (m_hwnd ? TRUE : FALSE);
    }
    
    HWND Window() const
    {
        return m_hwnd;
    }
    
protected:
    
    virtual PCWSTR class_name() const = 0;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    
    HWND m_hwnd;
};


#endif //WIN32_TEST_JAVA_COMPONENT_H
