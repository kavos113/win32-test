Texture2D tex : register(t0);
Texture2D sph : register(t1);
Texture2D spa : register(t2);
Texture2D toon : register(t3);
SamplerState sam : register(s0);
SamplerState samToon : register(s1);

Texture2D<float> shadowMap : register(t4);

cbuffer SceneMatrix : register(b0)
{
    matrix view;
    matrix projection;
    matrix lightView;
    matrix shadow;
    float3 eye;
}

cbuffer Transform : register(b1)
{
    matrix world;
    matrix bones[256];
}

cbuffer Material : register(b2)
{
    float4 diffuse;
    float4 specular;
    float3 ambient;
}

struct Output
{
    float4 svpos : SV_POSITION;
    float4 pos : POSITION;
    float4 normal : NORMAL0;
    float4 vnormal : NORMAL1;
    float2 uv : TEXCOORD;
    float3 ray : VECTOR;
    float4 shadowPos : SHADOWPOS;
};