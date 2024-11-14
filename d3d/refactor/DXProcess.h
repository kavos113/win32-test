#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <vector>

#include "PMDModel.h"

class DXProcess
{
public:
    void Init();
    void Render();

    DXProcess(HWND hwnd, RECT wr)
        : 
        m_dxgiFactory(nullptr),
        m_device(nullptr),
        m_commandQueue(nullptr),
        m_commandAllocator(nullptr),
        m_commandList(nullptr),
        m_fence(nullptr),
        m_fenceValue(0),
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
        wr(wr),
        hwnd(hwnd)
    {
        
    }

private:
    void EnableDebug();
    HRESULT CreateDevice();
    HRESULT CreateCommandQueue();
    HRESULT CreateCommandAllocator();
    HRESULT CreateCommandList();

    HRESULT CreateSwapChain();
    HRESULT SetRenderTargetView();
    HRESULT SetDepthStencilView();

    HRESULT CreateFence();

    HRESULT CompileShaders();

    HRESULT SetGraphicsPipeline();
    HRESULT CreateRootSignature();

    HRESULT CreateViewPort();

    HRESULT SetMatrixBuffer();

    HRESULT OnRender();

    IDXGIFactory6* m_dxgiFactory;
    ID3D12Device* m_device;

    ID3D12CommandQueue* m_commandQueue;
    ID3D12CommandAllocator* m_commandAllocator;
    ID3D12GraphicsCommandList* m_commandList;

    ID3D12Fence* m_fence;
    UINT64 m_fenceValue;

    IDXGISwapChain4* m_swapChain;
    ID3D12DescriptorHeap* m_rtvHeap;
    std::vector<ID3D12Resource*> back_buffers_;

    ID3D12Resource* m_depthStencilBuffer;
    ID3D12DescriptorHeap* m_dsvHeap;

    ID3D10Blob* m_vsBlob;
    ID3D10Blob* m_psBlob;

    ID3D12PipelineState* m_pipelineState;
    ID3D12RootSignature* m_rootSignature;

    ID3D12DescriptorHeap* m_cbvHeap;

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;

    RECT wr;
    HWND hwnd;

    std::unique_ptr<PMDModel> model;
};

