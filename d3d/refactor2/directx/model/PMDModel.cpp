#include "PMDModel.h"

#include <tchar.h>

#include "Util.h"
#include "directx/resources/DXDevice.h"

void PMDModel::Read()
{
    FILE* fp;
    fopen_s(&fp, path_.c_str(), "rb");
    if (fp == nullptr)
    {
        OutputDebugString(_T("Failed to open file\n"));
        return;
    }

    HRESULT hr = ReadHeader(fp);
    if (FAILED(hr)) return;

    hr = ReadVertices(fp);
    if (FAILED(hr)) return;

    hr = ReadIndices(fp);
    if (FAILED(hr)) return;

    hr = ReadMaterials(fp);
    if (FAILED(hr)) return;

    fclose(fp);

    hr = SetMaterialBuffer();
    if (FAILED(hr)) return;

    hr = SetVertexBuffer();
    if (FAILED(hr)) return;

    hr = SetIndexBuffer();
    if (FAILED(hr)) return;

    hr = SetMatrixBuffer();
    if (FAILED(hr)) return;
}

void PMDModel::Render()
{

}

HRESULT PMDModel::ReadHeader(FILE* fp)
{
    PMDHeader header = {};
    char signatures[3] = {};
    size_t numRead = fread(signatures, sizeof(signatures), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("Failed to read file\n"));
        return E_FAIL;
    }
    numRead = fread(&header, sizeof(header), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("Failed to read pmd file\n"));
        return E_FAIL;
    }

    return S_OK;
}

HRESULT PMDModel::ReadVertices(FILE* fp)
{
    unsigned int num_vertices = 0;
    size_t numRead = fread(&num_vertices, sizeof(num_vertices), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("Failed to read vertices\n"));
        return E_FAIL;
    }

    vertices_.resize(num_vertices);
    for (auto i = 0; i < num_vertices; ++i)
    {
        numRead = fread(&vertices_[i], pmd_vertex_size_, 1, fp);
        if (numRead != 1)
        {
            OutputDebugString(_T("Failed to read vertices\n"));
            return E_FAIL;
        }
    }

    return S_OK;
}

HRESULT PMDModel::ReadIndices(FILE* fp)
{
    unsigned int num_indices = 0;
    size_t numRead = fread(&num_indices, sizeof(num_indices), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("Failed to read indices\n"));
        return E_FAIL;
    }

    indices_.resize(num_indices);
    numRead = fread(indices_.data(), sizeof(unsigned short), num_indices, fp);
    if (numRead != num_indices)
    {
        OutputDebugString(_T("Failed to read indices\n"));
        return E_FAIL;
    }

    return S_OK;
}

