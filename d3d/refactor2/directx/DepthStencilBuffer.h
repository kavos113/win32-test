#pragma once
#include "BufferView.h"
class DepthStencilBuffer :
    public BufferView
{
public:
    HRESULT Create();

    DepthStencilBuffer(RECT wr)
        :
        wr(wr) 
    {
        m_buffer_ = nullptr;
    }

private:
    RECT wr;
};

