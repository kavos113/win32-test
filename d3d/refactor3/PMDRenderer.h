#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <Windows.h>

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
    PMDRenderer(HWND hwnd, RECT wr)
        :
        m_pipelineState(nullptr),
        m_rootSignature(nullptr),
        m_vsBlob(nullptr),
        m_psBlob(nullptr),
        hwnd(hwnd),
        wr(wr)
    {
    }

    HRESULT Init();
    void SetPipelineState() const;
    void SetRootSignature() const;
private:
    HRESULT CompileShaders();

    HRESULT SetGraphicsPipeline();
    HRESULT CreateRootSignature();

    ID3D12PipelineState* m_pipelineState;
    ID3D12RootSignature* m_rootSignature;

    ID3D10Blob* m_vsBlob;
    ID3D10Blob* m_psBlob;

    HWND hwnd;
    RECT wr;
};

