#include "DXFence.h"

ID3D12Fence* DXFence::m_fence = nullptr;
int DXFence::m_fenceValue = 0;