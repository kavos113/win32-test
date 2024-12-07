#include "DisplayMatrix.h"

HRESULT DisplayMatrix::Init(DescriptorHeapSegmentManager& model_heap)
{
    m_modelHeap = &model_heap;

    HRESULT hr = SetMatrixBuffer();
    if (FAILED(hr)) return E_FAIL;

    return S_OK;
}

void DisplayMatrix::Render() const
{
    m_modelHeap->SetGraphicsRootDescriptorTable(m_segment.GetID());
}

HRESULT DisplayMatrix::SetMatrixBuffer()
{
    DirectX::XMFLOAT3 eye(0.0f, 15.0f, -15.0f);
    DirectX::XMFLOAT3 target(0.0f, 10.0f, 0.0f);
    DirectX::XMFLOAT3 up(0.0f, 1.0f, 0.0f);

    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(
        XMLoadFloat3(&eye),
        XMLoadFloat3(&target),
        XMLoadFloat3(&up)
    );

    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV2,
        static_cast<float>(wr.right - wr.left) / static_cast<float>((wr.bottom - wr.top)),
        1.0f,
        100.0f
    );

    m_matrixBuffer.SetResourceWidth((sizeof(SceneMatrix) + 0xff) & ~0xff);
    HRESULT hr = m_matrixBuffer.CreateBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[DisplayMatrix.cpp] Failed to create matrix buffer\n"));
        return hr;
    }

    m_matrixBuffer.GetMappedBuffer()->view = viewMatrix;
    m_matrixBuffer.GetMappedBuffer()->proj = projectionMatrix;
    m_matrixBuffer.GetMappedBuffer()->eye = eye;

    m_segment = m_modelHeap->Allocate(1);

    m_matrixBuffer.SetSegment(m_segment);
    m_matrixBuffer.CreateView();

    D3D12_DESCRIPTOR_RANGE* range = new D3D12_DESCRIPTOR_RANGE();

    range->NumDescriptors = 1;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    range->BaseShaderRegister = 0;
    range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    range->RegisterSpace = 0;

    m_modelHeap->SetRootParameter(
        m_segment.GetID(),
        D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
        D3D12_SHADER_VISIBILITY_VERTEX,
        range,
        1
    );

    return S_OK;
}
