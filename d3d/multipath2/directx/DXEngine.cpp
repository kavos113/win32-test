#include "DXEngine.h"

#include <chrono>
#include <memory>
#include <tchar.h>

#include "descriptor_heap/GlobalDescriptorHeapManager.h"
#include "resources/DXCommand.h"
#include "resources/DXFence.h"
#include "resources/DXFactory.h"
#include "resources/DXDevice.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")
#pragma comment(lib, "winmm.lib")

HRESULT DXEngine::Init()
{
    EnableDebug();

    DXFactory::Init();
    DXDevice::Init();
    DXCommand::Init();
    DXFence::Init();

    GlobalDescriptorHeapManager::Init();

    DescriptorHeapSegmentManager& base_poly_manager = GlobalDescriptorHeapManager::CreateShaderManager(
        "base_poly",
        512,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    display.SetHWND(hwnd);
    HRESULT hr = display.Init(base_poly_manager);
    if (FAILED(hr)) return E_FAIL;

    DescriptorHeapSegmentManager& model_manager = GlobalDescriptorHeapManager::CreateShaderManager(
        "model",
        512,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    hr = displayMatrix.Init(model_manager);
    if (FAILED(hr)) return E_FAIL;

    model = std::make_unique<PMDModel>("model/初音ミク.pmd", model_manager);
    model->Read();

    renderer = std::make_unique<PMDRenderer>(hwnd, wr, model_manager);
    hr = renderer->Init();
    if (FAILED(hr)) return E_FAIL;

    model->PlayAnimation();

    return S_OK;
}

void DXEngine::Render()
{
    HRESULT hr = OnRender();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[DXEngine.cpp] Failed to render\n"));
        return;
    }
}

void DXEngine::SetHWND(HWND hwnd)
{
    this->hwnd = hwnd;
}

void DXEngine::EnableDebug()
{
    ID3D12Debug* debugController = nullptr;
    HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
    if (hr == S_OK)
    { 
        debugController->EnableDebugLayer();
    }
    debugController->EnableDebugLayer();
    debugController->Release();
    OutputDebugString(_T("[DXEngine.cpp] Debug layer is enabled\n"));
}

HRESULT DXEngine::OnRender()
{
    auto start = timeGetTime();

    GlobalDescriptorHeapManager::SetToCommand();

    display.SetRenderToBase1Begin();

    model->UpdateAnimation();

    renderer->SetPipelineState();
    renderer->SetRootSignature();
    displayMatrix.Render();
    display.SetViewports();
    model->SetIA();
    model->Render();

    display.SetRenderToBase1End();

    display.RenderToBase2();

    display.SetRenderToBackBuffer();
    display.RenderToBackBuffer();
    display.EndRender();

    HRESULT hr = DXCommand::ExecuteCommands();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[DXEngine.cpp] Failed to execute commands\n"));
        return hr;
    }

    display.Present();

    auto end = timeGetTime();
    auto elapsed = end - start;
    OutputDebugString(_T("Elapsed time: "));
    OutputDebugString(std::to_wstring(elapsed).c_str());
    OutputDebugString(_T("[DXEngine.cpp] ms\n"));
    
    return S_OK;
}
