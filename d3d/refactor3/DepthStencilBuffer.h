#pragma once
#include "DXBuffer.h"
class DepthStencilBuffer :
    public DXBuffer
{
public:
    HRESULT CreateBuffer() override;

    DepthStencilBuffer(RECT wr)
        :
        wr(wr)
    {
    }

private:
    RECT wr;
};

