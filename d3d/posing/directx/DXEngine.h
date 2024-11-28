#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <vector>

#include "DXWorld.h"
#include "PMDModel.h"

class DXEngine
{
public:
    HRESULT Init();
    void Render();

    void SetHWND(HWND hwnd);

    DXEngine(HWND hwnd, RECT wr)
        : 
        m_swapChain(nullptr),
        m_rtvHeap(nullptr),
        m_depthStencilBuffer(nullptr),
        m_dsvHeap(nullptr),
        m_vsBlob(nullptr),
        m_psBlob(nullptr),
        m_pipelineState(nullptr),
        m_rootSignature(nullptr),
        m_viewport(),
        m_scissorRect(),
        world(),
        wr(wr),
        hwnd(hwnd)
    {
        
    }

private:
    void EnableDebug();

    HRESULT CreateSwapChain();
    HRESULT SetRenderTargetView();
    HRESULT SetDepthStencilView();

    HRESULT CompileShaders();

    HRESULT SetGraphicsPipeline();
    HRESULT CreateRootSignature();

    HRESULT CreateViewPort();

    HRESULT OnRender();

    IDXGISwapChain4* m_swapChain;
    ID3D12DescriptorHeap* m_rtvHeap;
    std::vector<ID3D12Resource*> back_buffers_;

    ID3D12Resource* m_depthStencilBuffer;
    ID3D12DescriptorHeap* m_dsvHeap;

    ID3D10Blob* m_vsBlob;
    ID3D10Blob* m_psBlob;

    ID3D12PipelineState* m_pipelineState;
    ID3D12RootSignature* m_rootSignature;

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;

    DXWorld world;

    RECT wr;
    HWND hwnd;

    std::unique_ptr<PMDModel> model;
};

