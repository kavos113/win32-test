#include "BasicShaderHeader.hlsli"

Output BasicVS( 
    float4 pos : POSITION,
    float4 normal : NORMAL,
    float2 uv : TEXCOORD
)
{
    Output output;
    output.svpos = mul(mul(mul(projection, view), world), pos);
    normal.w = 0;
    output.normal = mul(world, normal);
    output.vnormal = mul(view, output.normal);
    output.pos = mul(view, pos);
    output.uv = uv;
    output.ray = normalize(pos.xyz - mul(view, eye));

    return output;
}