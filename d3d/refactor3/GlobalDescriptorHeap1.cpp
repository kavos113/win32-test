#include "GlobalDescriptorHeap1.h"

#include <tchar.h>

#include "DXCommand.h"

void GlobalDescriptorHeap1::Init()
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};

    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = kMaxDescriptorHeapSize;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;

    HRESULT hr = m_heap_.CreateDescriptorHeap(&desc);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create descriptor heap\n"));
    }
}

GLOBAL_HEAP_ID GlobalDescriptorHeap1::Allocate(unsigned int num_descriptor)
{
    sizes_.push_back(num_descriptor);

    if (last_id_ == 0)
    {
        offsets_.push_back(0);
    }
    else
    {
        offsets_.push_back(offsets_[last_id_ - 1] + sizes_[last_id_ - 1]);
    }

    GLOBAL_HEAP_ID id = last_id_;
    last_id_++;

    return id;
}

D3D12_CPU_DESCRIPTOR_HANDLE GlobalDescriptorHeap1::GetCPUHandle(GLOBAL_HEAP_ID id) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_heap_.GetCPUHandle();
    handle.ptr += static_cast<UINT64>(offsets_[id]) * m_heap_.GetIncrementSize(); 

    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE GlobalDescriptorHeap1::GetGPUHandle(GLOBAL_HEAP_ID id) const
{
    D3D12_GPU_DESCRIPTOR_HANDLE handle = m_heap_.GetGPUHandle();
    handle.ptr += static_cast<UINT64>(offsets_[id]) * m_heap_.GetIncrementSize();

    return handle;
}

unsigned int GlobalDescriptorHeap1::GetSize(GLOBAL_HEAP_ID id) const
{
    return sizes_[id];
}

UINT GlobalDescriptorHeap1::GetIncrementSize() const
{
    return m_heap_.GetIncrementSize();
}

std::pair<D3D12_ROOT_PARAMETER*, size_t> GlobalDescriptorHeap1::GetRootParameters() const
{
    D3D12_ROOT_PARAMETER* root_parameters = new D3D12_ROOT_PARAMETER[root_parameters_.size()];
    for (size_t i = 0; i < root_parameters_.size(); i++)
    {
        root_parameters[i] = root_parameters_[i];
    }

    return std::make_pair(root_parameters, root_parameters_.size());
}

void GlobalDescriptorHeap1::SetRootParameter(
    GLOBAL_HEAP_ID id,
    D3D12_ROOT_PARAMETER_TYPE type,
    D3D12_SHADER_VISIBILITY visibility,
    const D3D12_DESCRIPTOR_RANGE* descriptor_ranges,
    int num_descriptor_ranges
)
{
    D3D12_ROOT_PARAMETER root_parameter = {};

    root_parameter.ParameterType = type;
    root_parameter.ShaderVisibility = visibility;
    root_parameter.DescriptorTable.NumDescriptorRanges = num_descriptor_ranges;
    root_parameter.DescriptorTable.pDescriptorRanges = descriptor_ranges;

    root_parameters_.push_back(root_parameter);
}

void GlobalDescriptorHeap1::SetGraphicsRootDescriptorTable(GLOBAL_HEAP_ID id) const
{
    D3D12_GPU_DESCRIPTOR_HANDLE handle = GetGPUHandle(id);
    DXCommand::GetCommandList()->SetGraphicsRootDescriptorTable(
        id,
        handle
    );
}

void GlobalDescriptorHeap1::SetToCommand() const
{
    m_heap_.SetToCommand();
}