HRESULT PMDModel::ReadMaterials(FILE* fp)
{
    unsigned int num_materials = 0;
    size_t numRead = fread(&num_materials, sizeof(num_materials), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("Failed to read materials\n"));
        return E_FAIL;
    }

    pmd_materials_.resize(num_materials);
    numRead = fread(pmd_materials_.data(), pmd_materials_.size() * sizeof(PMDMaterial), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("Failed to read materials\n"));
        return E_FAIL;
    }

    int ret = fclose(fp);
    if (ret != 0)
    {
        OutputDebugString(_T("Failed to close file\n"));
        return E_FAIL;
    }

    materials_.resize(num_materials);
    for (size_t i = 0; i < pmd_materials_.size(); ++i)
    {
        materials_[i].indices_count = pmd_materials_[i].indices_count;
        materials_[i].material.diffuse = pmd_materials_[i].diffuse;
        materials_[i].material.alpha = pmd_materials_[i].alpha;
        materials_[i].material.specular = pmd_materials_[i].specular;
        materials_[i].material.specularity = pmd_materials_[i].specularity;
        materials_[i].material.ambient = pmd_materials_[i].ambient;
    }

    textures_.resize(num_materials);
    sph_.resize(num_materials);
    spa_.resize(num_materials);
    toon_.resize(num_materials);

    for (size_t i = 0; i < pmd_materials_.size(); ++i)
    {
        std::string toonFilePath = "toon/";
        char toonFileName[16] = {};

        sprintf_s(toonFileName, "toon%02d.bmp", pmd_materials_[i].toon_index + 1);

        toonFilePath += toonFileName;

        toon_[i] = LoadTextureFromFile(toonFilePath, resourceTable);

        OutputDebugString(GetWideString(pmd_materials_[i].texture_file).c_str());
        OutputDebugString(_T("-\n"));
        if (strlen(pmd_materials_[i].texture_file) == 0)
        {
            textures_[i] = nullptr;
        }

        std::string textureFileName = pmd_materials_[i].texture_file;
        std::string sphFileName = "";
        std::string spaFileName = "";

        if (std::count(textureFileName.begin(), textureFileName.end(), '*') > 0)
        {
            auto namepair = SplitPath(textureFileName, '*');
            if (GetExtension(namepair.first) == "sph")
            {
                textureFileName = namepair.second;
                sphFileName = namepair.first;
            }
            else if (GetExtension(namepair.first) == "spa")
            {
                textureFileName = namepair.second;
                spaFileName = namepair.first;
            }
            else
            {
                textureFileName = namepair.first;
                if (GetExtension(namepair.second) == "sph")
                {
                    sphFileName = namepair.second;
                }
                else if (GetExtension(namepair.second) == "spa")
                {
                    spaFileName = namepair.second;
                }
            }
        }
        else
        {
            if (GetExtension(textureFileName) == "sph")
            {
                sphFileName = pmd_materials_[i].texture_file;
                textureFileName = "";
            }
            else if (GetExtension(textureFileName) == "spa")
            {
                spaFileName = pmd_materials_[i].texture_file;
                textureFileName = "";
            }
            else
            {
                sphFileName = "";
                spaFileName = "";
                textureFileName = pmd_materials_[i].texture_file;
            }
        }

        if (textureFileName != "")
        {
            std::string texturePath = GetTexturePathFromModelAndTexPath(
                path_,
                textureFileName.c_str()
            );
            textures_[i] = LoadTextureFromFile(texturePath, resourceTable);
        }
        else
        {
            textures_[i] = nullptr;
        }

        if (sphFileName != "")
        {
            OutputDebugString(_T("sph file found"));
            OutputDebugString(GetWideString(sphFileName + "\n").c_str());
            std::string sphPath = GetTexturePathFromModelAndTexPath(
                path_,
                sphFileName.c_str()
            );
            sph_[i] = LoadTextureFromFile(sphPath, resourceTable);
        }
        else
        {
            sph_[i] = nullptr;
        }

        OutputDebugString(GetWideString(spaFileName).c_str());

        if (spaFileName != "")
        {
            OutputDebugString(_T("spa file found"));
            OutputDebugString(GetWideString(spaFileName + "\n").c_str());
            std::string spaPath = GetTexturePathFromModelAndTexPath(
                path_,
                spaFileName.c_str()
            );
            spa_[i] = LoadTextureFromFile(spaPath, resourceTable);
        }
        else
        {
            spa_[i] = nullptr;
        }
    }

    return S_OK;
}

