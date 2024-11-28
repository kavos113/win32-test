#pragma once
#include <DirectXMath.h>
#include <memory>
#include <Windows.h>

#include "ConstantBuffer.h"
#include "DXDescriptorHeap.h"
#include "GlobalDescriptorHeap.h"

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
    HRESULT Init(const std::shared_ptr<GlobalDescriptorHeap>& globalHeap);
    void Render();

    DisplayMatrix(RECT wr)
        :
        m_heapId(-1),
        wr(wr)
    {

    }

private:
    HRESULT SetMatrixBuffer();

    ConstantBuffer<SceneMatrix> m_matrixBuffer;
    std::shared_ptr<GlobalDescriptorHeap> globalHeap;
    GLOBAL_HEAP_ID m_heapId;

    RECT wr;

    float angle = 0.0f;
};

