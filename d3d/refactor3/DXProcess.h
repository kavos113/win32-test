#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <vector>

#include "DXDescriptorHeap.h"
#include "PMDModel.h"
#include "PMDRenderer.h"

class DXProcess
{
    struct SceneMatrix
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;

        DirectX::XMFLOAT3 eye;
    };

public:
    HRESULT Init();
    void Render();

    void SetHWND(HWND hwnd);

    DXProcess(HWND hwnd, RECT wr)
        : 
        m_swapChain(nullptr),
        m_depthStencilBuffer(nullptr),
        m_viewport(),
        m_scissorRect(),
        wr(wr),
        hwnd(hwnd),
        model(nullptr),
        m_constantBufferMap(nullptr)
    {
        
    }

private:
    static void EnableDebug();

    HRESULT CreateSwapChain();
    HRESULT SetRenderTargetView();
    HRESULT SetDepthStencilView();

    HRESULT CreateViewPort();

    HRESULT SetMatrixBuffer();

    HRESULT OnRender();

    IDXGISwapChain4* m_swapChain;
    DXDescriptorHeap m_rtvHeap;
    std::vector<ID3D12Resource*> back_buffers_;

    ID3D12Resource* m_depthStencilBuffer;
    DXDescriptorHeap m_dsvHeap;

    DXDescriptorHeap m_cbvHeap;

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;

    RECT wr;
    HWND hwnd;

    std::unique_ptr<PMDModel> model;
    std::unique_ptr<PMDRenderer> renderer; 

    SceneMatrix* m_constantBufferMap;
    float angle = 0.0f;
};

