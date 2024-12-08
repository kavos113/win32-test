#include "BaseHeader.hlsli"

float4 ps(Output input) : SV_TARGET
{
    return tex.Sample(sam, input.uv);
}