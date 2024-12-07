#include "DXUtil.h"

#include <tchar.h>
#include <vector>
#include <DirectXTex.h>
#include <map>

#include "Util.h"
#include "resources/DXCommand.h"
#include "resources/DXDevice.h"

#pragma comment(lib, "DirectXTex.lib")

ID3D12Resource* LoadTextureFromFile(
    const std::string& texturePath,
    std::map<std::string, ID3D12Resource*>& _resourceTable
)
{
    if (_resourceTable.contains(texturePath))
    {
        return _resourceTable[texturePath];
    }
    DirectX::TexMetadata metadata = {};
    DirectX::ScratchImage scratchImage = {};

    HRESULT hr = LoadFromWICFile(
        GetWideString(texturePath).c_str(),
        DirectX::WIC_FLAGS_NONE,
        &metadata,
        scratchImage
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to load texture from file: "));
        OutputDebugString(GetWideString(texturePath + "\n").c_str());
        return nullptr;
    }

    auto image = scratchImage.GetImage(0, 0, 0);

    // D3D12_HEAP_PROPERTIES heapProperties = {};
    //
    // heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
    // heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    // heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    // heapProperties.CreationNodeMask = 0;
    // heapProperties.VisibleNodeMask = 0;
    //
    // D3D12_RESOURCE_DESC resourceDesc = {};
    //
    // resourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
    // resourceDesc.Width = metadata.width;
    // resourceDesc.Height = metadata.height;
    // resourceDesc.DepthOrArraySize = metadata.arraySize;
    // resourceDesc.MipLevels = metadata.mipLevels;
    // resourceDesc.Format = metadata.format;
    // resourceDesc.SampleDesc.Count = 1;
    // resourceDesc.SampleDesc.Quality = 0;
    // resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    // resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    //
    // ID3D12Resource* texture = nullptr;
    //
    // hr = _dev->CreateCommittedResource(
    //     &heapProperties,
    //     D3D12_HEAP_FLAG_NONE,
    //     &resourceDesc,
    //     D3D12_RESOURCE_STATE_COPY_DEST,
    //     nullptr,
    //     IID_PPV_ARGS(&texture)
    // );
    // if (FAILED(hr))
    // {
    //     OutputDebugString(_T("Failed to create committed resource\n"));
    //     return nullptr;
    // }
    //
    // hr = texture->WriteToSubresource(
    //     0,
    //     nullptr,
    //     image->pixels,
    //     image->rowPitch,
    //     image->slicePitch
    // );
    // if (FAILED(hr))
    // {
    //     OutputDebugString(_T("Failed to write to subresource\n"));
    //     return nullptr;
    // }


    D3D12_HEAP_PROPERTIES uploadHeapProperties = {};

    uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    uploadHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    uploadHeapProperties.CreationNodeMask = 0;
    uploadHeapProperties.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC uploadResourceDesc = {};

    uploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    uploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    uploadResourceDesc.Width = AlignmentSize(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * image->height;
    uploadResourceDesc.Height = 1;
    uploadResourceDesc.DepthOrArraySize = 1;
    uploadResourceDesc.MipLevels = 1;
    uploadResourceDesc.SampleDesc.Count = 1;
    uploadResourceDesc.SampleDesc.Quality = 0;
    uploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    uploadResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* uploadBuffer = nullptr;

    hr = DXDevice::GetDevice() ->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &uploadResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create upload buffer\n"));
        return nullptr;
    }

    D3D12_HEAP_PROPERTIES textureHeapProps = {};

    textureHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    textureHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    textureHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    textureHeapProps.CreationNodeMask = 0;
    textureHeapProps.VisibleNodeMask = 0;

    uploadResourceDesc.Format = metadata.format;
    uploadResourceDesc.Width = metadata.width;
    uploadResourceDesc.Height = static_cast<UINT>(metadata.height);
    uploadResourceDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize); 
    uploadResourceDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
    uploadResourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
    uploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    ID3D12Resource* textureBuffer = nullptr;

    hr = DXDevice::GetDevice()->CreateCommittedResource(
        &textureHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &uploadResourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&textureBuffer)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create texture copy buffer\n"));
        return nullptr;
    }

    uint8_t* mapForImage = nullptr;
    hr = uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mapForImage));
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to map upload buffer\n"));
        return nullptr;
    }

    auto srcAddr = image->pixels;
    auto rowPitch = AlignmentSize(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

    for (size_t i = 0; i < image->height; ++i)
    {
        std::copy_n(
            srcAddr,
            rowPitch,
            mapForImage
        );

        srcAddr += image->rowPitch;
        mapForImage += rowPitch;
    }

    uploadBuffer->Unmap(0, nullptr);

    D3D12_TEXTURE_COPY_LOCATION src = {};

    src.pResource = uploadBuffer;
    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedFootprint = {};
    UINT nrow;
    UINT64 rowSize, totalSize;

    auto desc = textureBuffer->GetDesc();
    DXDevice::GetDevice()->GetCopyableFootprints(
        &desc,
        0,
        1,
        0,
        &placedFootprint,
        &nrow,
        &rowSize,
        &totalSize
    );

    src.PlacedFootprint = placedFootprint;
    src.PlacedFootprint.Offset = 0;
    src.PlacedFootprint.Footprint.Width = static_cast<UINT>(metadata.width);
    src.PlacedFootprint.Footprint.Height = static_cast<UINT>(metadata.height);
    src.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metadata.depth);
    src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(AlignmentSize(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
    src.PlacedFootprint.Footprint.Format = image->format;

    D3D12_TEXTURE_COPY_LOCATION dst = {};

    dst.pResource = textureBuffer;
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex = 0;

    {
        // process in GPU
        DXCommand::GetCommandList()->CopyTextureRegion(
            &dst,
            0,
            0,
            0,
            &src,
            nullptr
        );

        D3D12_RESOURCE_BARRIER barrierDesc = {};

        barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrierDesc.Transition.pResource = textureBuffer;
        barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

        DXCommand::GetCommandList()->ResourceBarrier(1, &barrierDesc);

        DXCommand::ExecuteCommands();
    }

    _resourceTable[texturePath] = textureBuffer;

    return textureBuffer;
}

ID3D12Resource* CreateWhiteTexture()
{
    D3D12_HEAP_PROPERTIES heap_properties;

    heap_properties.Type = D3D12_HEAP_TYPE_CUSTOM;
    heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    heap_properties.CreationNodeMask = 0;
    heap_properties.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC resourceDesc = {};

    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Width = 4;
    resourceDesc.Height = 4;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* texture = nullptr;

    HRESULT hr = DXDevice::GetDevice()->CreateCommittedResource(
        &heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&texture)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create committed resource\n"));
        return nullptr;
    }

    std::vector<unsigned char> data(4 * 4 * 4);
    std::fill(data.begin(), data.end(), 0xff);

    hr = texture->WriteToSubresource(
        0,
        nullptr,
        data.data(),
        4 * 4,
        static_cast<UINT>(data.size())
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to write to subresource\n"));
        return nullptr;
    }

    return texture;
}

