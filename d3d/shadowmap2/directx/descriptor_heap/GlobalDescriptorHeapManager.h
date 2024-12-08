#pragma once
#include <array>

#include "GlobalDescriptorHeap.h"
#include "ShaderGlobalDescriptorHeap.h"

class GlobalDescriptorHeapManager
{
public:
    static void Init();

    static DescriptorHeapSegmentManager& CreateShaderManager(const std::string& name, unsigned int size, D3D12_DESCRIPTOR_HEAP_TYPE type);
    static DescriptorHeapSegmentManager& GetCPUHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type);
    static DescriptorHeapSegmentManager& GetShaderHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type, const std::string& name);

    static void SetToCommand();
private:
    static std::array<ShaderGlobalDescriptorHeap, 2> m_shaderHeaps;
    static std::array<GlobalDescriptorHeap, 2> m_heaps;
};
