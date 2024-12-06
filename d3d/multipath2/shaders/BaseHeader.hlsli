Texture2D<float4> tex : register(t0);
SamplerState sam : register(s0);

cbuffer PostEffect : register(b0)
{
    float4 weights[2];
}

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};