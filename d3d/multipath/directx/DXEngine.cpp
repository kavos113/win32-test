#include "DXEngine.h"

#include <chrono>
#include <memory>
#include <tchar.h>

#include "resources/DXCommand.h"
#include "resources/DXFence.h"

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

    globalHeap = std::make_shared<GlobalDescriptorHeap1>();
    globalHeap->Init();

    display.SetHWND(hwnd);
    HRESULT hr = display.Init(globalHeap);
    if (FAILED(hr)) return E_FAIL;

    hr = displayMatrix.Init(globalHeap);
    if (FAILED(hr)) return E_FAIL;

    model = std::make_unique<PMDModel>("model/�����~�N.pmd", globalHeap);
    model->Read();

    renderer = std::make_unique<PMDRenderer>(hwnd, wr, globalHeap);
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
        OutputDebugString(_T("Failed to render\n"));
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
    OutputDebugString(_T("Debug layer is enabled\n"));
}

// �d�������Ă����̂�WM_PAINT���b�Z�[�W���Ă΂�Ă��炸�y�C���g����Ă��Ȃ��������������� T_T
HRESULT DXEngine::OnRender()
{
    auto start = timeGetTime();

    globalHeap->SetToCommand();

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
        OutputDebugString(_T("Failed to execute commands\n"));
        return hr;
    }

    display.Present();

    auto end = timeGetTime();
    auto elapsed = end - start;
    OutputDebugString(_T("Elapsed time: "));
    OutputDebugString(std::to_wstring(elapsed).c_str());
    OutputDebugString(_T("ms\n"));
    
    return S_OK;
}
