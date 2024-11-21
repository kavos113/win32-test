#include "DXWorld.h"

#include <d3d12.h>

#include "DXCommand.h"
#include "DXDevice.h"

HRESULT DXWorld::SetMatrixBuffer()
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
        aspect_ratio_,
        1.0f,
        100.0f
    );

    D3D12_HEAP_PROPERTIES heap_properties = {};

    heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heap_properties.CreationNodeMask = 0;
    heap_properties.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC resource_desc = {};

    resource_desc.Format = DXGI_FORMAT_UNKNOWN;
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resource_desc.Width = (sizeof(SceneMatrix) + 0xff) & ~0xff;  // ~0xff: 256ƒoƒCƒgˆÈ‰º‚ª0
    resource_desc.Height = 1;
    resource_desc.DepthOrArraySize = 1;
    resource_desc.MipLevels = 1;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.SampleDesc.Quality = 0;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* buffer = nullptr;

    HRESULT hr = DXDevice::GetDevice()->CreateCommittedResource(
        &heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &resource_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&buffer)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create constant buffer\n"));
        return hr;
    }

    hr = buffer->Map(0, nullptr, (void**)&matrix_buffer_map_);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to map constant buffer\n"));
        return hr;
    }

    matrix_buffer_map_->view = viewMatrix;
    matrix_buffer_map_->proj = projectionMatrix;
    matrix_buffer_map_->eye = eye;

    D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {};

    descriptor_heap_desc.NumDescriptors = 1; // texture(SRV) and constant(CBV) buffer
    descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descriptor_heap_desc.NodeMask = 0;

    hr = DXDevice::GetDevice()->CreateDescriptorHeap(
        &descriptor_heap_desc,
        IID_PPV_ARGS(&cbv_heap_)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create texture descriptor heap\n"));
        return hr;
    }

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};

    cbvDesc.BufferLocation = buffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = buffer->GetDesc().Width;

    D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = cbv_heap_->GetCPUDescriptorHandleForHeapStart();

    DXDevice::GetDevice()->CreateConstantBufferView(
        &cbvDesc,
        cbvHandle
    );

    return S_OK;
}

void DXWorld::SetDescriptorHeap() const
{
    DXCommand::GetCommandList()->SetDescriptorHeaps(1, &cbv_heap_);
    DXCommand::GetCommandList()->SetGraphicsRootDescriptorTable(
        0, 
        cbv_heap_->GetGPUDescriptorHandleForHeapStart()
    );
}