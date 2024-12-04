#pragma once
#include <DirectXMath.h>
#include <dxgi1_6.h>
#include <memory>
#include <vector>

#include "DXDescriptorHeap.h"
#include "GlobalDescriptorHeap.h"
#include "buffer/ConstantBuffer.h"
#include "buffer/DepthStencilBuffer.h"

class Display
{
    struct BaseVertex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 tex;
    };

public:
    HRESULT Init(const std::shared_ptr<GlobalDescriptorHeap>& globalHeap);
    void RenderToBackBuffer() const;
    void Present() const;

    void SetRenderToBase1();
    void SetViewport() const;
    void SetBaseEnd();
    void SetRenderToBackBuffer();
    void SetRenderToLightDSV();
    void UseLightDSV();
    void EndRenderToLightDSV();
    void EndRender();

    void SetHWND(HWND hwnd);

    Display(HWND hwnd, RECT wr)
        :
        m_swapChain(nullptr),
        m_depthStencilBuffer(wr),
        m_depthSrvHeapId(0),
        m_lightDepthBuffer(wr),
        m_viewport(),
        m_scissorRect(),
        wr(wr),
        hwnd(hwnd),
        m_barrier(),
        m_renderResource(nullptr),
        m_srvHeapId(0),
        vertex_buffer_view_(),
        m_pipelineState(nullptr),
        m_rootSignature(nullptr),
        blur_weight_heap_id_(0)
    {
    }

private:
    HRESULT CreateSwapChain();
    HRESULT CreateBackBuffers();
    HRESULT SetRenderTargetView();
    HRESULT SetDepthStencilView();
    HRESULT SetBaseRenderTargetView();

    HRESULT CreateViewPort();

    HRESULT CreateRenderResource();
    HRESULT CreateShaderResourceView();
    HRESULT CreateBasePolygon();
    HRESULT CreateBasePipeline();
    HRESULT CreateBlurBuffer();

    void Barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

    IDXGISwapChain4* m_swapChain;
    DXDescriptorHeap m_rtvHeap;
    std::vector<ID3D12Resource*> back_buffers_;

    DepthStencilBuffer m_depthStencilBuffer;
    DXDescriptorHeap m_dsvHeap;
    GLOBAL_HEAP_ID m_depthSrvHeapId;

    DepthStencilBuffer m_lightDepthBuffer;

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;

    RECT wr;
    HWND hwnd;

    D3D12_RESOURCE_BARRIER m_barrier;

    ID3D12Resource* m_renderResource;

    GLOBAL_HEAP_ID m_srvHeapId;
    DXDescriptorHeap m_baseRtvHeap;

    std::shared_ptr<GlobalDescriptorHeap> globalHeap;

    ConstantBuffer<BaseVertex> vertex_buffer_;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;

    ID3D12PipelineState* m_pipelineState;
    ID3D12RootSignature* m_rootSignature;

    ConstantBuffer<float> blur_weight_buffer_;
    GLOBAL_HEAP_ID blur_weight_heap_id_;

    float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    uint32_t shadow_definition = 1024;
};

