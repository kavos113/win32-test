#ifndef WIN32_TEST_DX_FACTORY_H
#define WIN32_TEST_DX_FACTORY_H

#include <d2d1.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

#include "util.h"

class DXFactory
{
public:
    
    static void ReleaseFactory()
    {
        SafeRelease(&pFactory);
    }
    
    static ID2D1Factory* GetFactory()
    {
        if (pFactory == nullptr)
        {
            if (CreateFactory() < 0)
            {
                return nullptr;
            }
        }
        return pFactory;
    }
    
    static IDWriteFactory* GetDWriteFactory()
    {
        if (pDWriteFactory == nullptr)
        {
            if (CreateDWriteFactory() < 0)
            {
                return nullptr;
            }
        }
        return pDWriteFactory;
    }
    
    static void ReleaseDWriteFactory()
    {
        SafeRelease(&pDWriteFactory);
    }

private:
        
    static ID2D1Factory *pFactory;
    static IDWriteFactory *pDWriteFactory;
    
    static int CreateFactory()
    {
        if (FAILED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            &pFactory)))
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }
    
    static int CreateDWriteFactory()
    {
        if (FAILED(DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&pDWriteFactory))))
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }
};


#endif //WIN32_TEST_DX_FACTORY_H
