#ifndef WIN32_TEST_UTIL_H
#define WIN32_TEST_UTIL_H

#include <string>


template<class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

#endif //WIN32_TEST_UTIL_H
