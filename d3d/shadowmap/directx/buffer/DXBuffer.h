#pragma once

#include <d3d12.h>
#include <Windows.h>


class DXBuffer
{
public:
    virtual HRESULT CreateBuffer() = 0;
    virtual void CreateView() = 0;
    ID3D12Resource* GetBuffer() const { return m_buffer; }

    DXBuffer()
        :
        m_buffer(nullptr)
    {
    }

    virtual ~DXBuffer() = default;

protected:
    ID3D12Resource* m_buffer;
};

