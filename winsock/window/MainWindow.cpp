#include "MainWindow.h"

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

void MainWindow::SetUp()
{
    RECT rc;
    GetClientRect(m_hwnd, &rc);

    if (isServer)
    {
        myPoint = D2D1::Point2F(rc.left + 100, rc.top + 100);
        otherPoint = D2D1::Point2F(rc.right - 100, rc.bottom - 100);

        myEllipse = D2D1::Ellipse(myPoint, 100, 100);
        otherEllipse = D2D1::Ellipse(otherPoint, 100, 100);
    }
    else
    {
        myPoint = D2D1::Point2F(rc.right - 100, rc.top + 100);
        otherPoint = D2D1::Point2F(rc.left + 100, rc.bottom - 100);

        myEllipse = D2D1::Ellipse(myPoint, 100, 100);
        otherEllipse = D2D1::Ellipse(otherPoint, 100, 100);
    }
}

HRESULT MainWindow::CreateGraphicsResources()
{
    if (pRenderTarget == nullptr)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

        HRESULT hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget
        );
        if (FAILED(hr))
        {
            return hr;
        }

        const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 0, 0);
        hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    return S_OK;
}

void MainWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}

void MainWindow::OnPaint()
{
    HRESULT hr = CreateGraphicsResources();
    if (FAILED(hr))
    {
        return;
    }

    PAINTSTRUCT ps;
    BeginPaint(m_hwnd, &ps);

    pRenderTarget->BeginDraw();
    pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
    pRenderTarget->FillEllipse(myEllipse, pBrush);
    pRenderTarget->FillEllipse(otherEllipse, pBrush);
    hr = pRenderTarget->EndDraw();
    if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
    {
        DiscardGraphicsResources();
    }

    EndPaint(m_hwnd, &ps);
}

void MainWindow::OnResize() const
{
    if (pRenderTarget != nullptr)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

        pRenderTarget->Resize(size);
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void MainWindow::OnKeyboardLeft()
{
    myPoint.x -= 10;
    myEllipse = D2D1::Ellipse(myPoint, 100, 100);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::OnKeyboardRight()
{
    myPoint.x += 10;
    myEllipse = D2D1::Ellipse(myPoint, 100, 100);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::OnKeyboardUp()
{
    myPoint.y -= 10;
    myEllipse = D2D1::Ellipse(myPoint, 100, 100);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::OnKeyboardDown()
{
    myPoint.y += 10;
    myEllipse = D2D1::Ellipse(myPoint, 100, 100);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
            if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
            {
                return -1;
            }
            return 0;

        case WM_DESTROY:
            DiscardGraphicsResources();
            SafeRelease(&pFactory);
            PostQuitMessage(0);
            return 0;

        case WM_PAINT:
            OnPaint();
            return 0;

        case WM_SIZE:
            OnResize();
            return 0;

        case WM_KEYDOWN:
            switch (wParam)
            {
                case VK_LEFT:
                    OnKeyboardLeft();
                    break;

                case VK_RIGHT:
                    OnKeyboardRight();
                    break;

                case VK_UP:
                    OnKeyboardUp();
                    break;

                case VK_DOWN:
                    OnKeyboardDown();
                    break;
            }
    }

    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}