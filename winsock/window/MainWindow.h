#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <windows.h>
#include <d2d1.h>

#include "BaseWindow.h"

class MainWindow : public BaseWindow
{
public:
    explicit MainWindow(bool isServer)
        : pFactory(nullptr)
        , pRenderTarget(nullptr)
        , pBrush(nullptr)
        , myEllipse()
        , otherEllipse()
        , myPoint()
        , otherPoint()
        , isServer(isServer)
    {

    }

    void SetUp();

    LPCSTR ClassName() const override
    {
        return "Main Window";
    }

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    ID2D1Factory *pFactory;
    ID2D1HwndRenderTarget *pRenderTarget;
    ID2D1SolidColorBrush *pBrush;
    D2D1_ELLIPSE myEllipse;
    D2D1_ELLIPSE otherEllipse;
    D2D1_POINT_2F myPoint;
    D2D1_POINT_2F otherPoint;
    bool isServer;

    HRESULT CreateGraphicsResources();
    void DiscardGraphicsResources();
    void OnPaint();
    void OnResize() const;
    void OnKeyboardLeft();
    void OnKeyboardRight();
    void OnKeyboardUp();
    void OnKeyboardDown();
};



#endif //MAINWINDOW_H
