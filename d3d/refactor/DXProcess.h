#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>

class DXProcess
{
public:
    void Init();
    void Render();

private:
    void EnableDebug();
    void CreateDevice();
    void CreateCommandQueue();
    void CreateCommandAllocator();
    void CreateCommandList();

    void CreateSwapChain();
    void SetRenderTargetView();
    void SetDepthStencilView();

    void CreateFence();

    void CompileShaders();

    void SetGraphicsPipeline();
    ID3D12RootSignature* CreateRootSignature();

    void CreateViewPort();

    void SetMatrixBuffer();

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
};

