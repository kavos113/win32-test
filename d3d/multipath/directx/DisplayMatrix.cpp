#include "DisplayMatrix.h"

HRESULT DisplayMatrix::Init(const std::shared_ptr<GlobalDescriptorHeap1>& globalHeap)
{
    this->globalHeap = globalHeap;
    m_matrixBuffer.SetGlobalHeap(globalHeap);

    HRESULT hr = SetMatrixBuffer();
    if (FAILED(hr)) return E_FAIL;

    return S_OK;
}

void DisplayMatrix::Render() const
{
    globalHeap->SetGraphicsRootDescriptorTable(m_heapId);
}

HRESULT DisplayMatrix::SetMatrixBuffer()
{
    DirectX::XMFLOAT3 eye(0.0f, 15.0f, -15.0f);
    DirectX::XMFLOAT3 target(0.0f, 10.0f, 0.0f);
    DirectX::XMFLOAT3 up(0.0f, 1.0f, 0.0f);

    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(
        DirectX::XMLoadFloat3(&eye),
        DirectX::XMLoadFloat3(&target),
        DirectX::XMLoadFloat3(&up)
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
        OutputDebugString(_T("Failed to create matrix buffer\n"));
        return hr;
    }

    m_matrixBuffer.GetMappedBuffer()->view = viewMatrix;
    m_matrixBuffer.GetMappedBuffer()->proj = projectionMatrix;
    m_matrixBuffer.GetMappedBuffer()->eye = eye;

    m_heapId = globalHeap->Allocate(1);

    m_matrixBuffer.SetSegment(m_heapId);
    m_matrixBuffer.CreateView();

    D3D12_DESCRIPTOR_RANGE* range = new D3D12_DESCRIPTOR_RANGE();

    range->NumDescriptors = 1;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    range->BaseShaderRegister = 0;
    range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    range->RegisterSpace = 0;

    globalHeap->SetRootParameter(
        m_heapId,
        D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
        D3D12_SHADER_VISIBILITY_ALL,
        range,
        1
    );

    return S_OK;
}
