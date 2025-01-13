struct SimpleBuffer
{
    int i;
    float f;
};

StructuredBuffer<SimpleBuffer> input : register(t0);

struct m_Buffer
{
    float groupId;
    float groupThreadId;
    float dispatchThreadId;
    uint groupIndex;
};
RWStructuredBuffer<m_Buffer> output : register(u0);

[numthreads(4, 4, 4)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    output[dtid.x * 8 * 8 + dtid.y * 8 + dtid.z].dispatchThreadId = dtid.x * 8 * 8 + dtid.y * 8 + dtid.z;
}
