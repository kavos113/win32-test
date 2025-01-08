#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "MainWindow.h"

#include <iostream>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <windows.h>

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
        myPoint = D2D1::Point2F(rc.right - 100, rc.bottom - 100);
        otherPoint = D2D1::Point2F(rc.left + 100, rc.top + 100);

        myEllipse = D2D1::Ellipse(myPoint, 100, 100);
        otherEllipse = D2D1::Ellipse(otherPoint, 100, 100);
    }

    CreateSocket();
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

void MainWindow::CreateSocket()
{
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
    {
        std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    int r = inet_pton(AF_INET, ipaddr.c_str(), &serverAddr.sin_addr);
    if (r == 0)
    {
        std::cout << "inet_pton failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(port);
    clientAddr.sin_addr.s_addr = INADDR_ANY;
    r = bind(sock, (sockaddr*)&clientAddr, sizeof(clientAddr));
    if (r == SOCKET_ERROR)
    {
        std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return;
    }
}

HRESULT MainWindow::SendCoordinates()
{
    float my_point[] = {myPoint.x, myPoint.y};
    char buf[sizeof(my_point)];
    memcpy(buf, my_point, sizeof(my_point));

    int r = sendto(sock, buf, sizeof(buf), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (r == SOCKET_ERROR)
    {
        std::cout << "sendto failed with error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return E_FAIL;
    }

    return 0;
}

void MainWindow::Listen()
{
    std::cout << "Listening..." << std::endl;

    while (true)
    {
        char recvbuf[sizeof(float) * 2];
        int r = recv(sock, recvbuf, sizeof(float) * 2, 0);
        if (r == SOCKET_ERROR)
        {
            std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            WSACleanup();
            return;
        }

        float val[2];
        memcpy(val, recvbuf, sizeof(float) * 2);

        otherPoint.x = val[0];
        otherPoint.y = val[1];
        otherEllipse = D2D1::Ellipse(otherPoint, 100, 100);
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
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
                    SendCoordinates();
                    break;

                case VK_RIGHT:
                    OnKeyboardRight();
                    SendCoordinates();
                    break;

                case VK_UP:
                    OnKeyboardUp();
                    SendCoordinates();
                    break;

                case VK_DOWN:
                    OnKeyboardDown();
                    SendCoordinates();
                    break;
            }
    }

    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}
