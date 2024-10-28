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

class JavaComponent
{
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        JavaComponent *pThis = nullptr;
        
        if (uMsg == WM_CREATE)
        {
            CREATESTRUCT *pCreate = (CREATESTRUCT *)lParam;
            pThis = (JavaComponent *)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
            
            pThis->m_hwnd = hwnd;
        }
        else
        {
            pThis = (JavaComponent *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }
        
        if (pThis)
        {
            pThis->HandleMessage(uMsg, wParam, lParam);
            
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    JavaComponent() :
        m_hwnd(nullptr),
        pRenderTarget(nullptr),
        m_backgroundColor(0)
    {
    
    }
    
    ATOM RegisterNewClass()
    {
        WNDCLASS wc = {};
        
        wc.lpfnWndProc = WindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = ClassName();
        
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
            ClassName(),
            lpWindowName,
            dwStyle | WS_CLIPCHILDREN,
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
    
    void SetBackgroundColor(int color);

protected:
    
    LRESULT ComponentHandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    virtual HRESULT Initialize() = 0;
    
    virtual PCWSTR ClassName() const = 0;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    
    virtual HRESULT CreateDeviceResources() = 0;
    virtual void DiscardDeviceResources() = 0;
    virtual void OnPaint() = 0;
    virtual void OnResize(UINT width, UINT height) = 0;
    
    HWND m_hwnd;
    D2D1::ColorF m_backgroundColor; // int more efficient?
    
    ID2D1HwndRenderTarget  *pRenderTarget;
    
    
};


#endif //WIN32_TEST_JAVA_COMPONENT_H
