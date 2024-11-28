#pragma once
#include <DirectXMath.h>
#include <Windows.h>

#include "ConstantBuffer.h"
#include "DXDescriptorHeap.h"

class DisplayMatrix
{
    struct SceneMatrix
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMFLOAT3 eye;
    };

public:
    HRESULT Init();
    void Render();

    DisplayMatrix(RECT wr)
        :
        wr(wr)
    {

    }

private:
    HRESULT SetMatrixBuffer();

    ConstantBuffer<SceneMatrix> m_matrixBuffer;
    DXDescriptorHeap m_cbvHeap;

    RECT wr;

    float angle = 0.0f;
};

