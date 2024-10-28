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
    
    HRESULT CreateDeviceResources() override;
    void    DiscardDeviceResources() override;
    void    OnPaint() override;
    void    OnResize(UINT width, UINT height) override;
    
public:
    
    HRESULT Initialize() override;
    
    PCWSTR ClassName() const override
    {
        return L"Window";
    }

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    
private:
    
    static std::once_flag flag;
};


#endif //WIN32_TEST_JAVA_WINDOW_H
