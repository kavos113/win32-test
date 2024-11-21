#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <Windows.h>

class DXWorld
{
    struct SceneMatrix
    {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMFLOAT3 eye;
    };

public:
    DXWorld()
        : matrix_buffer_map_(nullptr),
        cbv_heap_(nullptr),
        aspect_ratio_(0.0f)
    {
        
    }

    HRESULT SetMatrixBuffer();

    void SetAspectRatio(float aspect_ratio) { aspect_ratio_ = aspect_ratio; }
    void SetDescriptorHeap() const;

private:
    SceneMatrix* matrix_buffer_map_;
    ID3D12DescriptorHeap* cbv_heap_;

    float aspect_ratio_;
};

