#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <d2d1.h>
#include <wincodec.h>
#include "base_window.h"


class MainWindow : public BaseWindow<MainWindow>
{
    ID2D1Factory            *pFactory;
    ID2D1HwndRenderTarget   *pRenderTarget;
    ID2D1SolidColorBrush    *pBrush;
    D2D1_ELLIPSE            ellipse;
    D2D1_POINT_2F           ptMouse;

    IWICBitmapDecoder       *pDecoder;
    IWICImagingFactory      *pWICFactory;
    IWICBitmapFrameDecode   *pSource;
    IWICFormatConverter     *pConverter;
    ID2D1Bitmap             *pBitmap;

    void    CalculateLayout();
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();
    void    ImportImage();

public:

    void    Init();

    MainWindow()
        : pFactory(nullptr)
        , pRenderTarget(nullptr)
        , pBrush(nullptr)
        , ellipse(D2D1::Ellipse(D2D1::Point2F(), 0, 0))
        , ptMouse(D2D1::Point2F())
        , pDecoder(nullptr)
        , pWICFactory(nullptr)
        , pSource(nullptr)
        , pConverter(nullptr)
        , pBitmap(nullptr)
    {
    }

    [[nodiscard]] PCWSTR  ClassName() const override
    {
        return L"Direct2D sample class";
    }

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};


#endif //MAIN_WINDOW_H
