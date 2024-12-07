#pragma once
#include <d3d12.h>
#include <unordered_map>

#include "DescriptorHeapSegmentManager.h"
#include "GlobalDescriptorHeap.h"

class ShaderGlobalDescriptorHeap : public GlobalDescriptorHeap
{
public:
    void Init(D3D12_DESCRIPTOR_HEAP_TYPE type) override;
    ShaderGlobalDescriptorHeap();
    ~ShaderGlobalDescriptorHeap() override = default;

    DescriptorHeapSegmentManager& GetHeapManager(const std::string &name);
    void CreateManager(const std::string &name, unsigned int size);

    void SetToCommand() const;

private:
    std::unordered_map<std::string, DescriptorHeapSegmentManager> m_heapManagers;

    D3D12_GPU_DESCRIPTOR_HANDLE m_lastGpuHandle;

    constexpr static unsigned int kMaxDescriptorHeapSize = 131072;
    constexpr static unsigned int kMaxDescriptorHeapSizeSampler = 2048;
};