HRESULT PMDModel::SetMaterialBuffer()
{
    size_t materialBufferSize = sizeof(MaterialForHlsl);
    materialBufferSize = (materialBufferSize + 0xff) & ~0xff;

    D3D12_HEAP_PROPERTIES materialHeapProperties = {};

    materialHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    materialHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    materialHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC materialResourceDesc = {};

    materialResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    materialResourceDesc.Width = materialBufferSize * materials_.size();
    materialResourceDesc.Height = 1;
    materialResourceDesc.DepthOrArraySize = 1;
    materialResourceDesc.MipLevels = 1;
    materialResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    materialResourceDesc.SampleDesc.Count = 1;
    materialResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    materialResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* materialBuffer = nullptr;

    HRESULT hr = DXDevice::GetDevice()->CreateCommittedResource(
        &materialHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &materialResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&materialBuffer)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create committed resource\n"));
        return hr;
    }

    unsigned char* materialMap = nullptr;

    hr = materialBuffer->Map(0, nullptr, (void**)&materialMap);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to map material buffer\n"));
        return hr;
    }

    for (auto& m : materials_)
    {
        *(MaterialForHlsl*)materialMap = m.material;
        materialMap += materialBufferSize;
    }

    materialBuffer->Unmap(0, nullptr);

    D3D12_DESCRIPTOR_HEAP_DESC materialDescriptorHeapDesc = {};

    materialDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    materialDescriptorHeapDesc.NumDescriptors = materials_.size() * 5;
    materialDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    materialDescriptorHeapDesc.NodeMask = 0;

    hr = material_heap_.CreateDescriptorHeap(materialDescriptorHeapDesc);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create descriptor heap\n"));
        return hr;
    }

    D3D12_CONSTANT_BUFFER_VIEW_DESC materialCbvDesc = {};

    materialCbvDesc.BufferLocation = materialBuffer->GetGPUVirtualAddress();
    materialCbvDesc.SizeInBytes = materialBufferSize;

    D3D12_SHADER_RESOURCE_VIEW_DESC materialSrvDesc = {};

    materialSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    materialSrvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    materialSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    materialSrvDesc.Texture2D.MipLevels = 1;

    D3D12_CPU_DESCRIPTOR_HANDLE materialHandle = material_heap_.GetCPUHandle();
    UINT incSize = material_heap_.GetIncrementSize();

    ID3D12Resource* whiteTexture = CreateWhiteTexture();
    ID3D12Resource* blackTexture = CreateBlackTexture();
    ID3D12Resource* grayGradationTexture = CreateGrayGradationTexture();

    for (size_t i = 0; i < materials_.size(); ++i)
    {
        DXDevice::GetDevice()->CreateConstantBufferView(&materialCbvDesc, materialHandle);

        materialCbvDesc.BufferLocation += materialBufferSize;
        materialHandle.ptr += incSize;

        if (textures_[i] == nullptr)
        {
            materialSrvDesc.Format = whiteTexture->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(whiteTexture, &materialSrvDesc, materialHandle);
        }
        else
        {
            materialSrvDesc.Format = textures_[i]->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(textures_[i], &materialSrvDesc, materialHandle);
        }

        materialHandle.ptr += incSize;

        if (sph_[i] == nullptr)
        {
            materialSrvDesc.Format = whiteTexture->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(whiteTexture, &materialSrvDesc, materialHandle);
        }
        else
        {
            materialSrvDesc.Format = sph_[i]->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(sph_[i], &materialSrvDesc, materialHandle);
        }

        materialHandle.ptr += incSize;

        if (spa_[i] == nullptr)
        {
            materialSrvDesc.Format = blackTexture->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(blackTexture, &materialSrvDesc, materialHandle);
        }
        else
        {
            materialSrvDesc.Format = spa_[i]->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(spa_[i], &materialSrvDesc, materialHandle);
        }

        materialHandle.ptr += incSize;

        if (toon_[i] == nullptr)
        {
            materialSrvDesc.Format = whiteTexture->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(grayGradationTexture, &materialSrvDesc, materialHandle);
        }
        else
        {
            materialSrvDesc.Format = toon_[i]->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(toon_[i], &materialSrvDesc, materialHandle);
        }

        materialHandle.ptr += incSize;
    }

    return S_OK;
}

