#include "DXProcess.h"

#include <chrono>
#include <memory>
#include <ratio>
#include <tchar.h>

#include "DXCommand.h"
#include "DXDevice.h"
#include "DXFactory.h"
#include "DXFence.h"
#include "PMDModel.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")

HRESULT DXProcess::Init()
{
    EnableDebug();

    DXFactory::Init();
    DXDevice::Init();
    DXCommand::Init();
    DXFence::Init();

    globalHeap = std::make_shared<GlobalDescriptorHeap>();
    globalHeap->Init();

    display.SetHWND(hwnd);
    HRESULT hr = display.Init();
    if (FAILED(hr)) return E_FAIL;

    hr = displayMatrix.Init(globalHeap);
    if (FAILED(hr)) return E_FAIL;

    model = std::make_unique<PMDModel>("model/初音ミクmetal.pmd", globalHeap);
    model->Read();

    renderer = std::make_unique<PMDRenderer>(hwnd, wr, globalHeap);
    hr = renderer->Init();
    if (FAILED(hr)) return E_FAIL;

    return S_OK;
}

void DXProcess::Render()
{
    HRESULT hr = OnRender();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to render\n"));
        return;
    }
}

void DXProcess::SetHWND(HWND hwnd)
{
    this->hwnd = hwnd;
}

void DXProcess::EnableDebug()
{
    ID3D12Debug* debugController = nullptr;
    HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
    if (hr == S_OK)
    {
        debugController->EnableDebugLayer();
    }
    debugController->EnableDebugLayer();
    debugController->Release();
    OutputDebugString(_T("Debug layer is enabled\n"));
}

// 重く見えていたのはWM_PAINTメッセージが呼ばれておらずペイントされていなかっただけだった T_T
HRESULT DXProcess::OnRender()
{
    auto start = std::chrono::high_resolution_clock::now();

    display.SetBeginBarrier();

    renderer->SetPipelineState();

    display.SetView();
    model->SetIA();
    renderer->SetRootSignature();

    globalHeap->SetToCommand();

    displayMatrix.Render();
    model->Render();

    display.SetEndBarrier();

    HRESULT hr = DXCommand::ExecuteCommands();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to execute commands\n"));
        return hr;
    }

    display.Present();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    OutputDebugStringA(std::to_string(elapsed.count()).c_str());
    OutputDebugStringA("ms\n");

    return S_OK;
}
