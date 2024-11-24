#pragma once
#include <DirectXMath.h>
#include <Windows.h>

#include "DXDescriptorHeap.h"

class DisplayMatrix
{
    struct SceneMatrix
    {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMFLOAT3 eye;
    };

    DisplayMatrix(RECT wr)
        :
        wr(wr)
    {
    }

public:
    RECT wr;

    DXDescriptorHeap m_cbvHeap;

    HRESULT Init();
};

