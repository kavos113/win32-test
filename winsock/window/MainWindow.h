#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <windows.h>
#include <d2d1.h>
#include <string>
#include <winsock2.h>

#include "BaseWindow.h"

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

class MainWindow : public BaseWindow
{
public:
    explicit MainWindow(bool isServer, const std::string &ipaddr)
        : pFactory(nullptr)
        , pRenderTarget(nullptr)
        , pBrush(nullptr)
        , myEllipse()
        , otherEllipse()
        , myPoint()
        , otherPoint()
        , isServer(isServer)
        , ipaddr(ipaddr)
        , sock(INVALID_SOCKET)
        , serverAddr()
    {
    }

    ~MainWindow() override
    {
        DiscardGraphicsResources();
        SafeRelease(&pFactory);

        closesocket(sock);
        WSACleanup();
    }

    void SetUp();
    void Listen();

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

    std::string ipaddr;
    const unsigned short port = 27015;
    SOCKET sock;
    sockaddr_in serverAddr;

    HRESULT CreateGraphicsResources();
    void DiscardGraphicsResources();
    void OnPaint();
    void OnResize() const;
    void OnKeyboardLeft();
    void OnKeyboardRight();
    void OnKeyboardUp();
    void OnKeyboardDown();

    void CreateSocket();
    HRESULT SendCoordinates();
};



#endif //MAINWINDOW_H
