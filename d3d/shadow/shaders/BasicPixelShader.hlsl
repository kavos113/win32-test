#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
    if (input.instance == 1)
    {
        return float4(0, 0, 0, 1);
    }

    float3 light = normalize(float3(1, -1, 1));
    float3 lightColor = float3(1, 1, 1);

    float brightness = saturate(dot(-light, input.normal));

    float3 refLight = normalize(reflect(light, input.normal.xyz));
    float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);

    float4 toonDiffuse = toon.Sample(sam, float2(0, 1.0 - brightness));

    float2 sphereMapUV = input.normal.xy;
    sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);

    float4 color = tex.Sample(sam, input.uv);

    return max(saturate(toonDiffuse
        * diffuse
        * color
        * sph.Sample(sam, sphereMapUV))
        + saturate(spa.Sample(sam, sphereMapUV) * color
        + float4(specularB * specular))
        , float4(color * ambient, 1)
    );
}