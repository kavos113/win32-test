#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <windows.h>

class BaseWindow
{
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        BaseWindow* pThis = nullptr;

        if (uMsg == WM_CREATE)
        {
            auto pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            pThis = static_cast<BaseWindow *>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

            pThis->m_hwnd = hwnd;
        }
        else
        {
            pThis = reinterpret_cast<BaseWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (pThis)
        {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    BaseWindow() : m_hwnd(nullptr) {}
    virtual ~BaseWindow() = default;

    HRESULT Create(
        LPCSTR lpWindowName,
        DWORD dwStyle,
        DWORD dwExStyle = 0,
        int x = CW_USEDEFAULT,
        int y = CW_USEDEFAULT,
        int nWidth = CW_USEDEFAULT,
        int nHeight = CW_USEDEFAULT,
        HWND hParentWnd = nullptr,
        HMENU hMenu = nullptr
    )
    {
        WNDCLASS wc = {};

        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = ClassName();

        auto result = RegisterClass(&wc);
        if (result == 0)
        {
            return E_FAIL;
        }

        m_hwnd = CreateWindowEx(
            dwExStyle,
            ClassName(),
            lpWindowName,
            dwStyle,
            x, y, nWidth, nHeight,
            hParentWnd,
            hMenu,
            GetModuleHandle(nullptr),
            this
        );
        if (m_hwnd == nullptr)
        {
            return E_FAIL;
        }

        return S_OK;
    }

    HWND Window() const
    {
        return m_hwnd;
    }

protected:

    virtual LPCSTR ClassName() const = 0;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

    HWND m_hwnd;
};

#endif //BASEWINDOW_H
