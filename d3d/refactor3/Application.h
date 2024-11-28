#pragma once
#include "DXEngine.h"

class Application
{
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    Application();

    void Run();
    void Init(RECT wr);
    void Cleanup();

private:
    HWND InitWindows(HINSTANCE hInstance, int nCmdShow, RECT wr);

    const wchar_t* m_windowClassName_ = L"WindowClass1";
    HWND m_hwnd_;

    std::unique_ptr<DXEngine> m_dxProcess_; 
};

