#ifndef WIN32_TEST_JAVA_WINDOW_H
#define WIN32_TEST_JAVA_WINDOW_H

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")

#include "java_component.h"
#include "java_window4_java_Window.h"

class JavaWindow : public JavaComponent<JavaWindow>
{
    ID2D1HwndRenderTarget  *pRenderTarget;
    
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();
    
public:
    
    JavaWindow() :
        pRenderTarget(nullptr)
    {
    
    }
    
    PCWSTR class_name() const
    {
        return L"Window";
    }

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


#endif //WIN32_TEST_JAVA_WINDOW_H
