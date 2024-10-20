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

class JavaWindow : public JavaComponent
{
    
    HRESULT CreateGraphicsResources() override;
    void    DiscardGraphicsResources() override;
    void    OnPaint() override;
    void    Resize() override;
    
public:
    
    PCWSTR ClassName() const override
    {
        return L"Window";
    }

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};


#endif //WIN32_TEST_JAVA_WINDOW_H
