#include<Windows.h>
#include<tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<vector>

#include<d3dcompiler.h>
#include<DirectXTex.h>
#include <string>
//#include<d3dx12.h>

#pragma comment(lib,"DirectXTex.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

#pragma pack(push, 1)
struct PMD_VERTEX
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 uv;
	uint16_t bone_no[2];
	uint8_t  weight;
	uint8_t  EdgeFlag;
	uint16_t dummy;
};
#pragma pack(pop)

LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

const unsigned int window_width = 1280;
const unsigned int window_height = 720;

IDXGIFactory4* _dxgiFactory = nullptr;
ID3D12Device* _dev = nullptr;
ID3D12CommandAllocator* _cmdAllocator = nullptr;
ID3D12GraphicsCommandList* _cmdList = nullptr;
ID3D12CommandQueue* _cmdQueue = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

void EnableDebugLayer() {
    ID3D12Debug* debugLayer = nullptr;
    auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
    debugLayer->EnableDebugLayer();
    debugLayer->Release();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    auto result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = _T("pmd2");
    RegisterClassEx(&wc);

    RECT rc = { 0, 0, window_width, window_height };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
    HWND hwnd = CreateWindow(
        wc.lpszClassName, 
        _T("pmd2"), 
        WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT,
        CW_USEDEFAULT, 
        rc.right - rc.left, 
        rc.bottom - rc.top, 
        nullptr,
        nullptr,
        hInstance, 
        nullptr);

    EnableDebugLayer();

    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
    std::vector <IDXGIAdapter*> adapters;
    IDXGIAdapter* tmpAdapter = nullptr;
    for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        adapters.push_back(tmpAdapter);
    }
    for (auto adpt : adapters) {
        DXGI_ADAPTER_DESC adesc = {};
        adpt->GetDesc(&adesc);
        std::wstring strDesc = adesc.Description;
        if (strDesc.find(L"NVIDIA") != std::string::npos) {
            tmpAdapter = adpt;
            break;
        }
    }

    D3D_FEATURE_LEVEL featureLevel;
    for (auto l : levels) {
        if (D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(&_dev)) == S_OK) {
            featureLevel = l;
            break;
        }
    }

    result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
    result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));

    D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cmdQueueDesc.NodeMask = 0;
    cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
    swapchainDesc.Width = window_width;
    swapchainDesc.Height = window_height;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.Stereo = false;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    swapchainDesc.BufferCount = 2;
    swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


    result = _dxgiFactory->CreateSwapChainForHwnd(_cmdQueue,
        hwnd,
        &swapchainDesc,
        nullptr,
        nullptr,
        (IDXGISwapChain1**)&_swapchain);

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.NodeMask = 0;
    heapDesc.NumDescriptors = 2;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ID3D12DescriptorHeap* rtvHeaps = nullptr;
    result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
    DXGI_SWAP_CHAIN_DESC swcDesc = {};
    result = _swapchain->GetDesc(&swcDesc);
    std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    for (size_t i = 0; i < swcDesc.BufferCount; ++i) {
        result = _swapchain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&_backBuffers[i]));
        rtvDesc.Format = _backBuffers[i]->GetDesc().Format;
        _dev->CreateRenderTargetView(_backBuffers[i], &rtvDesc, rtvH);
        rtvH.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    D3D12_RESOURCE_DESC depthResDesc = {};
    depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthResDesc.Width = window_width;
    depthResDesc.Height = window_height;
    depthResDesc.DepthOrArraySize = 1;
    depthResDesc.MipLevels = 1;
    depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthResDesc.SampleDesc.Count = 1;
    depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    depthResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthResDesc.Alignment = 0;

    D3D12_HEAP_PROPERTIES depthHeapProp = {};
    depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
    depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_CLEAR_VALUE _depthClearValue = {};
    _depthClearValue.DepthStencil.Depth = 1.0f;
    _depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

    ID3D12Resource* depthBuffer = nullptr;
    result = _dev->CreateCommittedResource(&depthHeapProp, D3D12_HEAP_FLAG_NONE, &depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &_depthClearValue, IID_PPV_ARGS(&depthBuffer));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = 1;
    ID3D12DescriptorHeap* dsvHeap = nullptr;
    result = _dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    _dev->CreateDepthStencilView(depthBuffer, &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());

    ID3D12Fence* _fence = nullptr;
    UINT64 _fenceVal = 0;
    result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

    ShowWindow(hwnd, nCmdShow);

    struct PMDHeader {
        float version; //例：00 00 80 3F == 1.00
        char model_name[20];//モデル名
        char comment[256];//モデルコメント
    };
    char signature[3];
    PMDHeader pmdheader = {};
    FILE* fp;
    auto err = fopen_s(&fp, "Model/初音ミク.pmd", "rb");
    if (fp == nullptr) {
        char strerr[256];
        strerror_s(strerr, 256, err);
        MessageBoxA(hwnd, strerr, "Open Error", MB_ICONERROR);
        return -1;
    }
    fread(signature, sizeof(signature), 1, fp);
    fread(&pmdheader, sizeof(pmdheader), 1, fp);

    unsigned int vertNum;
    fread(&vertNum, sizeof(vertNum), 1, fp);

    constexpr unsigned int pmdvertex_size = 38;
    std::vector<PMD_VERTEX> vertices(vertNum);
    for (auto i = 0; i < vertNum; i++)
    {
        fread(&vertices[i], pmdvertex_size, 1, fp);
    }

    unsigned int indicesNum;
    fread(&indicesNum, sizeof(indicesNum), 1, fp);

    std::vector<unsigned short> indices(indicesNum);

    fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);
    fclose(fp);

    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = vertices.size() * sizeof(PMD_VERTEX);
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    resDesc.Alignment = 0;

    ID3D12Resource* vertBuff= nullptr;
    result = _dev->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertBuff));

    PMD_VERTEX* vertMap = nullptr;
    result = vertBuff->Map(0, nullptr, (void**)&vertMap);
    std::copy(vertices.begin(), vertices.end(), vertMap);
    vertBuff->Unmap(0, nullptr);

    D3D12_VERTEX_BUFFER_VIEW vbView = {};
    vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
    vbView.SizeInBytes = static_cast<UINT>(vertices.size() * sizeof(PMD_VERTEX));
    vbView.StrideInBytes = sizeof(PMD_VERTEX);

    ID3D12Resource* idxBuff = nullptr;
    resDesc.Width = indices.size() * sizeof(indices[0]);

    result = _dev->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&idxBuff));

    unsigned short* mappedIdx = nullptr;
    idxBuff->Map(0, nullptr, (void**)&mappedIdx);
    std::copy(indices.begin(), indices.end(), mappedIdx);
    idxBuff->Unmap(0, nullptr);

    D3D12_INDEX_BUFFER_VIEW ibView = {};
    ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
    ibView.Format = DXGI_FORMAT_R16_UINT;
    ibView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(indices[0]));

    ID3DBlob* _vsBlob = nullptr;
    ID3DBlob* _psBlob = nullptr;

    ID3DBlob* errorBlob = nullptr;
    result = D3DCompileFromFile(L"BasicVertexShader.hlsl",
        nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "BasicVS", "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0, &_vsBlob, &errorBlob);
    if (FAILED(result)) {
        if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            ::OutputDebugStringA("ファイルが見当たりません");
        }
        else {
            std::string errstr;
            errstr.resize(errorBlob->GetBufferSize());
            std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
            errstr += "\n";
            OutputDebugStringA(errstr.c_str());
        }
        exit(1);
    }
    result = D3DCompileFromFile(L"BasicPixelShader.hlsl",
        nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "BasicPS", "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0, &_psBlob, &errorBlob);
    if (FAILED(result)) {
        if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            ::OutputDebugStringA("ファイルが見当たりません");
        }
        else {
            std::string errstr;
            errstr.resize(errorBlob->GetBufferSize());
            std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
            errstr += "\n";
            OutputDebugStringA(errstr.c_str());
        }
        exit(1);
    }

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
    gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
    gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
    gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
    gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();

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

    gpipeline.NumRenderTargets = 1;
    gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

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

    D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};

    renderTargetBlendDesc.BlendEnable = false;
    renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    renderTargetBlendDesc.LogicOpEnable = false;

    gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

    gpipeline.InputLayout.pInputElementDescs = inputLayout;
    gpipeline.InputLayout.NumElements = ARRAYSIZE(inputLayout);

    gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    gpipeline.NumRenderTargets = 1;
    gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    gpipeline.SampleDesc.Count = 1;
    gpipeline.SampleDesc.Quality = 0;

    ID3D12RootSignature* rootsignature = nullptr;
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descTblRange[1] = {};
    descTblRange[0].NumDescriptors = 1;
    descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    descTblRange[0].BaseShaderRegister = 0;
    descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParam = {};
    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParam.DescriptorTable.NumDescriptorRanges = 1;
    rootParam.DescriptorTable.pDescriptorRanges = &descTblRange[0];

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

    ID3DBlob* rootSigBlob = nullptr;
    result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
    result = _dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature));
    rootSigBlob->Release();

    gpipeline.pRootSignature = rootsignature;
    ID3D12PipelineState* _pipelinestate = nullptr;
    result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));

    D3D12_VIEWPORT viewport = {};
    viewport.Width = window_width;
    viewport.Height = window_height;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissor = {};
    scissor.left = 0;
    scissor.top = 0;
    scissor.right = scissor.left + window_width;
    scissor.bottom = scissor.top + window_height;

    struct MatricesData {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX viewproj;
    };

    DirectX::XMMATRIX worldMat = DirectX::XMMatrixIdentity();
    DirectX::XMFLOAT3 eye(0, 10, -15);
    DirectX::XMFLOAT3 target(0, 10, 0);
    DirectX::XMFLOAT3 up(0, 1, 0);
    auto viewMat = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eye), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));
    auto projMat = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, window_width / (float)window_height, 1.0f, 100.0f);
    resDesc.Width = (sizeof(MatricesData) + 0xff) & ~0xff;
    ID3D12Resource* constBuff = nullptr;
    result = _dev->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&constBuff)
    );

    MatricesData* mapMatrix;
    result = constBuff->Map(0, nullptr, (void**)&mapMatrix);
    mapMatrix->world = worldMat;
    mapMatrix->viewproj = viewMat * projMat;

    ID3D12DescriptorHeap* basicDescHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
    descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descHeapDesc.NodeMask = 0;
    descHeapDesc.NumDescriptors = 1;
    descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

    auto basicHeapHandle = basicDescHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = static_cast<UINT>(constBuff->GetDesc().Width);
    _dev->CreateConstantBufferView(&cbvDesc, basicHeapHandle);



    MSG msg = {};
    unsigned int frame = 0;
    float angle = 0.0f;
    while (true)
    {
        worldMat = DirectX::XMMatrixRotationY(angle);
        mapMatrix->world = worldMat;
        mapMatrix->viewproj = viewMat * projMat;
        angle += 0.005f;

        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (msg.message == WM_QUIT) {
            break;
        }

        auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

        D3D12_RESOURCE_BARRIER BarrierDesc = {};
        BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
        BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        _cmdList->SetPipelineState(_pipelinestate);

        auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
        rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        auto dsvH = dsvHeap->GetCPUDescriptorHandleForHeapStart();

        _cmdList->ResourceBarrier(1, &BarrierDesc);
        _cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);
        float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };
        _cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
        _cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        _cmdList->RSSetViewports(1, &viewport);
        _cmdList->RSSetScissorRects(1, &scissor);

        _cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        _cmdList->IASetVertexBuffers(0, 1, &vbView);
        _cmdList->IASetIndexBuffer(&ibView);

        _cmdList->SetGraphicsRootSignature(rootsignature);
        _cmdList->SetDescriptorHeaps(1, &basicDescHeap);
        _cmdList->SetGraphicsRootDescriptorTable(0, basicDescHeap->GetGPUDescriptorHandleForHeapStart());

        _cmdList->DrawIndexedInstanced(indicesNum, 1, 0, 0, 0);

        BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

        _cmdList->ResourceBarrier(1, &BarrierDesc);

        _cmdList->Close();

        ID3D12CommandList* cmdLists[] = { _cmdList };
        _cmdQueue->ExecuteCommandLists(1, cmdLists);
        _cmdQueue->Signal(_fence, ++_fenceVal);
        while (_fence->GetCompletedValue() != _fenceVal) {
            ;
        }
        _swapchain->Present(1, 0);
        _cmdAllocator->Reset();
        _cmdList->Reset(_cmdAllocator, nullptr);
    }

    UnregisterClass(wc.lpszClassName, hInstance);
    return 0;
}