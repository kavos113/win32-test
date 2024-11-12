#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXTex.h>
#include <windows.h>
#include <tchar.h>
#include <debugapi.h>
#include <string>
#include <vector>
#include <stdio.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

D3D_FEATURE_LEVEL featureLevels[] = {
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0
};

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
};

struct TexRGBA
{
	unsigned char r, g, b, a;
};

std::vector<TexRGBA> textureData(256 * 256);

// D3D12 initialization
ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

void InitTextureData()
{
	for (auto& rgba : textureData)
	{
		rgba.r = rand() % 256;
		rgba.g = rand() % 256;
		rgba.b = rand() % 256;
		rgba.a = 255;
	}
}

size_t AlignmentSize(size_t size, size_t alignment)
{
	return size + alignment - (size % alignment);
}

std::string GetTexturePathFromModelAndTexPath(
    const std::string& modelPath,
    const std::string& texPath
)
{
    auto folderPath = modelPath.substr(0, modelPath.find_last_of('/'));
    return folderPath + "/" + texPath;
}

std::wstring GetWideString(const std::string& str)
{
    auto num1 = MultiByteToWideChar(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(),
		-1,
		nullptr,
		0
	);

    std::wstring wstr(num1, 0);
    MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
        str.c_str(),
        -1,
        &wstr[0],
        num1
    );

    assert(wstr.size() == num1);

    return wstr;
}

std::string GetExtention(const std::string& path)
{
    auto pos = path.find_last_of('.');
    if (pos == std::string::npos)
    {
        return "";
    }

    return path.substr(pos + 1);
}

std::pair<std::string, std::string> SplitPath(const std::string& path, const char splitter = '*')
{
	int i = path.find(splitter);
    return std::make_pair(path.substr(0, i), path.substr(i + 1));
}

ID3D12Resource* LoadTextureFromFile(std::string& texturePath)
{
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
        OutputDebugString(_T("Failed to load texture from file\n"));
        return nullptr;
    }

    auto image = scratchImage.GetImage(0, 0, 0);

    D3D12_HEAP_PROPERTIES heapProperties = {};

    heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	heapProperties.CreationNodeMask = 0;
	heapProperties.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC resourceDesc = {};

    resourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension); 
    resourceDesc.Width = metadata.width;
    resourceDesc.Height = metadata.height;
    resourceDesc.DepthOrArraySize = metadata.arraySize;
    resourceDesc.MipLevels = metadata.mipLevels;
    resourceDesc.Format = metadata.format;
    resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* texture = nullptr;

    hr = _dev->CreateCommittedResource(
        &heapProperties,
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

    hr = texture->WriteToSubresource(
        0,
        nullptr,
        image->pixels,
        image->rowPitch,
        image->slicePitch
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to write to subresource\n"));
        return nullptr;
    }

    return texture;
}

ID3D12Resource* CreateWhiteTexture()
{
	D3D12_HEAP_PROPERTIES heapProperties = {};

	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    heapProperties.CreationNodeMask = 0;
    heapProperties.VisibleNodeMask = 0;

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

    HRESULT hr = _dev->CreateCommittedResource(
        &heapProperties,
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
        data.size()
    );

    return texture;
}

