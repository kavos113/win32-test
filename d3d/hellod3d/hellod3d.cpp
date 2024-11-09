#include <d3d12.h>
#include <dxgi1_6.h>
#include <windows.h>
#include <tchar.h>
#include <debugapi.h>
#include <string>
#include <vector>


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

D3D_FEATURE_LEVEL featureLevels[] = {
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc = {};

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = _T("WindowClass1");

	RegisterClassEx(&wc);

	RECT wr = { 0, 0, 800, 600 };

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
	if (hr != S_OK)
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
	if (hr != S_OK)
	{
		OutputDebugString(_T("Failed to create command allocator\n"));
		return 1;
	}
	hr = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));
	if (hr != S_OK)
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
	if (hr != S_OK)
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
	if (hr != S_OK)
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
	if (hr != S_OK)
	{
		OutputDebugString(_T("Failed to create RTV descriptor heap\n"));
		return 1;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	hr = _swapchain->GetDesc(&swapChainDesc);
	if (hr != S_OK)
	{
		OutputDebugString(_T("Failed to get swap chain description\n"));
		return 1;
	}

	std::vector<ID3D12Resource*> _backBuffers(swapChainDesc.BufferCount);
	for (int idx = 0; idx < swapChainDesc.BufferCount; ++idx)
	{
		hr = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
		if (hr != S_OK)
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
