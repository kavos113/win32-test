#include <windows.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <tchar.h>
#include <vector>
#include <array>
#include <print>
#include <random>

// input: srv -> output: uav

namespace
{
    void EnableDebugLayer()
    {
        ID3D12Debug5* debugLayer = nullptr;
        HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
        if (SUCCEEDED(hr))
        {
            debugLayer->EnableDebugLayer();
            debugLayer->SetEnableGPUBasedValidation(TRUE);
            debugLayer->SetEnableAutoName(TRUE);
            debugLayer->Release();
        }
    }
}

// for input
struct SimpleBuffer
{
    int i;
    float f;
};

// for output
struct ID
{
    float groupID;
    float groupThreadID;
    float dispatchThreadID;
    unsigned int groupIndex;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    EnableDebugLayer();

    ID3D12Device* device = nullptr;
    ID3D12CommandQueue* commandQueue = nullptr;
    ID3D12CommandAllocator* commandAllocator = nullptr;
    ID3D12GraphicsCommandList* commandList = nullptr;
    ID3D12DescriptorHeap* descriptorHeap = nullptr;
    ID3D12Fence* fence = nullptr;
    UINT64 fenceValue = 0;
    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* pipelineState = nullptr;

    ID3D12Resource* inputBuffer = nullptr;
    ID3D12Resource* outputBuffer = nullptr;
    SimpleBuffer* inputBufferMapped = nullptr;

    // data
    std::vector<ID> uavData(2 * 2 * 2 * 4 * 4 * 4);
    std::array<SimpleBuffer, 64> inputData;

    std::random_device seed;
    std::mt19937_64 mt(seed());
    std::uniform_int_distribution<int> dist(0, 100);
    std::uniform_real_distribution<float> distf(0.0f, 1.0f);
    for (auto& d : inputData)
    {
        d.i = dist(mt);
        d.f = distf(mt);
    }

    HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create D3D12 device.\n"));
        return 1;
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {
        .Type = D3D12_COMMAND_LIST_TYPE_COMPUTE, // daijidayo!!!!!!!!!!
        .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0,
    };
    hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create command queue.\n"));
        return 1;
    }

    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&commandAllocator));
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create command allocator.\n"));
        return 1;
    }

    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create command list.\n"));
        return 1;
    }

    hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create fence.\n"));
        return 1;
    }

    // desc heap
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
        .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        .NumDescriptors = 10,
        .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
        .NodeMask = 0,
    };
    hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create descriptor heap.\n"));
        return 1;
    }

    // buffer
    D3D12_HEAP_PROPERTIES heap_prop = {
        .Type = D3D12_HEAP_TYPE_UPLOAD,
        .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
        .CreationNodeMask = 0,
        .VisibleNodeMask = 0,
    };
    D3D12_RESOURCE_DESC resource_desc = {
        .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
        .Alignment = 0,
        .Width = sizeof(inputData[0]) * inputData.size(),
        .Height = 1,
        .DepthOrArraySize = 1,
        .MipLevels = 1,
        .Format = DXGI_FORMAT_UNKNOWN,
        .SampleDesc = {
            .Count = 1,
            .Quality = 0,
        },
        .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        .Flags = D3D12_RESOURCE_FLAG_NONE,
    };
    hr = device->CreateCommittedResource(
        &heap_prop,
        D3D12_HEAP_FLAG_NONE,
        &resource_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&inputBuffer)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create input buffer.\n"));
        return 1;
    }

    D3D12_HEAP_PROPERTIES heap_prop_uav = {
        .Type = D3D12_HEAP_TYPE_DEFAULT,
        .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
        .CreationNodeMask = 0,
        .VisibleNodeMask = 0,
    };
    D3D12_RESOURCE_DESC resource_desc_uav = {
        .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
        .Alignment = 0,
        .Width = sizeof(uavData[0]) * uavData.size(),
        .Height = 1,
        .DepthOrArraySize = 1,
        .MipLevels = 1,
        .Format = DXGI_FORMAT_UNKNOWN,
        .SampleDesc = {
            .Count = 1,
            .Quality = 0,
        },
        .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        .Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
    };
    hr = device->CreateCommittedResource(
        &heap_prop_uav,
        D3D12_HEAP_FLAG_NONE,
        &resource_desc_uav,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(&outputBuffer)
    );

    inputBuffer->Map(0, nullptr, reinterpret_cast<void**>(&inputBufferMapped));
    std::ranges::copy(inputData, inputBufferMapped);
    inputBuffer->Unmap(0, nullptr);

    // view
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {
        .Format = DXGI_FORMAT_UNKNOWN,
        .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        .Buffer = {
            .FirstElement = 0,
            .NumElements = inputData.size(),
            .StructureByteStride = sizeof(inputData[0]),
            .Flags = D3D12_BUFFER_SRV_FLAG_NONE,
        },
    };
    device->CreateShaderResourceView(inputBuffer, &srv_desc, descriptorHeap->GetCPUDescriptorHandleForHeapStart());

    D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {
        .Format = DXGI_FORMAT_UNKNOWN,
        .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
        .Buffer = {
            .FirstElement = 0,
            .NumElements = static_cast<UINT>(uavData.size()),
            .StructureByteStride = sizeof(uavData[0]),
            .CounterOffsetInBytes = 0,
            .Flags = D3D12_BUFFER_UAV_FLAG_NONE,
        }
    };
    auto handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    device->CreateUnorderedAccessView(outputBuffer, nullptr, &uav_desc, handle);

    // root signature
    ID3DBlob* errorBlob = nullptr;
    D3D12_DESCRIPTOR_RANGE range[2] = {};
    range[0].NumDescriptors = 1;
    range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range[0].BaseShaderRegister = 0;
    range[0].OffsetInDescriptorsFromTableStart = 0;
    range[0].RegisterSpace = 0;
    range[1].NumDescriptors = 1;
    range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    range[1].BaseShaderRegister = 0;
    range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    range[1].RegisterSpace = 0;

    D3D12_ROOT_PARAMETER rootParam = {
        .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
        .DescriptorTable = {
            .NumDescriptorRanges = 2,
            .pDescriptorRanges = range,
        },
        .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL,
    };

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {
        .NumParameters = 1,
        .pParameters = &rootParam,
        .NumStaticSamplers = 0,
        .pStaticSamplers = nullptr,
        .Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE,
    };
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc = {
        .Version = D3D_ROOT_SIGNATURE_VERSION_1_0,
        .Desc_1_0 = rootSignatureDesc,
    };

    ID3DBlob* rootSignatureBlob = nullptr;
    hr = D3D12SerializeVersionedRootSignature(&versionedRootSignatureDesc, &rootSignatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to serialize root signature.\n"));
        return 1;
    }
    hr = device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create root signature.\n"));
        return 1;
    }

    // pipeline state
    ID3DBlob* csBlob = nullptr;
    hr = D3DCompileFromFile(L"cs.hlsl", nullptr, nullptr, "main", "cs_5_1", 0, 0, &csBlob, &errorBlob);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to compile compute shader.\n"));
        return 1;
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateDesc = {
        .pRootSignature = rootSignature,
        .CS = {
            .pShaderBytecode = csBlob->GetBufferPointer(),
            .BytecodeLength = csBlob->GetBufferSize(),
        },
        .NodeMask = 0,
        .Flags = D3D12_PIPELINE_STATE_FLAG_NONE,
    };
    hr = device->CreateComputePipelineState(&pipelineStateDesc, IID_PPV_ARGS(&pipelineState));
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create pipeline state.\n"));
        return 1;
    }
    csBlob->Release();


    commandList->SetComputeRootSignature(rootSignature);
    commandList->SetDescriptorHeaps(1, &descriptorHeap);
    commandList->SetComputeRootDescriptorTable(0, descriptorHeap->GetGPUDescriptorHandleForHeapStart());
    commandList->SetPipelineState(pipelineState);
    commandList->Dispatch(2, 2, 2);


    // uavをcpuから読む
    ID3D12Resource* copyBuffer = nullptr;
    D3D12_HEAP_PROPERTIES heapProp = {
        .Type = D3D12_HEAP_TYPE_READBACK,
        .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
        .CreationNodeMask = 0,
        .VisibleNodeMask = 0,
    };
    D3D12_RESOURCE_DESC resDesc = {
        .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
        .Alignment = 0,
        .Width = sizeof(uavData[0]) * uavData.size(),
        .Height = 1,
        .DepthOrArraySize = 1,
        .MipLevels = 1,
        .Format = DXGI_FORMAT_UNKNOWN,
        .SampleDesc = {
            .Count = 1,
            .Quality = 0,
        },
        .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        .Flags = D3D12_RESOURCE_FLAG_NONE,
    };
    hr = device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&copyBuffer)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create copy buffer.\n"));
        return 1;
    }

    D3D12_RESOURCE_BARRIER barrier = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition = {
            .pResource = outputBuffer,
            .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            .StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            .StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE,
        },
    };
    commandList->ResourceBarrier(1, &barrier);
    commandList->CopyResource(copyBuffer, outputBuffer);

    hr = commandList->Close();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to close command list.\n"));
        return 1;
    }

    ID3D12CommandList* commandLists[] = { commandList };
    commandQueue->ExecuteCommandLists(1, commandLists);
    commandQueue->Signal(fence, ++fenceValue);
    HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fence->GetCompletedValue() != fenceValue)
    {
        fence->SetEventOnCompletion(fenceValue, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    ID* mappedResult = nullptr;
    D3D12_RANGE ra = {
        .Begin = 0,
        .End = sizeof(uavData[0]) * uavData.size(),
    };
    copyBuffer->Map(0, &ra, reinterpret_cast<void**>(&mappedResult));
    std::ranges::copy_n(mappedResult, uavData.size(), uavData.data());
    copyBuffer->Unmap(0, nullptr);

    inputBuffer->Release();
    outputBuffer->Release();
    copyBuffer->Release();
    rootSignature->Release();
    pipelineState->Release();
    fence->Release();
    descriptorHeap->Release();
    commandList->Release();
    commandAllocator->Release();
    commandQueue->Release();
    device->Release();

    for (auto& d : uavData)
    {
        std::println("dispatchThreadID: {}", d.dispatchThreadID);
    }

    return 0;
}