ID3D12Resource* CreateBlackTexture()
{
    D3D12_HEAP_PROPERTIES heap_properties;

    heap_properties.Type = D3D12_HEAP_TYPE_CUSTOM;
    heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    heap_properties.CreationNodeMask = 0;
    heap_properties.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC resourceDesc = {};

    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Width = 4;
    resourceDesc.Height = 4;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* texture = nullptr;

    HRESULT hr = DXDevice::GetDevice()->CreateCommittedResource(
        &heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&texture)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create committed resource\n"));
        return nullptr;
    }

    std::vector<unsigned char> data(4 * 4 * 4);
    std::fill(data.begin(), data.end(), 0x00);

    hr = texture->WriteToSubresource(
        0,
        nullptr,
        data.data(),
        4 * 4,
        static_cast<UINT>(data.size()) 
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to write to subresource\n"));
        return nullptr;
    }

    return texture;
}

ID3D12Resource* CreateGrayGradationTexture()
{
    D3D12_HEAP_PROPERTIES heap_properties;

    heap_properties.Type = D3D12_HEAP_TYPE_CUSTOM;
    heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    heap_properties.CreationNodeMask = 0;
    heap_properties.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC resourceDesc = {};

    resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    resourceDesc.Width = 4;
    resourceDesc.Height = 256;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* texture = nullptr;

    HRESULT hr = DXDevice::GetDevice()->CreateCommittedResource(
        &heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&texture)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create committed resource\n"));
        return nullptr;
    }

    std::vector<unsigned int> data(4 * 256);
    unsigned int c = 0xff;
    for (auto it = data.begin(); it != data.end(); it += 4)
    {
        auto col = (0xff << 24) | RGB(c, c, c);
        std::fill_n(it, 4, col);
        c -= 1;
    }

    hr = texture->WriteToSubresource(
        0,
        nullptr,
        data.data(),
        4 * sizeof(unsigned int),
        static_cast<UINT>(sizeof(unsigned int) * data.size())
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to write to subresource\n"));
        return nullptr;
    }

    return texture;

}