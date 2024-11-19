#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
    float3 light = normalize(float3(1, -1, 1));
    float brightness = dot(-light, input.normal);

    float3 refLight = normalize(reflect(light, input.normal.xyz));
    float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);

    float4 toonDiffuse = toon.Sample(sam, float2(0, 1.0 - brightness));

    float2 sphereMapUV = input.normal.xy;
    sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);

    float4 color = tex.Sample(sam, input.uv);

	return float4(brightness, brightness, brightness, 1)
        * diffuse
        * toonDiffuse
        * color
        * sph.Sample(sam, sphereMapUV)
        + spa.Sample(sam, sphereMapUV)
        + float4(color * ambient, 1)
        + float4(specular.rgb * specularB, 1);
}