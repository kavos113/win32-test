#pragma once
#include <DirectXMath.h>
#include <Windows.h>

#include "buffer/ConstantBuffer.h"
#include "descriptor_heap/DescriptorHeapSegment.h"
#include "descriptor_heap/DescriptorHeapSegmentManager.h"

class DisplayMatrix
{
    struct SceneMatrix
    {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMFLOAT3 eye;
    };

public:
    HRESULT Init(DescriptorHeapSegmentManager& model_heap);
    void Render() const;

    DisplayMatrix(RECT wr)
        :
        m_segment(),
        wr(wr),
        m_modelHeap(nullptr)
    {

    }

private:
    HRESULT SetMatrixBuffer();

    ConstantBuffer<SceneMatrix> m_matrixBuffer;

    DescriptorHeapSegment m_segment;
    RECT wr;

    DescriptorHeapSegmentManager* m_modelHeap;

    float angle = 0.0f;
};

