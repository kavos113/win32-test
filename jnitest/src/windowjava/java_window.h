#ifndef WIN32_TEST_JAVA_WINDOW_H
#define WIN32_TEST_JAVA_WINDOW_H

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>

#include "java_component.h"
#include "windowjava_Window.h"

class JavaWindow : public JavaComponent<JavaWindow>
{
public:
    PCWSTR ClassName() const
    {
        return L"JavaWindow";
    }
    
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


#endif //WIN32_TEST_JAVA_WINDOW_H