HRESULT PMDModel::SetVertexBuffer()
{
    D3D12_HEAP_PROPERTIES heap_properties = {};

    heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resource_desc = {};

    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resource_desc.Width = vertices_.size() * sizeof(PMDVertex);
    resource_desc.Height = 1;
    resource_desc.DepthOrArraySize = 1;
    resource_desc.MipLevels = 1;
    resource_desc.Format = DXGI_FORMAT_UNKNOWN;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    resource_desc.Alignment = 0;

    ID3D12Resource* vertexBuffer = nullptr;

    HRESULT hr = DXDevice::GetDevice()->CreateCommittedResource(
        &heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &resource_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create committed resource\n"));
        return 1;
    }

    PMDVertex* vertexMap = nullptr;

    hr = vertexBuffer->Map(0, nullptr, (void**)&vertexMap);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to map vertex buffer\n"));
        return hr;
    }

    std::copy(vertices_.begin(), vertices_.end(), vertexMap);

    vertexBuffer->Unmap(0, nullptr);

    vertex_buffer_view_.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertex_buffer_view_.SizeInBytes = static_cast<UINT>(vertices_.size() * sizeof(PMDVertex));
    vertex_buffer_view_.StrideInBytes = sizeof(PMDVertex);

    return S_OK;
}

HRESULT PMDModel::SetIndexBuffer()
{
    D3D12_HEAP_PROPERTIES heap_properties = {};

    heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resource_desc = {};

    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resource_desc.Width = indices_.size() * sizeof(indices_[0]);
    resource_desc.Height = 1;
    resource_desc.DepthOrArraySize = 1;
    resource_desc.MipLevels = 1;
    resource_desc.Format = DXGI_FORMAT_UNKNOWN;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* indexBuffer = nullptr;

    HRESULT hr = DXDevice::GetDevice()->CreateCommittedResource(
        &heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &resource_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&indexBuffer)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create committed resource\n"));
        return hr;
    }

    unsigned short* indexMap = nullptr;
    indexBuffer->Map(0, nullptr, (void**)&indexMap);

    std::copy(indices_.begin(), indices_.end(), indexMap);

    indexBuffer->Unmap(0, nullptr);

    index_buffer_view_.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    index_buffer_view_.SizeInBytes = static_cast<UINT>(indices_.size() * sizeof(indices_[0]));
    index_buffer_view_.Format = DXGI_FORMAT_R16_UINT;

    return S_OK;
}

HRESULT PMDModel::SetMatrixBuffer()
{
    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();

    D3D12_HEAP_PROPERTIES heap_properties = {};

    heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heap_properties.CreationNodeMask = 0;
    heap_properties.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC resource_desc = {};

    resource_desc.Format = DXGI_FORMAT_UNKNOWN;
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resource_desc.Width = (sizeof(ModelMatrix) + 0xff) & ~0xff;
    resource_desc.Height = 1;
    resource_desc.DepthOrArraySize = 1;
    resource_desc.MipLevels = 1;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.SampleDesc.Quality = 0;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* matrix_buffer = nullptr;

    HRESULT hr = DXDevice::GetDevice()->CreateCommittedResource(
        &heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &resource_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&matrix_buffer)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create constant buffer\n"));
        return hr;
    }

    hr = matrix_buffer->Map(0, nullptr, (void**)&matrix_buffer_map_);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to map constant buffer\n"));
        return hr;
    }

    matrix_buffer_map_->world = worldMatrix;

    D3D12_DESCRIPTOR_HEAP_DESC matrix_descriptor_heap_desc = {};

    matrix_descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    matrix_descriptor_heap_desc.NumDescriptors = 1;
    matrix_descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    matrix_descriptor_heap_desc.NodeMask = 0;

    hr = matrix_heap_.CreateDescriptorHeap(matrix_descriptor_heap_desc);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create descriptor heap\n"));
        return hr;
    }

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};

    cbv_desc.BufferLocation = matrix_buffer->GetGPUVirtualAddress();
    cbv_desc.SizeInBytes = matrix_buffer->GetDesc().Width;

    DXDevice::GetDevice()->CreateConstantBufferView(
        &cbv_desc,
        matrix_heap_.GetCPUHandle()
    );

    return S_OK;
}