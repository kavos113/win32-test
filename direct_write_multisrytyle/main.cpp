#include <windows.h>

#include "hello_text.h"
#include "multi_style_text.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    //HelloText helloText;
    //helloText.Initialize();
    
    MultiStyleText multiStyleText;
    multiStyleText.Initialize();

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}