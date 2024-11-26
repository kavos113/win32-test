#pragma once
#include "DXBuffer.h"

template <typename T>
class ConstantBuffer :
    public DXBuffer
{
public:
    HRESULT CreateBuffer() override;
    void CreateView() override;

private:
    T* mappedBuffer;
};

