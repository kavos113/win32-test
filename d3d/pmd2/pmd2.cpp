#include <windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <vector>

#include <d3dcompiler.h>
#include <DirectXTex.h>
#include <string>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")

#define CheckHR(hr, str) if (FAILED(hr)) { OutputDebugString(str); return -1; }

#pragma pack(push, 1)
struct PMD_VERTEX
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 uv;
    uint16_t boneNo[2];
    uint8_t boneWeight;
    uint8_t edgeFlag;
	uint16_t dummy;
};
#pragma pack(pop)

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

const unsigned int WIDTH = 1280;
const unsigned int HEIGHT = 720;

IDXGIFactory4* _dxgiFactory;
ID3D12Device* _device;
ID3D12CommandAllocator* _commandAllocator;
ID3D12GraphicsCommandList* _commandList;
ID3D12CommandQueue* _commandQueue;
IDXGISwapChain4* _swapChain;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        return -1;
    }

    WNDCLASSEX wc = {};

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = _T("PMD2");

    RegisterClassEx(&wc);

    RECT rect = { 0, 0, WIDTH, HEIGHT };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

    HWND hwnd = CreateWindow(
        wc.lpszClassName, 
        _T("PMD2"),
        WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT,
        CW_USEDEFAULT, 
        rect.right - rect.left,
        rect.bottom - rect.top, 
        nullptr, 
        nullptr,
        hInstance, 
        nullptr
    );
    if(!hwnd)
    {
        return -1;
    }

    // DirectX12の初期化
    ID3D12Debug* debugLayer = nullptr;
    hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
    if (SUCCEEDED(hr))
    {
        debugLayer->EnableDebugLayer();
    }
    debugLayer->Release();

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };
    hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
    CheckHR(hr, _T("CreateDXGIFactory2 failed\n"));

    std::vector<IDXGIAdapter*> adapters;
    IDXGIAdapter* adapter = nullptr;
    for (int i = 0; _dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        adapters.push_back(adapter);
    }
    for (auto adpt : adapters)
    {
        DXGI_ADAPTER_DESC desc;
        adpt->GetDesc(&desc);
        OutputDebugString(desc.Description);
        OutputDebugString(_T("\n"));

        std::wstring strDesc = desc.Description;

        if (strDesc.find(L"NVIDIA") != std::string::npos)
        {
            adapter = adpt;
            break;
        }
    }

    D3D_FEATURE_LEVEL featureLevel;
    for (auto level : featureLevels)
    {
        hr = D3D12CreateDevice(adapter, level, IID_PPV_ARGS(&_device));
        if (SUCCEEDED(hr))
        {
            featureLevel = level;
            break;
        }
    }

    hr = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator));
    CheckHR(hr, _T("Command Allocator Creation Failed"));

    hr = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator, nullptr, IID_PPV_ARGS(&_commandList));
    CheckHR(hr, _T("Command List Creation Failed"));

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    hr = _device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue));
    CheckHR(hr, _T("Command Queue Creation Failed"));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = WIDTH;
    swapChainDesc.Height = HEIGHT;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = false;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    hr = _dxgiFactory->CreateSwapChainForHwnd(_commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, (IDXGISwapChain1**)&_swapChain);
    CheckHR(hr, _T("Swap Chain Creation Failed"));

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ID3D12DescriptorHeap* rtvHeap;
    hr = _device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
    CheckHR(hr, _T("RTV Heap Creation Failed"));
    DXGI_SWAP_CHAIN_DESC swcDesc = {};
    hr = _swapChain->GetDesc(&swcDesc);
    CheckHR(hr, _T("Get Swap Chain Desc Failed"));
    std::vector<ID3D12Resource*> backBuffers(swcDesc.BufferCount);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    for (int i = 0; i < swcDesc.BufferCount; ++i)
    {
        hr = _swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
        CheckHR(hr, _T("Get Buffer Failed"));
        _device->CreateRenderTargetView(backBuffers[i], &rtvDesc, rtvHandle);
        rtvHandle.ptr += _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Alignment = 0;
    depthDesc.Width = WIDTH;
    depthDesc.Height = HEIGHT;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    depthDesc.MipLevels = 1;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    D3D12_HEAP_PROPERTIES depthHeapProp = {};
    depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
    depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_CLEAR_VALUE depthClearValue = {};
    depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthClearValue.DepthStencil.Depth = 1.0f;

    ID3D12Resource* depthBuffer = nullptr;
    hr = _device->CreateCommittedResource(
        &depthHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue,
        IID_PPV_ARGS(&depthBuffer)
    );

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = 1;
    ID3D12DescriptorHeap* dsvHeap;
    hr = _device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    _device->CreateDepthStencilView(depthBuffer, &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());

    ID3D12Fence* fence = nullptr;
    UINT64 fenceValue = 0;
    hr = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    CheckHR(hr, _T("Create Fence Failed"));

    ShowWindow(hwnd, nCmdShow);

    // PMD
    struct PMDHeader
    {
        float version;
        char model_name[20];
        char comment[256];
    };
    char signature[3];
    PMDHeader header;
    FILE* fp;
    auto err = fopen_s(&fp, "Model/初音ミク.pmd", "rb");
    if (fp == nullptr)
    {
        OutputDebugString(_T("File Open Failed\n"));
        return -1;
    }

    fread(signature, 1, 3, fp);
    fread(&header, sizeof(PMDHeader), 1, fp);

    unsigned int vertexCount;
    fread(&vertexCount, sizeof(unsigned int), 1, fp);

    constexpr unsigned int pmd_vertex_size = 38;
    std::vector<PMD_VERTEX> vertices(vertexCount);
    for (int i = 0; i < vertexCount; ++i)
    {
        fread(&vertices[i], pmd_vertex_size, 1, fp);
    }

    unsigned int indicesCount;
    fread(&indicesCount, sizeof(unsigned int), 1, fp);

    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = sizeof(PMD_VERTEX) * vertexCount;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* vertexBuffer;
    hr = _device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer)
    );
    CheckHR(hr, _T("Create Vertex Buffer Failed"));

    PMD_VERTEX* vertexMap = nullptr;
    hr = vertexBuffer->Map(0, nullptr, (void**)&vertexMap);
    CheckHR(hr, _T("Map Vertex Buffer Failed"));
    std::copy(vertices.begin(), vertices.end(), vertexMap);
    vertexBuffer->Unmap(0, nullptr);

    D3D12_VERTEX_BUFFER_VIEW vbView = {};
    vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vbView.SizeInBytes = sizeof(PMD_VERTEX) * vertexCount;
    vbView.StrideInBytes = sizeof(PMD_VERTEX);

    std::vector<unsigned short> indices(indicesCount);
    fread(indices.data(), sizeof(indices[0]) * indices.size(), 1, fp);
    fclose(fp);

    ID3D12Resource* indexBuffer;
    resDesc.Width = sizeof(indices[0]) * indices.size();
    hr = _device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&indexBuffer)
    );
    CheckHR(hr, _T("Create Index Buffer Failed"));

    unsigned short* indexMap = nullptr;
    hr = indexBuffer->Map(0, nullptr, (void**)&indexMap);
    CheckHR(hr, _T("Map Index Buffer Failed"));
    std::copy(indices.begin(), indices.end(), indexMap);
    indexBuffer->Unmap(0, nullptr);

    D3D12_INDEX_BUFFER_VIEW ibView = {};
    ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    ibView.SizeInBytes = sizeof(indices[0]) * indices.size();
    ibView.Format = DXGI_FORMAT_R16_UINT;


    // シェーダのコンパイル
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    hr = D3DCompileFromFile(
        L"BasicVertexShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "BasicVS",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &vsBlob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            OutputDebugString(_T("File Not Found\n"));
        }
        else
        {
            std::string errstr;
            errstr.resize(errorBlob->GetBufferSize());
            std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
            errstr += "\n";
            OutputDebugStringA(errstr.c_str());
        }
        return 1;
    }
    hr = D3DCompileFromFile(
        L"BasicPixelShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "BasicPS",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &psBlob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            OutputDebugString(_T("File Not Found\n"));
        }
        else
        {
            std::string errstr;
            errstr.resize(errorBlob->GetBufferSize());
            std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
            errstr += "\n";
            OutputDebugStringA(errstr.c_str());
        }
        return 1;
    }

    // pipeline
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
        { "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
        { "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
        { "BONE_NO",0,DXGI_FORMAT_R16G16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
        { "WEIGHT",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
        //{ "EDGE_FLG",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
    gpipeline.pRootSignature = nullptr;
    gpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();
    gpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();
    gpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
    gpipeline.PS.BytecodeLength = psBlob->GetBufferSize();

    gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    gpipeline.HS.BytecodeLength = 0;
    gpipeline.HS.pShaderBytecode = nullptr;
    gpipeline.DS.BytecodeLength = 0;
    gpipeline.DS.pShaderBytecode = nullptr;
    gpipeline.GS.BytecodeLength = 0;
    gpipeline.GS.pShaderBytecode = nullptr;

    gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    gpipeline.RasterizerState.FrontCounterClockwise = false;
    gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    gpipeline.RasterizerState.DepthClipEnable = true;
    gpipeline.RasterizerState.MultisampleEnable = false;
    gpipeline.RasterizerState.AntialiasedLineEnable = false;
    gpipeline.RasterizerState.ForcedSampleCount = 0;
    gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    gpipeline.DepthStencilState.DepthEnable = true;
    gpipeline.DepthStencilState.StencilEnable = false;
    gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

    gpipeline.BlendState.AlphaToCoverageEnable = false;
    gpipeline.BlendState.IndependentBlendEnable = false;
    gpipeline.BlendState.RenderTarget->BlendEnable = true;
    gpipeline.BlendState.RenderTarget->SrcBlend = D3D12_BLEND_SRC_ALPHA;
    gpipeline.BlendState.RenderTarget->DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    gpipeline.BlendState.RenderTarget->BlendOp = D3D12_BLEND_OP_ADD;

    gpipeline.NodeMask = 0;
    gpipeline.SampleDesc.Count = 1;
    gpipeline.SampleDesc.Quality = 0;
    gpipeline.SampleMask = 0xffffffff;
    gpipeline.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    gpipeline.BlendState.AlphaToCoverageEnable = false;
    gpipeline.BlendState.IndependentBlendEnable = false;

    D3D12_RENDER_TARGET_BLEND_DESC rtvBlendDesc = {};
    rtvBlendDesc.BlendEnable = false;
    rtvBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    rtvBlendDesc.LogicOpEnable = false;

    gpipeline.BlendState.RenderTarget[0] = rtvBlendDesc;

    gpipeline.InputLayout.pInputElementDescs = inputLayout;
    gpipeline.InputLayout.NumElements = _countof(inputLayout);

    gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    gpipeline.NumRenderTargets = 1;
    gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    gpipeline.SampleDesc.Count = 1;
    gpipeline.SampleDesc.Quality = 0;

    ID3D12RootSignature* rootSignature = nullptr;
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descTblRange[1] = {};
    descTblRange[0].NumDescriptors = 1;
    descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descTblRange[0].BaseShaderRegister = 0;
    descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParam = {};
    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam.DescriptorTable.pDescriptorRanges = &descTblRange[0];
    rootParam.DescriptorTable.NumDescriptorRanges = 1;
    rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = &rootParam;

    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = &samplerDesc;

    ID3DBlob* sigBlob = nullptr;
    hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &sigBlob, &errorBlob);
    hr = _device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    sigBlob->Release();

    gpipeline.pRootSignature = rootSignature;
    ID3D12PipelineState* pipelineState = nullptr;
    hr = _device->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelineState));

    D3D12_VIEWPORT viewport = {};
    viewport.Width = WIDTH;
    viewport.Height = HEIGHT;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissor = {};
    scissor.left = 0;
    scissor.top = 0;
    scissor.right = scissor.left + WIDTH;
    scissor.bottom = scissor.top + HEIGHT;

    struct MatricesData
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX viewproj;
    };

    DirectX::XMMATRIX worldMat = DirectX::XMMatrixIdentity();
    DirectX::XMFLOAT3 eye(0, 10, -15);
    DirectX::XMFLOAT3 target(0, 10, 0);
    DirectX::XMFLOAT3 up(0, 1, 0);
    DirectX::XMMATRIX viewMat = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eye), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));
    DirectX::XMMATRIX projMat = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, (float)WIDTH / HEIGHT, 1.0f, 100.0f);

    D3D12_HEAP_PROPERTIES cbHeapProp = {};
    cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
    cbHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    cbHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC cbResDesc = {};
    cbResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    cbResDesc.Width = (sizeof(MatricesData) + 0xff) & ~0xff;
    cbResDesc.Height = 1;
    cbResDesc.DepthOrArraySize = 1;
    cbResDesc.MipLevels = 1;
    cbResDesc.Format = DXGI_FORMAT_UNKNOWN;
    cbResDesc.SampleDesc.Count = 1;
    cbResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    cbResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* cbMatrices;
    hr = _device->CreateCommittedResource(
        &cbHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &cbResDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&cbMatrices)
    );

    MatricesData* matricesMap = nullptr;
    hr = cbMatrices->Map(0, nullptr, (void**)&matricesMap);
    matricesMap->world = worldMat;
    matricesMap->viewproj = viewMat * projMat;

    ID3D12DescriptorHeap* cbvHeap;
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    hr = _device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvHeap));

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = cbMatrices->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = cbMatrices->GetDesc().Width;
    _device->CreateConstantBufferView(&cbvDesc, cbvHeap->GetCPUDescriptorHandleForHeapStart());

    auto basicHeapHandle = cbvHeap->GetGPUDescriptorHandleForHeapStart();

    MSG msg = {};
    unsigned int frame = 0;
    float angle = 0.0f;
    while (true)
    {
        worldMat = DirectX::XMMatrixRotationY(angle);
        matricesMap->world = worldMat;
        matricesMap->viewproj = viewMat * projMat;
        angle += 0.01f;

        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (msg.message == WM_QUIT)
        {
            break;
        }


        auto bbIdx = _swapChain->GetCurrentBackBufferIndex();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = backBuffers[bbIdx];
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        _commandList->SetPipelineState(pipelineState);

        auto rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        rtvHandle.ptr += bbIdx * _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        auto dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();

        _commandList->ResourceBarrier(1, &barrier);
        _commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
        float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        _commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        _commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        _commandList->RSSetViewports(1, &viewport);
        _commandList->RSSetScissorRects(1, &scissor);

        _commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        _commandList->IASetVertexBuffers(0, 1, &vbView);
        _commandList->IASetIndexBuffer(&ibView);

        _commandList->SetGraphicsRootSignature(rootSignature);
        _commandList->SetDescriptorHeaps(1, &cbvHeap);
        _commandList->SetGraphicsRootDescriptorTable(0, cbvHeap->GetGPUDescriptorHandleForHeapStart());

        _commandList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

        _commandList->ResourceBarrier(1, &barrier);

        _commandList->Close();

        ID3D12CommandList* cmdLists[] = { _commandList };
        _commandQueue->ExecuteCommandLists(1, cmdLists);
        _commandQueue->Signal(fence, ++fenceValue);

        while (fence->GetCompletedValue() < fenceValue)
        {
          ;
        }

        _swapChain->Present(1, 0);
        _commandAllocator->Reset();
        _commandList->Reset(_commandAllocator, nullptr);
    }

    UnregisterClass(wc.lpszClassName, hInstance);
    return 0;
}
