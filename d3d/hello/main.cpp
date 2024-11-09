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
	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;

	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
	if (hr == S_OK)
	{
		OutputDebugString(_T("Failed to create DxGI Factory"));
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

	WNDCLASSEX wc = {};

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = _T("WindowClass1");

	RegisterClassEx(&wc);

	RECT wr = {0, 0, 800, 600};

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
