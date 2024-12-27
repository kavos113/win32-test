#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <windows.h>
#include <tchar.h>
#include <debugapi.h>
#include <string>
#include <vector>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

D3D_FEATURE_LEVEL featureLevels[] = {
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0
};

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

	RECT wr = { 0, 0, 800, 600 };

    HWND hwnd = InitWindows(hInstance, nCmdShow, wr);

	ID3D12Debug* debugController = nullptr;
	HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	if (hr == S_OK)
	{
		debugController->EnableDebugLayer();
	}
	debugController->EnableDebugLayer();
	debugController->Release();

	// D3D12 initialization
	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;

	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
	if (FAILED(hr))
	{
		OutputDebugString(_T("Failed to create DxGI Factory"));
		return 1;
	}

	// Selecting a GPU
	//
	// std::vector<IDXGIAdapter*> adapters;
	//
	// IDXGIAdapter* adapter = nullptr;
	//
	// for (int i = 0; 
	// 	_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; 
	// 	++i)
	// {
	// 	adapters.push_back(adapter);
	// }
	//
	// for (auto adp : adapters)
	// {
	// 	DXGI_ADAPTER_DESC desc;
	// 	adp->GetDesc(&desc);
	//
	// 	std::wstring str(desc.Description);
	//
	// 	if (str.find(L"NVIDIA") != std::string::npos)
	// 	{
	// 		adapter = adp;
	// 		break;
	// 	}
	// }


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

	std::vector<ID3D12Resource*> _backBuffers(swapChainDesc.BufferCount);
	for (int idx = 0; idx < swapChainDesc.BufferCount; ++idx)
	{
		hr = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
		if (FAILED(hr))
		{
			OutputDebugString(_T("Failed to get buffer from swap chain\n"));
			return 1;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, rtvHandle);
	}

	// fence
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceValue = 0;

	hr = _dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));


    // copy vertex data to GPU
	// DirectX::XMFLOAT3 vertices[] = {
	// 	{-0.5f, -0.5f, 0.0f}, // index 0
 //        {-0.5f,  0.5f, 0.0f}, // index 1
 //        { 0.5f, -0.5f, 0.0f}, // index 2
 //        { 0.5f,  0.5f, 0.0f}  // index 3
 //    };
 //
	// unsigned short indices[] = {
	// 	0, 1, 2,
	// 	2, 1, 3
 //    };

	DirectX::XMFLOAT3 vertices[] = {
		{-0.5f, 0.5f, 0.0f},
		{-0.5f, -0.5f, 0.0f},
		{0.5f, 0.5f, 0.0f}
	};

	unsigned short indices[] = {
		0, 1, 2
	};

	D3D12_HEAP_PROPERTIES heap_properties = {};

	heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heap_properties.CreationNodeMask = 0;
	heap_properties.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC resource_desc = {};

	resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resource_desc.Alignment = 0;
	resource_desc.Width = sizeof(vertices);
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

	DirectX::XMFLOAT3* vertexMap = nullptr;

	hr = vertexBuffer->Map(0, nullptr, (void**)&vertexMap);
	if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to map vertex buffer\n"));
        return 1;
    }

	std::copy(std::begin(vertices), std::end(vertices), vertexMap);

	vertexBuffer->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbv = {};

	vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbv.SizeInBytes = sizeof(vertices);
	vbv.StrideInBytes = sizeof(vertices[0]);

    ID3D12Resource* indexBuffer = nullptr;

    resource_desc.Width = sizeof(indices);

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

    std::copy(std::begin(indices), std::end(indices), indexMap);

    indexBuffer->Unmap(0, nullptr);

    D3D12_INDEX_BUFFER_VIEW ibv = {};

    ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    ibv.SizeInBytes = sizeof(indices);
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
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
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

    graphics_pipeline.RasterizerState.MultisampleEnable = FALSE;
    graphics_pipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    graphics_pipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    graphics_pipeline.RasterizerState.DepthClipEnable = TRUE;

    graphics_pipeline.BlendState.AlphaToCoverageEnable = FALSE;
    graphics_pipeline.BlendState.IndependentBlendEnable = FALSE;

    D3D12_RENDER_TARGET_BLEND_DESC rtvBlendDesc = {};
    rtvBlendDesc.BlendEnable = FALSE;
    rtvBlendDesc.LogicOpEnable = FALSE;
    rtvBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    graphics_pipeline.BlendState.RenderTarget[0] = rtvBlendDesc;

    graphics_pipeline.InputLayout.pInputElementDescs = inputElementDescs;
    graphics_pipeline.InputLayout.NumElements = _countof(inputElementDescs);

    graphics_pipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    graphics_pipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    graphics_pipeline.NumRenderTargets = 1;
    graphics_pipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    graphics_pipeline.SampleDesc.Count = 1;
	graphics_pipeline.SampleDesc.Quality = 0;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

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

    ID3D12RootSignature* rootSignature = nullptr;

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

    // main loop

	ShowWindow(hwnd, nCmdShow);

	MSG msg;

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

		// swap chain
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

		D3D12_RESOURCE_BARRIER barrier = {};

		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = _backBuffers[bbIdx];
		barrier.Transition.Subresource = 0;

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		_cmdList->ResourceBarrier(1, &barrier);

		auto rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		_cmdList->OMSetRenderTargets(1, &rtvHandle, TRUE, nullptr);

		float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		_cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

        // draw polygon
        _cmdList->SetPipelineState(_pipelineState);
        _cmdList->SetGraphicsRootSignature(rootSignature);
        _cmdList->RSSetViewports(1, &viewport);
        _cmdList->RSSetScissorRects(1, &scissorRect);

        _cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        _cmdList->IASetVertexBuffers(0, 1, &vbv);
        _cmdList->IASetIndexBuffer(&ibv);

        _cmdList->DrawIndexedInstanced(3, 1, 0, 0, 0);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

		_cmdList->ResourceBarrier(1, &barrier);

		_cmdList->Close();

		ID3D12CommandList* cmdLists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdLists);

		_cmdQueue->Signal(_fence, ++_fenceValue);

		if (_fence->GetCompletedValue() != _fenceValue)
		{
			HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			_fence->SetEventOnCompletion(_fenceValue, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		_cmdAllocator->Reset();
		_cmdList->Reset(_cmdAllocator, nullptr);

		_swapchain->Present(1, 0);
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