HWND InitWindows(HINSTANCE hInstance, int nCmdShow, RECT wr)
{
	WNDCLASSEX wc = {};

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = _T("WindowClass1");

	RegisterClassEx(&wc);

	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hwnd = CreateWindowEx(
		0,
		_T("WindowClass1"),
		_T("Hello, Direct3D!"),
		WS_OVERLAPPEDWINDOW,
		300,
		300,
		wr.right - wr.left,
		wr.bottom - wr.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	return hwnd;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	auto result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	RECT wr = { 0, 0, 800, 600 };

	HWND hwnd = InitWindows(hInstance, nCmdShow, wr);

	InitTextureData();

	ID3D12Debug* debugController = nullptr;
	HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	if (hr == S_OK)
	{
		debugController->EnableDebugLayer();
	}
	debugController->EnableDebugLayer();
	debugController->Release();

	hr = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
	if (FAILED(hr))
	{
		OutputDebugString(_T("Failed to create DxGI Factory"));
		return 1;
	}

	// Selecting a GPU

	std::vector<IDXGIAdapter*> adapters;

	IDXGIAdapter* adapter = nullptr;

	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		adapters.push_back(adapter);
	}

	for (auto adp : adapters)
	{
		DXGI_ADAPTER_DESC desc;
		adp->GetDesc(&desc);

		std::wstring str(desc.Description);

		if (str.find(L"NVIDIA") != std::string::npos)
		{
			adapter = adp;
			break;
		}
	}


	D3D_FEATURE_LEVEL featureLevel;
	for (auto level : featureLevels)
	{
		if (D3D12CreateDevice(nullptr, level, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = level;
			break;
		}
	}
	if (_dev == nullptr)
	{
		OutputDebugString(_T("Failed to create D3D12 device\n"));
		return 1;
	}

	// Command list and command allocator
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* _cmdList = nullptr;

	hr = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	if (FAILED(hr))
	{
		OutputDebugString(_T("Failed to create command allocator\n"));
		return 1;
	}
	hr = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));
	if (FAILED(hr))
	{
		OutputDebugString(_T("Failed to create command list\n"));
		return 1;
	}

	ID3D12CommandQueue* _cmdQueue = nullptr;

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;

	hr = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));
	if (FAILED(hr))
	{
		OutputDebugString(_T("Failed to create command queue\n"));
		return 1;
	}

	// Swap chain
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

	swapchainDesc.Width = wr.right - wr.left;
	swapchainDesc.Height = wr.bottom - wr.top;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = FALSE;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	hr = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapchain
	);
	if (FAILED(hr))
	{
		OutputDebugString(_T("failed to create swap chain"));
		return 1;
	}

	// Descriptor
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};

	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = 2;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	ID3D12DescriptorHeap* _rtvHeap = nullptr;

	hr = _dev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&_rtvHeap));
	if (FAILED(hr))
	{
		OutputDebugString(_T("Failed to create RTV descriptor heap\n"));
		return 1;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	hr = _swapchain->GetDesc(&swapChainDesc);
	if (FAILED(hr))
	{
		OutputDebugString(_T("Failed to get swap chain description\n"));
		return 1;
	}

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	std::vector<ID3D12Resource*> _backBuffers(swapChainDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (int idx = 0; idx < swapChainDesc.BufferCount; ++idx)
	{
		hr = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
		if (FAILED(hr))
		{
			OutputDebugString(_T("Failed to get buffer from swap chain\n"));
			return 1;
		}

		_dev->CreateRenderTargetView(
			_backBuffers[idx],
			&rtvDesc,
			rtvHandle
		);

		rtvHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

    // depth buffer
    D3D12_RESOURCE_DESC depthResourceDesc = {};

    depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthResourceDesc.Width = wr.right - wr.left;
    depthResourceDesc.Height = wr.bottom - wr.top;
    depthResourceDesc.DepthOrArraySize = 1;
    depthResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthResourceDesc.SampleDesc.Count = 1;
    depthResourceDesc.SampleDesc.Quality = 0;
    depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_HEAP_PROPERTIES heapProperties = {};

    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_CLEAR_VALUE clearValue = {};

    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;

    ID3D12Resource* _depthBuffer = nullptr;

    hr = _dev->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &depthResourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(&_depthBuffer)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create depth buffer\n"));
        return 1;
    }

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};

    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;

    ID3D12DescriptorHeap* _dsvHeap = nullptr;

    hr = _dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_dsvHeap));
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create DSV descriptor heap\n"));
        return 1;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};

    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    _dev->CreateDepthStencilView(
        _depthBuffer,
        &dsvDesc,
        _dsvHeap->GetCPUDescriptorHandleForHeapStart()
    );

	// fence
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceValue = 0;

	hr = _dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create fence\n"));
        return 1;
    }


	struct PMDHeader
	{
		float version;
		char model_name[20];
		char comment[256];
	};

#pragma pack(1) // for padding

    struct PMDMaterial
	{
        DirectX::XMFLOAT3 diffuse;
        float alpha;
        float specularity;
        DirectX::XMFLOAT3 specular;
        DirectX::XMFLOAT3 ambient;
        unsigned char toon_index;
        unsigned char edge_flag;
        unsigned int indices_count;
        char texture_file[20];
	};

