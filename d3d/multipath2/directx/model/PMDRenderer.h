#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <Windows.h>

#include "directx/descriptor_heap/DescriptorHeapSegmentManager.h"

class PMDRenderer
{
    struct SceneMatrix
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;

        DirectX::XMFLOAT3 eye;
    };
public:
    PMDRenderer(HWND hwnd, RECT wr, const DescriptorHeapSegmentManager& model_manager)
        :
        m_pipelineState(nullptr),
        m_rootSignature(nullptr),
        m_vsBlob(nullptr),
        m_psBlob(nullptr),
        m_modelManager(model_manager),
        hwnd(hwnd),
        wr(wr)
    {
    }

    HRESULT Init();
    void SetPipelineState() const;
    void SetRootSignature() const;
private:
    HRESULT CompileShaders();

    HRESULT CreateGraphicsPipeline();
    HRESULT CreateRootSignature();

    ID3D12PipelineState* m_pipelineState;
    ID3D12RootSignature* m_rootSignature;

    ID3D10Blob* m_vsBlob;
    ID3D10Blob* m_psBlob;

    const DescriptorHeapSegmentManager& m_modelManager;

    HWND hwnd;
    RECT wr;
};

