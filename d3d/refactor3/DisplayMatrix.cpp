#include "DisplayMatrix.h"

#include "DXCommand.h"

HRESULT DisplayMatrix::Init()
{
    HRESULT hr = SetMatrixBuffer();
    if (FAILED(hr)) return E_FAIL;

    return S_OK;
}

void DisplayMatrix::Render()
{
    angle += 0.05f;
    m_matrixBuffer.GetMappedBuffer()->world = DirectX::XMMatrixRotationY(angle);

    m_cbvHeap.SetToCommand();
    DXCommand::GetCommandList()->SetGraphicsRootDescriptorTable(
        0,
        m_cbvHeap.GetGPUHandle()
    );
}

HRESULT DisplayMatrix::SetMatrixBuffer()
{
    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();

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

    m_matrixBuffer.GetMappedBuffer()->world = worldMatrix;
    m_matrixBuffer.GetMappedBuffer()->view = viewMatrix;
    m_matrixBuffer.GetMappedBuffer()->proj = projectionMatrix;
    m_matrixBuffer.GetMappedBuffer()->eye = eye;

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};

    descriptorHeapDesc.NumDescriptors = 1; // texture(SRV) and constant(CBV) buffer
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descriptorHeapDesc.NodeMask = 0;

    hr = m_cbvHeap.CreateDescriptorHeap(&descriptorHeapDesc);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create texture descriptor heap\n"));
        return hr;
    }

    m_matrixBuffer.SetDescriptorHeap(&m_cbvHeap);
    m_matrixBuffer.CreateView();

    return S_OK;
}
