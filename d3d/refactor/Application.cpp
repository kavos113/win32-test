#include <windows.h>

#include "Application.h"

#include <tchar.h>

Application::Application()
{
    m_hwnd_ = nullptr;
    RECT wr = { 0, 0, 1280, 720 };
    m_dxProcess_ = std::make_unique<DXEngine>(m_hwnd_, wr);
}

void Application::Init(RECT wr)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        exit(1);
    }

    HWND hwnd = InitWindows(GetModuleHandle(nullptr), SW_SHOW, wr);
    this->m_hwnd_ = hwnd;

    m_dxProcess_->SetHWND(hwnd);
    hr = m_dxProcess_->Init();
    if (FAILED(hr))
    {
        exit(1);
    }
}

HWND Application::InitWindows(HINSTANCE hInstance, int nCmdShow, RECT wr)
{
    WNDCLASSEX wc = {};

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = m_windowClassName_;

    RegisterClassEx(&wc);

    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowEx(
        0,
        _T("WindowClass1"),
        _T("Hello, Direct3D!"),
        WS_OVERLAPPEDWINDOW,
        700,
        700,
        wr.right - wr.left,
        wr.bottom - wr.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    return hwnd;
}

void Application::Run()
{
    ShowWindow(m_hwnd_, SW_SHOW);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        m_dxProcess_->Render();
    }
}

void Application::Cleanup()
{
    UnregisterClass(m_windowClassName_, GetModuleHandle(nullptr));
}