#pragma pack()

	std::string strModelPath = "Model/�������J.pmd";
	char signatures[3] = {};
    FILE* fp = nullptr;
    errno_t err = fopen_s(&fp, strModelPath.c_str(), "rb");
    if (err != 0)
    {
        OutputDebugString(_T("Failed to open pmd file\n"));
        return 1;
    }

	// read header
    PMDHeader header = {};
	size_t numRead = fread(signatures, sizeof(signatures), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("Failed to read file\n"));
        return 1;
    }
    numRead = fread(&header, sizeof(PMDHeader), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("Failed to read pmd file\n"));
        return 1;
    }

    // read vertices
	constexpr size_t pmd_vertex_size = 38;

    unsigned int numVertices = 0;
	numRead = fread(&numVertices, sizeof(numVertices), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("Failed to read vertices\n"));
        return 1;
    }

    std::vector<unsigned char> pmd_vertices(numVertices * pmd_vertex_size);
    numRead = fread(pmd_vertices.data(), pmd_vertex_size, numVertices, fp);
    if (numRead != numVertices)
    {
        OutputDebugString(_T("Failed to read vertices\n"));
        return 1;
    }

    unsigned int numIndices = 0;
    numRead = fread(&numIndices, sizeof(numIndices), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("Failed to read indices\n"));
        return 1;
    }

    std::vector<unsigned short> pmd_indices(numIndices);
    numRead = fread(pmd_indices.data(), sizeof(pmd_indices[0]), numIndices, fp);
    if (numRead != numIndices)
    {
        OutputDebugString(_T("Failed to read indices\n"));
        return 1;
    }

    unsigned int numMaterials = 0;
    numRead = fread(&numMaterials, sizeof(numMaterials), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("Failed to read materials\n"));
        return 1;
    }

    std::vector<PMDMaterial> pmd_materials(numMaterials);
    numRead = fread(pmd_materials.data(), pmd_materials.size() * sizeof(PMDMaterial), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("Failed to read materials\n"));
        return 1;
    }

	int ret = fclose(fp);
	if (ret != 0)
	{
		OutputDebugString(_T("Failed to close file\n"));
		return 1;
	}

	struct MaterialForHlsl
    {
        DirectX::XMFLOAT3 diffuse;
        float alpha;
        DirectX::XMFLOAT3 specular;
        float specularity;
        DirectX::XMFLOAT3 ambient;
    };

    struct AdditionalMaterial
    {
        std::string texture_file;
		int toon_index;
        bool edge_flag;
    };

	struct Material
	{
        unsigned int indices_count;
        MaterialForHlsl material;
        AdditionalMaterial additional;
	};

    std::vector<Material> materials(pmd_materials.size());

    std::vector<ID3D12Resource*> textureResources(numMaterials);
	std::vector<ID3D12Resource*> sphResources(numMaterials);

    for (size_t i = 0; i < pmd_materials.size(); ++i)
    {
        materials[i].indices_count = pmd_materials[i].indices_count;
        materials[i].material.diffuse = pmd_materials[i].diffuse;
        materials[i].material.alpha = pmd_materials[i].alpha;
        materials[i].material.specular = pmd_materials[i].specular;
		materials[i].material.specularity = pmd_materials[i].specularity;
		materials[i].material.ambient = pmd_materials[i].ambient;
    }

    for (size_t i = 0; i < pmd_materials.size(); ++i)
    {
        if (strlen(pmd_materials[i].texture_file) == 0)
        {
            textureResources[i] = nullptr;
        }

        std::string textureFileName = pmd_materials[i].texture_file;
		std::string sphFileName = "";
        std::string spaFileName = "";

        if (std::count(textureFileName.begin(), textureFileName.end(), '*') > 0)
        {
            auto namepair = SplitPath(textureFileName, '*');
            if (GetExtention(namepair.first) == "sph")
            {
                textureFileName = namepair.second;
                sphFileName = namepair.first;
            }
			else if (GetExtention(namepair.first) == "spa")
            {
                textureFileName = namepair.second;
                spaFileName = namepair.first;
            }
            else
            {
                textureFileName = namepair.first;
                if (GetExtention(namepair.second) == "sph")
                {
                    sphFileName = namepair.second;
                }
				else if (GetExtention(namepair.second) == "spa")
				{
					spaFileName = namepair.second;
				}
            }
        }

        std::string texturePath = GetTexturePathFromModelAndTexPath(
			strModelPath, 
			textureFileName.c_str()
		);

        textureResources[i] = LoadTextureFromFile(texturePath);

        if (sphFileName != "")
        {
            std::string sphPath = GetTexturePathFromModelAndTexPath(
                strModelPath,
                sphFileName.c_str()
            );
            sphResources[i] = LoadTextureFromFile(sphPath);
        }
		else
		{
            sphResources[i] = nullptr;
		}
    }

	size_t materialBufferSize = sizeof(MaterialForHlsl);
	materialBufferSize = (materialBufferSize + 0xff) & ~0xff;

    D3D12_HEAP_PROPERTIES materialHeapProperties = {};

    materialHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    materialHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    materialHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC materialResourceDesc = {};

    materialResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    materialResourceDesc.Width = materialBufferSize * numMaterials;
    materialResourceDesc.Height = 1;
    materialResourceDesc.DepthOrArraySize = 1;
    materialResourceDesc.MipLevels = 1;
    materialResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    materialResourceDesc.SampleDesc.Count = 1;
    materialResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    materialResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* materialBuffer = nullptr;

    hr = _dev->CreateCommittedResource(
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
        return 1;
    }

    unsigned char* materialMap = nullptr;

    hr = materialBuffer->Map(0, nullptr, (void**)&materialMap);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to map material buffer\n"));
        return 1;
    }

	for (auto& m : materials)
	{
        *(MaterialForHlsl*)materialMap = m.material;
        materialMap += materialBufferSize;
	}

	materialBuffer->Unmap(0, nullptr);

	ID3D12DescriptorHeap* materialDescriptorHeap = nullptr;

    D3D12_DESCRIPTOR_HEAP_DESC materialDescriptorHeapDesc = {};

    materialDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	materialDescriptorHeapDesc.NumDescriptors = numMaterials * 3;
    materialDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    materialDescriptorHeapDesc.NodeMask = 0;

    hr = _dev->CreateDescriptorHeap(&materialDescriptorHeapDesc, IID_PPV_ARGS(&materialDescriptorHeap));
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create material descriptor heap\n"));
        return 1;
    }

	D3D12_CONSTANT_BUFFER_VIEW_DESC materialCbvDesc = {};

    materialCbvDesc.BufferLocation = materialBuffer->GetGPUVirtualAddress();
    materialCbvDesc.SizeInBytes = materialBufferSize;

    D3D12_SHADER_RESOURCE_VIEW_DESC materialSrvDesc = {};

    materialSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    materialSrvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    materialSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    materialSrvDesc.Texture2D.MipLevels = 1;

    D3D12_CPU_DESCRIPTOR_HANDLE materialHandle = materialDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    auto incSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    ID3D12Resource* whiteTexture = CreateWhiteTexture();

    for (size_t i = 0; i < numMaterials; ++i)
    {
        _dev->CreateConstantBufferView(&materialCbvDesc, materialHandle);

        materialCbvDesc.BufferLocation += materialBufferSize;
		materialHandle.ptr += incSize;

        if (textureResources[i] == nullptr)
        {
            materialSrvDesc.Format = whiteTexture->GetDesc().Format;
            _dev->CreateShaderResourceView(whiteTexture, &materialSrvDesc, materialHandle);
        }
		else
		{
            materialSrvDesc.Format = textureResources[i]->GetDesc().Format;
            _dev->CreateShaderResourceView(textureResources[i], &materialSrvDesc, materialHandle);
		}

        materialHandle.ptr += incSize;

        if (sphResources[i] == nullptr)
        {
            materialSrvDesc.Format = whiteTexture->GetDesc().Format;
            _dev->CreateShaderResourceView(whiteTexture, &materialSrvDesc, materialHandle);
        }
        else
        {
            materialSrvDesc.Format = sphResources[i]->GetDesc().Format;
            _dev->CreateShaderResourceView(sphResources[i], &materialSrvDesc, materialHandle);
        }

        materialHandle.ptr += incSize;
    }

	// copy vertex data to GPU
	// Vertex vertices[] = {
	// 	{{-1.0f,-1.0f,0.0f},{0.0f,1.0f} },//����
	// 	{{-1.0f,1.0f,0.0f} ,{0.0f,0.0f}},//����
	// 	{{1.0f,-1.0f,0.0f} ,{1.0f,1.0f}},//�E��
	// 	{{1.0f,1.0f,0.0f} ,{1.0f,0.0f}},//�E�� 
	// };
	//
	// unsigned short indices[] = {
	// 	0, 1, 2,
	// 	2, 1, 3
	// };

	D3D12_HEAP_PROPERTIES heap_properties = {};

	heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resource_desc = {};

	resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resource_desc.Width = pmd_vertices.size();
	resource_desc.Height = 1;
	resource_desc.DepthOrArraySize = 1;
	resource_desc.MipLevels = 1;
	resource_desc.Format = DXGI_FORMAT_UNKNOWN;
	resource_desc.SampleDesc.Count = 1;
	resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ID3D12Resource* vertexBuffer = nullptr;

	hr = _dev->CreateCommittedResource(
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

	unsigned char* vertexMap = nullptr;

	hr = vertexBuffer->Map(0, nullptr, (void**)&vertexMap);
	if (FAILED(hr))
	{
		OutputDebugString(_T("Failed to map vertex buffer\n"));
		return 1;
	}

	std::copy(std::begin(pmd_vertices), std::end(pmd_vertices), vertexMap);

	vertexBuffer->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbv = {};

	vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vbv.SizeInBytes = pmd_vertices.size();
	vbv.StrideInBytes = pmd_vertex_size;

	ID3D12Resource* indexBuffer = nullptr;
	
    resource_desc.Width = pmd_indices.size() * sizeof(pmd_indices[0]);
	
	hr = _dev->CreateCommittedResource(
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
		return 1;
	}
	
	unsigned short* indexMap = nullptr;
	indexBuffer->Map(0, nullptr, (void**)&indexMap);
	
	std::copy(std::begin(pmd_indices), std::end(pmd_indices), indexMap);
	
	indexBuffer->Unmap(0, nullptr);
	
	D3D12_INDEX_BUFFER_VIEW ibv = {};
	
	ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    ibv.SizeInBytes = pmd_indices.size() * sizeof(pmd_indices[0]);
	ibv.Format = DXGI_FORMAT_R16_UINT;

	// compile shaders
	ID3D10Blob* vertexShaderBlob = nullptr;
	ID3D10Blob* pixelShaderBlob = nullptr;
	ID3D10Blob* errorBlob = nullptr;

	hr = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS",
		"vs_5_0",
		D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG,
		0,
		&vertexShaderBlob,
		&errorBlob
	);
	if (FAILED(hr))
	{
		OutputDebugString(_T("Failed to compile vertex shader\n"));

		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			OutputDebugString(_T("File not found\n"));
			return 0;
		}

		std::string errStr;
		errStr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errStr.begin());

		OutputDebugStringA(errStr.c_str());
		return 1;
	}

	hr = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS",
		"ps_5_0",
		D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG,
		0,
		&pixelShaderBlob,
		&errorBlob
	);
	if (FAILED(hr))
	{
		OutputDebugString(_T("Failed to compile pixel shader\n"));

		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			OutputDebugString(_T("File not found\n"));
			return 0;
		}

		std::string errStr;
		errStr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errStr.begin());

		OutputDebugStringA(errStr.c_str());
		return 1;
	}

	// pipeline state
	// D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
	// 	{
	// 		"POSITION",
	// 		0,
	// 		DXGI_FORMAT_R32G32B32_FLOAT,
	// 		0,
	// 		D3D12_APPEND_ALIGNED_ELEMENT,
	// 		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
	// 		0
	// 	},
	// 	{
	// 		"TEXCOORD",
	// 		0,
	// 		DXGI_FORMAT_R32G32_FLOAT,
	// 		0,
	// 		D3D12_APPEND_ALIGNED_ELEMENT,
	// 		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
	// 		0
	// 	}
	// };
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "NORMAL",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "BONE_NUMBER",
			0,
            DXGI_FORMAT_R16G16_UINT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "WEIGHT",
			0,
			DXGI_FORMAT_R8_UINT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "EDGE_FLAG",
            0,
            DXGI_FORMAT_R8_UINT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        }
    };

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphics_pipeline = {};

	graphics_pipeline.pRootSignature = nullptr;

	graphics_pipeline.VS.pShaderBytecode = vertexShaderBlob->GetBufferPointer();
	graphics_pipeline.VS.BytecodeLength = vertexShaderBlob->GetBufferSize();

	graphics_pipeline.PS.pShaderBytecode = pixelShaderBlob->GetBufferPointer();
	graphics_pipeline.PS.BytecodeLength = pixelShaderBlob->GetBufferSize();

	graphics_pipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	graphics_pipeline.BlendState.AlphaToCoverageEnable = FALSE;
	graphics_pipeline.BlendState.IndependentBlendEnable = FALSE;

	D3D12_RENDER_TARGET_BLEND_DESC rtvBlendDesc = {};

	rtvBlendDesc.BlendEnable = FALSE;
	rtvBlendDesc.LogicOpEnable = FALSE;
	rtvBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	graphics_pipeline.BlendState.RenderTarget[0] = rtvBlendDesc;

	graphics_pipeline.RasterizerState.MultisampleEnable = FALSE;
	graphics_pipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	graphics_pipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	graphics_pipeline.RasterizerState.DepthClipEnable = TRUE;

	graphics_pipeline.RasterizerState.FrontCounterClockwise = FALSE;
	graphics_pipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	graphics_pipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	graphics_pipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	graphics_pipeline.RasterizerState.AntialiasedLineEnable = FALSE;
	graphics_pipeline.RasterizerState.ForcedSampleCount = 0;
	graphics_pipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	graphics_pipeline.DepthStencilState.DepthEnable = TRUE;
    graphics_pipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    graphics_pipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	graphics_pipeline.DepthStencilState.StencilEnable = FALSE;

    graphics_pipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	graphics_pipeline.InputLayout.pInputElementDescs = inputElementDescs;
	graphics_pipeline.InputLayout.NumElements = _countof(inputElementDescs);

	graphics_pipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	graphics_pipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	graphics_pipeline.NumRenderTargets = 1;
	graphics_pipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	graphics_pipeline.SampleDesc.Count = 1;
	graphics_pipeline.SampleDesc.Quality = 0;

	ID3D12RootSignature* rootSignature = nullptr;
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


	D3D12_DESCRIPTOR_RANGE descRange[3] = {};

    // descriptor range for constant buffer
    descRange[0].NumDescriptors = 1;
    descRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    descRange[0].BaseShaderRegister = 0;
    descRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// descriptor range for material buffer
    descRange[1].NumDescriptors = 1;
    descRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    descRange[1].BaseShaderRegister = 1;
    descRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // descriptor range for texture
    descRange[2].NumDescriptors = 2;
    descRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descRange[2].BaseShaderRegister = 0;
    descRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParam[2] = {};

    // root parameter for constant buffer
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParam[0].DescriptorTable.pDescriptorRanges = &descRange[0];
    rootParam[0].DescriptorTable.NumDescriptorRanges = 1;

    // root parameter for material buffer
    rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParam[1].DescriptorTable.pDescriptorRanges = &descRange[1];
    rootParam[1].DescriptorTable.NumDescriptorRanges = 2;

	rootSignatureDesc.NumParameters = 2;
	rootSignatureDesc.pParameters = rootParam;

	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};

	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pStaticSamplers = &samplerDesc;

	ID3DBlob* signatureBlob = nullptr;

	hr = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob
	);
	if (FAILED(hr))
	{
		OutputDebugString(_T("Failed to serialize root signature\n"));

		std::string errStr;
		errStr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errStr.begin());

		OutputDebugStringA(errStr.c_str());
		return 1;
	}

	hr = _dev->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature)
	);
	if (FAILED(hr))
	{
		OutputDebugString(_T("Failed to create root signature\n"));
		return 1;
	}

	signatureBlob->Release();

	graphics_pipeline.pRootSignature = rootSignature;

	ID3D12PipelineState* _pipelineState = nullptr;

	hr = _dev->CreateGraphicsPipelineState(
		&graphics_pipeline,
		IID_PPV_ARGS(&_pipelineState)
	);
	if (FAILED(hr))
	{
		OutputDebugString(_T("Failed to create pipeline state\n"));
		return 1;
	}

	// viewport
	D3D12_VIEWPORT viewport = {};

	viewport.Width = wr.right - wr.left;
	viewport.Height = wr.bottom - wr.top;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect = {};

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = scissorRect.left + (wr.right - wr.left);
	scissorRect.bottom = scissorRect.top + (wr.bottom - wr.top);

	// set texture
	// DirectX::TexMetadata metadata = {};
	// DirectX::ScratchImage scratch_image = {};
	//
	// hr = LoadFromWICFile(
	// 	L"icon.png",
	// 	DirectX::WIC_FLAGS_NONE,
	// 	&metadata,
	// 	scratch_image
	// );
	// if (FAILED(hr))
	// {
	// 	if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	// 	{
	// 		OutputDebugString(_T("File not found\n"));
	// 		return 0;
	// 	}
	// 	OutputDebugString(_T("Failed to load texture\n"));
	// 	return 1;
	// }
	//
	// const DirectX::Image* image = scratch_image.GetImage(0, 0, 0);
	//
	   // // texture buffer
	//    D3D12_HEAP_PROPERTIES textureHeapProperties = {};
	//
	   // textureHeapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
	//    textureHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	//    textureHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	//    textureHeapProperties.CreationNodeMask = 0;
	//    textureHeapProperties.VisibleNodeMask = 0;
	//
	//    D3D12_RESOURCE_DESC textureResourceDesc = {};
	//
	//    textureResourceDesc.Format = metadata.format;
	   // textureResourceDesc.Width = metadata.width;
	   // textureResourceDesc.Height = metadata.height;
	   // textureResourceDesc.DepthOrArraySize = metadata.arraySize;
	   // textureResourceDesc.SampleDesc.Count = 1;
	   // textureResourceDesc.SampleDesc.Quality = 0;
	   // textureResourceDesc.MipLevels = metadata.mipLevels;
	//    textureResourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	   // textureResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	   // textureResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	//
	//    ID3D12Resource* textureBuffer = nullptr;
	//
	//    hr = _dev->CreateCommittedResource(
	//        &textureHeapProperties,
	//        D3D12_HEAP_FLAG_NONE,
	//        &textureResourceDesc,
	//        D3D12_RESOURCE_STATE_COPY_DEST,
	//        nullptr,
	//        IID_PPV_ARGS(&textureBuffer)
	//    );
	//    if (FAILED(hr))
	//    {
	//        OutputDebugString(_T("Failed to create texture buffer\n"));
	//        return 1;
	//    }

	//    hr = textureBuffer->WriteToSubresource(
	   // 	0,
	   // 	nullptr,
	   // 	textureData.data(), 
	//        sizeof(TexRGBA) * 256,
	//        sizeof(TexRGBA) * textureData.size()
	   // );
	//    hr = textureBuffer->WriteToSubresource(
	   // 	0, 
	   // 	nullptr, 
	   // 	image->pixels, 
	   // 	image->rowPitch,
	   // 	image->slicePitch
	   // );
	//    if (FAILED(hr))
	//    {
	//        OutputDebugString(_T("Failed to write to texture buffer\n"));
	//        return 1;
	//    }
	   // upload texture data
	// D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
	//
	// uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	// uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	// uploadHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	// uploadHeapProperties.CreationNodeMask = 0;
	// uploadHeapProperties.VisibleNodeMask = 0;
	//
	// D3D12_RESOURCE_DESC uploadResourceDesc = {};
	//
	// uploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	// uploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	// uploadResourceDesc.Width = AlignmentSize(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * image->height;
	// uploadResourceDesc.Height = 1;
	// uploadResourceDesc.DepthOrArraySize = 1;
	// uploadResourceDesc.MipLevels = 1;
	// uploadResourceDesc.SampleDesc.Count = 1;
	// uploadResourceDesc.SampleDesc.Quality = 0;
	// uploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// uploadResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	//
	// ID3D12Resource* uploadBuffer = nullptr;
	//
	// hr = _dev->CreateCommittedResource(
	// 	&uploadHeapProperties,
	// 	D3D12_HEAP_FLAG_NONE,
	// 	&uploadResourceDesc,
	// 	D3D12_RESOURCE_STATE_GENERIC_READ,
	// 	nullptr,
	// 	IID_PPV_ARGS(&uploadBuffer)
	// );
	// if (FAILED(hr))
	// {
	// 	OutputDebugString(_T("Failed to create upload buffer\n"));
	// 	return 1;
	// }
	//
	// D3D12_HEAP_PROPERTIES textureHeapProps = {};
	//
	// textureHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	// textureHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	// textureHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	// textureHeapProps.CreationNodeMask = 0;
	// textureHeapProps.VisibleNodeMask = 0;
	//
	// uploadResourceDesc.Format = metadata.format;
	// uploadResourceDesc.Width = metadata.width;
	// uploadResourceDesc.Height = metadata.height;
	// uploadResourceDesc.DepthOrArraySize = metadata.arraySize;
	// uploadResourceDesc.MipLevels = metadata.mipLevels;
	// uploadResourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	// uploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	//
	// ID3D12Resource* textureBuffer = nullptr;
	//
	// hr = _dev->CreateCommittedResource(
	// 	&textureHeapProps,
	// 	D3D12_HEAP_FLAG_NONE,
	// 	&uploadResourceDesc,
	// 	D3D12_RESOURCE_STATE_COPY_DEST,
	// 	nullptr,
	// 	IID_PPV_ARGS(&textureBuffer)
	// );
	// if (FAILED(hr))
	// {
	// 	OutputDebugString(_T("Failed to create texture copy buffer\n"));
	// 	return 1;
	// }
	//
	// uint8_t* mapForImage = nullptr;
	// hr = uploadBuffer->Map(0, nullptr, (void**)&mapForImage);
	// if (FAILED(hr))
	// {
	// 	OutputDebugString(_T("Failed to map upload buffer\n"));
	// 	return 1;
	// }
	//
	// auto srcAddr = image->pixels;
	// auto rowPitch = AlignmentSize(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	//
	// for (size_t i = 0; i < image->height; ++i)
	// {
	// 	std::copy_n(
	// 		srcAddr,
	// 		rowPitch,
	// 		mapForImage
	// 	);
	//
	// 	srcAddr += image->rowPitch;
	// 	mapForImage += rowPitch;
	// }
	//
	// uploadBuffer->Unmap(0, nullptr);
	//
	// D3D12_TEXTURE_COPY_LOCATION src = {};
	//
	// src.pResource = uploadBuffer;
	// src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	//
	// D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedFootprint = {};
	// UINT nrow;
	// UINT64 rowSize, totalSize;
	//
	// auto desc = textureBuffer->GetDesc();
	// _dev->GetCopyableFootprints(
	// 	&desc,
	// 	0,
	// 	1,
	// 	0,
	// 	&placedFootprint,
	// 	&nrow,
	// 	&rowSize,
	// 	&totalSize
	// );
	//
	// src.PlacedFootprint = placedFootprint;
	// src.PlacedFootprint.Offset = 0;
	// src.PlacedFootprint.Footprint.Width = metadata.width;
	// src.PlacedFootprint.Footprint.Height = metadata.height;
	// src.PlacedFootprint.Footprint.Depth = metadata.depth;
	// src.PlacedFootprint.Footprint.RowPitch = AlignmentSize(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	// src.PlacedFootprint.Footprint.Format = image->format;
	//
	// D3D12_TEXTURE_COPY_LOCATION dst = {};
	//
	// dst.pResource = textureBuffer;
	// dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	// dst.SubresourceIndex = 0;
	//
	// {
	// 	// process in GPU
	// 	_cmdList->CopyTextureRegion(
	// 		&dst,
	// 		0,
	// 		0,
	// 		0,
	// 		&src,
	// 		nullptr
	// 	);
	//
	// 	D3D12_RESOURCE_BARRIER barrierDesc = {};
	//
	// 	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// 	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// 	barrierDesc.Transition.pResource = textureBuffer;
	// 	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	// 	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	// 	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	//
	// 	_cmdList->ResourceBarrier(1, &barrierDesc);
	// 	hr = _cmdList->Close();
	// 	if (FAILED(hr))
	// 	{
	// 		OutputDebugString(_T("Failed to close command list\n"));
	// 		return 1;
	// 	}
	//
	// 	ID3D12CommandList* cpyCmdLists[] = { _cmdList };
	// 	_cmdQueue->ExecuteCommandLists(1, cpyCmdLists);
	//
	// 	hr = _cmdQueue->Signal(_fence, ++_fenceValue);
	// 	if (FAILED(hr))
	// 	{
	// 		OutputDebugString(_T("Failed to signal fence\n"));
	// 		return 1;
	// 	}
	//
	// 	if (_fence->GetCompletedValue() != _fenceValue)
	// 	{
	// 		HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	// 		hr = _fence->SetEventOnCompletion(_fenceValue, event);
	// 		if (FAILED(hr))
	// 		{
	// 			OutputDebugString(_T("Failed to set event on completion\n"));
	// 			return 1;
	// 		}
	// 		WaitForSingleObject(event, INFINITE);
	// 		CloseHandle(event);
	// 	}
	//
	// 	hr = _cmdAllocator->Reset();
	// 	if (FAILED(hr))
	// 	{
	// 		OutputDebugString(_T("Failed to reset command allocator\n"));
	// 		return 1;
	// 	}
	// 	hr = _cmdList->Reset(_cmdAllocator, nullptr);
	// 	if (FAILED(hr))
	// 	{
	// 		OutputDebugString(_T("Failed to reset command list\n"));
	// 		return 1;
	// 	}
	// }

    // constant buffer
	struct MatricesData
	{
		DirectX::XMMATRIX world;
        DirectX::XMMATRIX viewproj;
	};

    float angle = 0;
	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();

    DirectX::XMFLOAT3 eye(0.0f, 10.0f, -15.0f);
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
    constantBufferResourceDesc.Width = (sizeof(MatricesData) + 0xff) & ~0xff;  // ~0xff: 256�o�C�g�ȉ���0
    constantBufferResourceDesc.Height = 1;
    constantBufferResourceDesc.DepthOrArraySize = 1;
    constantBufferResourceDesc.MipLevels = 1;
    constantBufferResourceDesc.SampleDesc.Count = 1;
    constantBufferResourceDesc.SampleDesc.Quality = 0;
    constantBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    constantBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* constantBuffer = nullptr;

    hr = _dev->CreateCommittedResource(
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

    MatricesData* constantBufferMap = nullptr;
    hr = constantBuffer->Map(0, nullptr, (void**)&constantBufferMap);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to map constant buffer\n"));
        return 1;
    }

    constantBufferMap->world = worldMatrix;
    constantBufferMap->viewproj = viewMatrix * projectionMatrix;

	ID3D12DescriptorHeap* descriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};

    descriptorHeapDesc.NumDescriptors = 1; // texture(SRV) and constant(CBV) buffer
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDesc.NodeMask = 0;

	hr = _dev->CreateDescriptorHeap(
		&descriptorHeapDesc,
		IID_PPV_ARGS(&descriptorHeap)
	);
	if (FAILED(hr))
	{
		OutputDebugString(_T("Failed to create texture descriptor heap\n"));
		return 1;
	}

	// D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//
	// srvDesc.Format = metadata.format;
	// srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	// srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	// srvDesc.Texture2D.MipLevels = 1;
	//
	// _dev->CreateShaderResourceView(
	// 	textureBuffer,
	// 	&srvDesc,
	// 	descriptorHeap->GetCPUDescriptorHandleForHeapStart()
	// );

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};

    cbvDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = constantBuffer->GetDesc().Width;

    D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    //cbvHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    _dev->CreateConstantBufferView(
        &cbvDesc,
        cbvHandle
    );

	// main loop

	ShowWindow(hwnd, nCmdShow);

	MSG msg;
	unsigned int frameIndex = 0;

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		angle += 0.02f;
	    worldMatrix = DirectX::XMMatrixRotationY(angle);
  //       worldMatrix *= DirectX::XMMatrixRotationX(angle);
  //       worldMatrix *= DirectX::XMMatrixRotationZ(angle);
        constantBufferMap->world = worldMatrix;

		// swap chain
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

		D3D12_RESOURCE_BARRIER barrier = {};

		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = _backBuffers[bbIdx];
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		_cmdList->ResourceBarrier(1, &barrier);

		_cmdList->SetPipelineState(_pipelineState);

		auto rendertvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rendertvHandle.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        auto dsvHandle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
		_cmdList->OMSetRenderTargets(1, &rendertvHandle, TRUE, &dsvHandle);

		float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		_cmdList->ClearRenderTargetView(rendertvHandle, clearColor, 0, nullptr);
        _cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		++frameIndex;

		// draw polygon
		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorRect);
		_cmdList->SetGraphicsRootSignature(rootSignature);

		_cmdList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_cmdList->IASetVertexBuffers(0, 1, &vbv);
		_cmdList->IASetIndexBuffer(&ibv);

		_cmdList->SetGraphicsRootSignature(rootSignature);
		_cmdList->SetDescriptorHeaps(1, &descriptorHeap);
		_cmdList->SetGraphicsRootDescriptorTable(
			0,
			descriptorHeap->GetGPUDescriptorHandleForHeapStart()
		);

        _cmdList->SetDescriptorHeaps(1, &materialDescriptorHeap);
        _cmdList->SetGraphicsRootDescriptorTable(
            1,
            materialDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
        );

        D3D12_GPU_DESCRIPTOR_HANDLE materialDescHandle = materialDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

		unsigned int indexOffset = 0;
        auto matIncSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 3;

        for (auto& m : materials)
        {
            _cmdList->SetGraphicsRootDescriptorTable(
                1,
                materialDescHandle
            );

            _cmdList->DrawIndexedInstanced(m.indices_count, 1, indexOffset, 0, 0);

            indexOffset += m.indices_count;
            materialDescHandle.ptr += matIncSize;
        }

        // auto descriptorHandle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
        // descriptorHandle.ptr
	       //  += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        //
        // _cmdList->SetGraphicsRootDescriptorTable(
        //     1,
        //     descriptorHandle
        // );

		//_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
		_cmdList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

		_cmdList->ResourceBarrier(1, &barrier);

		hr = _cmdList->Close();
		if (FAILED(hr))
		{
			OutputDebugString(_T("Failed to close command list\n"));
			return 1;
		}

		ID3D12CommandList* cmdLists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdLists);

		hr = _cmdQueue->Signal(_fence, ++_fenceValue);
		if (FAILED(hr))
		{
			OutputDebugString(_T("Failed to signal fence\n"));
			return 1;
		}

		if (_fence->GetCompletedValue() != _fenceValue)
		{
			HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			hr = _fence->SetEventOnCompletion(_fenceValue, event);
			if (FAILED(hr))
			{
				OutputDebugString(_T("Failed to set event on completion\n"));
				return 1;
			}
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		hr = _cmdAllocator->Reset();
		if (FAILED(hr))
		{
			OutputDebugString(_T("Failed to reset command allocator\n"));
			return 1;
		}
		hr = _cmdList->Reset(_cmdAllocator, nullptr);
		if (FAILED(hr))
		{
			OutputDebugString(_T("Failed to reset command list\n"));
			return 1;
		}

		hr = _swapchain->Present(1, 0);
		if (FAILED(hr))
		{
			OutputDebugString(_T("Failed to present\n"));
			return 1;
		}
	}

	UnregisterClass(_T("WindowClass1"), hInstance);

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
