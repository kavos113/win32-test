#include "DisplayMatrix.h"

using DirectX::operator+;
using DirectX::operator-;
using DirectX::operator*;

HRESULT DisplayMatrix::Init(const std::shared_ptr<GlobalDescriptorHeap>& globalHeap)
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

    DirectX::XMVECTOR eyeVec = XMLoadFloat3(&eye);
    DirectX::XMVECTOR targetVec = XMLoadFloat3(&target);
    DirectX::XMVECTOR upVec = XMLoadFloat3(&up);

    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eyeVec, targetVec, upVec);

    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV2,
        static_cast<float>(wr.right - wr.left) / static_cast<float>((wr.bottom - wr.top)),
        1.0f,
        100.0f
    );

    DirectX::XMFLOAT4 plane(0, 1, 0, 0);
    DirectX::XMVECTOR planeVec = XMLoadFloat4(&plane);
    DirectX::XMVECTOR lightVec = XMLoadFloat4(&parallelLightVector);
    DirectX::XMMATRIX shadow = DirectX::XMMatrixShadow(planeVec, lightVec);

    DirectX::XMVECTOR light = targetVec + DirectX::XMVector3Normalize(lightVec) * DirectX::XMVector3Length(DirectX::XMVectorSubtract(targetVec, eyeVec)).m128_f32[0];
    DirectX::XMMATRIX lightCamera = DirectX::XMMatrixLookAtLH(light, targetVec, upVec) * DirectX::XMMatrixOrthographicLH(40, 40, 1.0f, 100.0f);

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

    m_matrixBuffer.GetMappedBuffer()->shadow = shadow;
    m_matrixBuffer.GetMappedBuffer()->lightCamera = lightCamera;

    m_heapId = globalHeap->Allocate(1);

    m_matrixBuffer.SetHeapID(m_heapId);
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
