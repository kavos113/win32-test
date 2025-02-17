#pragma once
#include <vector>

#include "DXDescriptorHeap.h"

typedef int GLOBAL_HEAP_ID;

// GlobalDescriptorHeap for CBV_SRV_UAV Engine�Ɏ����Ă�����������������ȁH
class GlobalDescriptorHeap1
{
public:
    void Init();
    GLOBAL_HEAP_ID Allocate(unsigned int num_descriptor);

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(GLOBAL_HEAP_ID id) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(GLOBAL_HEAP_ID id) const;
    unsigned int GetSize(GLOBAL_HEAP_ID id) const;
    UINT GetIncrementSize() const;
    std::pair<D3D12_ROOT_PARAMETER*, size_t> GetRootParameters() const;

    void SetRootParameter(
        GLOBAL_HEAP_ID id,
        D3D12_ROOT_PARAMETER_TYPE type,
        D3D12_SHADER_VISIBILITY visibility,
        const D3D12_DESCRIPTOR_RANGE* descriptor_ranges,
        int num_descriptor_ranges
    );

    void SetGraphicsRootDescriptorTable(GLOBAL_HEAP_ID id) const;
    void SetGraphicsRootDescriptorTable(GLOBAL_HEAP_ID id, unsigned int offset) const;
    void SetToCommand() const;
private:
    DXDescriptorHeap m_heap_;

    GLOBAL_HEAP_ID last_id_ = 0; // ���Ɋ��蓖�Ă�ID
    std::vector<unsigned int> sizes_; // �eID�Ɋ��蓖�Ă�ꂽ�T�C�Y sizes_[id]�ł킩��
    std::vector<unsigned int> offsets_; // �eID�Ɋ��蓖�Ă�ꂽ�I�t�Z�b�g offsets_[id]�ł킩��
    std::vector<D3D12_ROOT_PARAMETER> root_parameters_; // �eID�Ɋ��蓖�Ă�ꂽRootParameter root_parameter�̐������Ȃ������͂���ł��s����

    constexpr static unsigned int kMaxDescriptorHeapSize = 65536;
};

