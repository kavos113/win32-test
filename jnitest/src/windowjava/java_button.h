#ifndef WIN32_TEST_JAVA_BUTTON_H
#define WIN32_TEST_JAVA_BUTTON_H

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>

#include "java_component.h"
#include "windowjava_Button.h"


class JavaButton : public JavaComponent<JavaButton>
{
public:
    PCWSTR ClassName() const
    {
        return L"Button";
    }
    
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


#endif //WIN32_TEST_JAVA_BUTTON_H
