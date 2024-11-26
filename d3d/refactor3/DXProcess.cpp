#include "DXProcess.h"

#include <chrono>
#include <DirectXMath.h>
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

    hr = SetMatrixBuffer();
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

HRESULT DXProcess::SetMatrixBuffer()
{
    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();

    DirectX::XMFLOAT3 eye(0.0f, 15.0f, -15.0f);
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

    m_matrixBuffer.SetResourceWidth((sizeof(SceneMatrix) + 0xff) & ~0xff);
    HRESULT hr = m_matrixBuffer.CreateBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create matrix buffer\n"));
        return hr;
    }

    m_matrixBuffer.GetMappedBuffer()->world = worldMatrix;
    m_matrixBuffer.GetMappedBuffer()->view = viewMatrix;
    m_matrixBuffer.GetMappedBuffer()->proj = projectionMatrix;
    m_matrixBuffer.GetMappedBuffer()->eye = eye;

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};

    descriptorHeapDesc.NumDescriptors = 1; // texture(SRV) and constant(CBV) buffer
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descriptorHeapDesc.NodeMask = 0;

    hr = m_cbvHeap.CreateDescriptorHeap(&descriptorHeapDesc);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create texture descriptor heap\n"));
        return hr;
    }

    m_matrixBuffer.SetDescriptorHeap(&m_cbvHeap);
    m_matrixBuffer.CreateView();

    return S_OK;
}
 
// 重く見えていたのはWM_PAINTメッセージが呼ばれておらずペイントされていなかっただけだった T_T
HRESULT DXProcess::OnRender()
{
    auto start = std::chrono::high_resolution_clock::now();

    angle += 0.05f;
    m_matrixBuffer.GetMappedBuffer()->world = DirectX::XMMatrixRotationY(angle);

    display.SetBeginBarrier();

    renderer->SetPipelineState();

    display.SetView();

    model->SetIA();

    renderer->SetRootSignature();

    m_cbvHeap.SetToCommand();
    DXCommand::GetCommandList()->SetGraphicsRootDescriptorTable(
        0,
        m_cbvHeap.GetGPUHandle()
    );

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
