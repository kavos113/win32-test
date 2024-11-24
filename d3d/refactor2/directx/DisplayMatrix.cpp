#include "DisplayMatrix.h"

HRESULT DisplayMatrix::Init()
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

    D3D12_HEAP_PROPERTIES constantBufferHeapProperties = {};

    constantBufferHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    constantBufferHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    constantBufferHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    constantBufferHeapProperties.CreationNodeMask = 0;
    constantBufferHeapProperties.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC constantBufferResourceDesc = {};

    constantBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    constantBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    constantBufferResourceDesc.Width = (sizeof(SceneMatrix) + 0xff) & ~0xff;  // ~0xff: 256ƒoƒCƒgˆÈ‰º‚ª0
    constantBufferResourceDesc.Height = 1;
    constantBufferResourceDesc.DepthOrArraySize = 1;
    constantBufferResourceDesc.MipLevels = 1;
    constantBufferResourceDesc.SampleDesc.Count = 1;
    constantBufferResourceDesc.SampleDesc.Quality = 0;
    constantBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    constantBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* constantBuffer = nullptr;

    HRESULT hr = DXDevice::GetDevice()->CreateCommittedResource(
        &constantBufferHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &constantBufferResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&constantBuffer)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create constant buffer\n"));
        return 1;
    }

    SceneMatrix* constantBufferMap = nullptr;
    hr = constantBuffer->Map(0, nullptr, (void**)&constantBufferMap);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to map constant buffer\n"));
        return 1;
    }

    constantBufferMap->view = viewMatrix;
    constantBufferMap->proj = projectionMatrix;
    constantBufferMap->eye = eye;

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};

    descriptorHeapDesc.NumDescriptors = 1; // texture(SRV) and constant(CBV) buffer
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descriptorHeapDesc.NodeMask = 0;

    hr = m_cbvHeap.CreateDescriptorHeap(descriptorHeapDesc);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create CBV descriptor heap\n"));
        return 1;
    }

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};

    cbvDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = constantBuffer->GetDesc().Width;

    D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_cbvHeap.GetCPUHandle();

    DXDevice::GetDevice()->CreateConstantBufferView(
        &cbvDesc,
        cbvHandle
    );
}
