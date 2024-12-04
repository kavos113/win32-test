#include "BasicShaderHeader.hlsli"

Output BasicVS( 
    float4 pos : POSITION,
    float4 normal : NORMAL,
    float2 uv : TEXCOORD,
    min16uint2 bone_number : BONE_NUMBER,
    min16uint weight : WEIGHT,
    uint instance : SV_InstanceID
)
{
    float w = weight / 100.0f;
    matrix bone_matrix = bones[bone_number[0]] * w + bones[bone_number[1]] * (1.0f - w);

    Output output;

    output.pos = mul(world, mul(bone_matrix, pos));
    output.instance = instance;
    output.svpos = mul(projection, mul(view, output.pos));
    output.tpos = mul(lightCamera, output.pos);
    output.uv = uv;
    normal.w = 0;
    output.normal = mul(world, normal);
    output.vnormal = mul(view, output.normal);
    output.ray = normalize(output.pos.xyz - mul(view, eye));

    return output;
}

float4 shadowVS(
    float4 pos : POSITION,
    float4 normal : NORMAL,
    float2 uv : TEXCOORD,
    min16uint2 bone_number : BONE_NUMBER,
    min16uint weight : WEIGHT
) : SV_POSITION
{
    float w = float(weight) / 100.0f;

    matrix bone_matrix = bones[bone_number[0]] * w + bones[bone_number[1]] * (1.0f - w);

    pos = mul(world, mul(bone_matrix, pos));

    return mul(lightCamera, pos);
}