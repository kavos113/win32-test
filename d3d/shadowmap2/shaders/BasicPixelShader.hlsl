#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
    float3 eyeray = normalize(input.pos - eye);
    float3 light = normalize(float3(1, -1, 1));
    float3 rlight = reflect(light, input.normal);

    float p = saturate(dot(rlight, -eyeray));

    float diffB = dot(-light, input.normal);

    float4 toonCol = toon.Sample(samToon, float2(0, 1 - diffB));

    float4 texCol = tex.Sample(sam, input.uv);

    float2 spUV = (input.normal.xy * float2(1, -1) + float2(1, 1)) / 2;
    float4 sphCol = sph.Sample(sam, spUV);
    float4 spaCol = spa.Sample(sam, spUV);

    float4 ret = float4((spaCol + sphCol * texCol * toonCol * diffuse).rgb, diffuse.a) + float4(specular * p);

    float shadowWeight = 1.0f;
    float3 posFromLight = input.shadowPos.xyz / input.shadowPos.w;
    float2 shadowUV = (posFromLight + float2(1, -1)) * float2(0.5, -0.5);
    float shadowDepth = shadowMap.Sample(sam, shadowUV);
    if (shadowDepth < posFromLight.z)
    {
        shadowWeight = 0.5f;
    }

    return float4(ret.rgb * shadowWeight, ret.a);
}