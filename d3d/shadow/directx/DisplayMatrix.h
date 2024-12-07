#pragma once
#include <DirectXMath.h>
#include <memory>
#include <Windows.h>

#include "GlobalDescriptorHeap1.h"
#include "buffer/ConstantBuffer.h"

class DisplayMatrix
{
    struct SceneMatrix
    {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMMATRIX shadow;
        DirectX::XMFLOAT3 eye;
    };

public:
    HRESULT Init(const std::shared_ptr<GlobalDescriptorHeap1>& globalHeap);
    void Render() const;

    DisplayMatrix(RECT wr)
        :
        m_heapId(-1),
        wr(wr),
        parallelLightVector(1.0f, -1.0f, 1.0f)
    {

    }

private:
    HRESULT SetMatrixBuffer();

    ConstantBuffer<SceneMatrix> m_matrixBuffer;
    std::shared_ptr<GlobalDescriptorHeap1> globalHeap;
    GLOBAL_HEAP_ID m_heapId;

    RECT wr;

    float angle = 0.0f;

    DirectX::XMFLOAT3 parallelLightVector;
};

