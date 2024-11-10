#include "BasicShaderHeader.hlsli"

Texture2D<float4> tex : register(t0);
SamplerState sam : register(s0);

float4 BasicPS(Output input) : SV_TARGET
{
	return float4(tex.Sample(sam, input.uv));
}