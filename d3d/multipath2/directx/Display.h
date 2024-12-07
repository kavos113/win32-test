#pragma once
#include <DirectXMath.h>
#include <dxgi1_6.h>
#include <vector>

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
    HRESULT Init(DescriptorHeapSegmentManager& base_poly_manager);
    void Present() const;

    void SetRenderToBase1Begin();    // base polygon1枚目に書き込むよう設定
    void SetViewports() const;
    void SetRenderToBase1End();      // base polygon1枚目への書き込み終了
    void RenderToBase2();            // base polygon2枚目に，1枚目のbase polygon + blur(by pso1)を描画
    void SetRenderToBackBuffer();    // back bufferに描画するよう設定
    void RenderToBackBuffer() const; // base polygon2枚目 + blur(by pso2)をback bufferに描画
    void EndRender();

    void SetHWND(HWND hwnd);

    Display(HWND hwnd, RECT wr)
        :
        m_swapChain(nullptr),
        m_backBufferSegment(),
        m_depthStencilBuffer(wr),
        m_viewport(),
        m_scissorRect(),
        wr(wr),
        hwnd(hwnd),
        m_barrier(),
        m_renderResource(nullptr),
        m_renderResource2(nullptr),
        m_baseRTVsSegment(),
        m_baseSRVsSegment(),
        m_basePolyManager(nullptr),
        m_rtvManager(nullptr),
        m_dsvManager(nullptr),
        m_vertexBufferView(),
        m_pipelineState(nullptr),
        m_pipelineState2(nullptr),
        m_rootSignature(nullptr),
        m_blurWeightSegment()
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
    std::vector<ID3D12Resource*> back_buffers_;
    DescriptorHeapSegment m_backBufferSegment; // rtv

    DepthStencilBuffer m_depthStencilBuffer;

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;

    RECT wr;
    HWND hwnd;

    D3D12_RESOURCE_BARRIER m_barrier;

    ID3D12Resource* m_renderResource;  // base polygon 1   rtv+srv
    ID3D12Resource* m_renderResource2; // base polygon 2   rtv+srv

    DescriptorHeapSegment m_baseRTVsSegment;
    DescriptorHeapSegment m_baseSRVsSegment;

    DescriptorHeapSegmentManager* m_basePolyManager;
    DescriptorHeapSegmentManager* m_rtvManager;
    DescriptorHeapSegmentManager* m_dsvManager;

    ConstantBuffer<BaseVertex> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    ID3D12PipelineState* m_pipelineState;
    ID3D12PipelineState* m_pipelineState2;
    ID3D12RootSignature* m_rootSignature;

    ConstantBuffer<float> m_blurWeightBuffer;
    DescriptorHeapSegment m_blurWeightSegment;

    const float m_clearColor[4] = { 0.7f, 0.8f, 0.6f, 1.0f };
};

