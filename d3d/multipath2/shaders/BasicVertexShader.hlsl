#include "BasicShaderHeader.hlsli"

Output BasicVS( 
    float4 pos : POSITION,
    float4 normal : NORMAL,
    float2 uv : TEXCOORD,
    min16uint2 bone_number : BONE_NUMBER,
    min16uint weight : WEIGHT
)
{
    float w = weight / 100.0f;
    matrix bone_matrix = bones[bone_number[0]] * w + bones[bone_number[1]] * (1.0f - w);

    pos = mul(bone_matrix, pos);
    pos = mul(world, pos);

    Output output;
    output.svpos = mul(mul(projection, view), pos);
    normal.w = 0;
    output.normal = mul(world, normal);
    output.vnormal = mul(view, output.normal);
    output.pos = mul(view, pos);
    output.uv = uv;
    output.ray = normalize(pos.xyz - mul(view, eye));

    return output;
}