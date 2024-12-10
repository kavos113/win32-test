#include "DisplayMatrix.h"

using DirectX::operator*;
using DirectX::operator+;

HRESULT DisplayMatrix::Init(DescriptorHeapSegmentManager& model_heap)
{
    m_modelHeap = &model_heap;

    const HRESULT hr = SetMatrixBuffer();
    if (FAILED(hr)) return E_FAIL;

    return S_OK;
}

void DisplayMatrix::Render() const
{
    m_modelHeap->SetGraphicsRootDescriptorTable(m_segment.GetID());
}

HRESULT DisplayMatrix::SetMatrixBuffer()
{
    constexpr DirectX::XMFLOAT3 eye(0.0f, 15.0f, -15.0f);
    constexpr DirectX::XMFLOAT3 focus(0.0f, 10.0f, 0.0f);
    constexpr DirectX::XMFLOAT3 up(0.0f, 1.0f, 0.0f);

    const DirectX::XMVECTOR eye_position = DirectX::XMLoadFloat3(&eye);
    const DirectX::XMVECTOR focus_position = DirectX::XMLoadFloat3(&focus);
    const DirectX::XMVECTOR up_vec = DirectX::XMLoadFloat3(&up);

    const DirectX::XMMATRIX view_matrix = DirectX::XMMatrixLookAtLH(eye_position, focus_position, up_vec);

    const DirectX::XMMATRIX projection_matrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV2,
        static_cast<float>(wr.right - wr.left) / static_cast<float>((wr.bottom - wr.top)),
        1.0f,
        100.0f
    );

    auto plane = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR plane_vec = DirectX::XMLoadFloat4(&plane);
    auto light = DirectX::XMFLOAT4(-1.0f, 1.0f, -1.0f, 0.0f);
    DirectX::XMVECTOR light_vec = DirectX::XMLoadFloat4(&light);

    DirectX::XMVECTOR light_pos = focus_position + DirectX::XMVector3Normalize(light_vec) * DirectX::XMVector3Length(DirectX::XMVectorSubtract(focus_position, eye_position));

    m_matrixBuffer.SetResourceWidth((sizeof(SceneMatrix) + 0xff) & ~0xff);
    const HRESULT hr = m_matrixBuffer.CreateBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[DisplayMatrix.cpp] Failed to create matrix buffer\n"));
        return hr;
    }

    m_matrixBuffer.GetMappedBuffer()->view = view_matrix;
    m_matrixBuffer.GetMappedBuffer()->proj = projection_matrix;
    m_matrixBuffer.GetMappedBuffer()->eye = eye;

    m_matrixBuffer.GetMappedBuffer()->lightView = DirectX::XMMatrixLookAtLH(light_pos, focus_position, up_vec)
        * DirectX::XMMatrixOrthographicLH(40.0f, 40.0f, 1.0f, 100.0f);

    m_segment = m_modelHeap->Allocate(1);

    m_matrixBuffer.SetSegment(m_segment);
    m_matrixBuffer.CreateView();

    D3D12_DESCRIPTOR_RANGE* range = new D3D12_DESCRIPTOR_RANGE();

    range->NumDescriptors = 1;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    range->BaseShaderRegister = 0;
    range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    range->RegisterSpace = 0;

    m_modelHeap->SetRootParameter(
        m_segment.GetID(),
        D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
        D3D12_SHADER_VISIBILITY_ALL,
        range,
        1
    );

    return S_OK;
}
