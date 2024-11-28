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

    display.SetHWND(hwnd);
    HRESULT hr = display.Init();
    if (FAILED(hr)) return E_FAIL;

    renderer = std::make_unique<PMDRenderer>(hwnd, wr);
    hr = renderer->Init();
    if (FAILED(hr)) return E_FAIL;

    hr = displayMatrix.Init();
    if (FAILED(hr)) return E_FAIL;

    if (DXFactory::GetDXGIFactory() != nullptr)
    {
        OutputDebugString(_T("DXGI Factory is created\n"));
    }
    else
    {
        OutputDebugString(_T("DXGI Factory is not created\n"));
    }

    model = std::make_unique<PMDModel>("model/初音ミクmetal.pmd");
    model->Read();

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
