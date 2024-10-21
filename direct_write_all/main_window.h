#ifndef WIN32_TEST_MAIN_WINDOW_H
#define WIN32_TEST_MAIN_WINDOW_H

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")

#include "base_window.h"

class MainWindow : public BaseWindow<MainWindow>
{
public:
    
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();
    
    MainWindow() :
        pFactory(NULL),
        pRenderTarget(NULL),
        pBrush(NULL)
    {
    
    }
    
    PCWSTR ClassName() const override
    {
        return L"Main Window";
    }
    
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    
private:
    ID2D1Factory            *pFactory;
    ID2D1HwndRenderTarget   *pRenderTarget;
    ID2D1SolidColorBrush    *pBrush;
};


#endif //WIN32_TEST_MAIN_WINDOW_H
