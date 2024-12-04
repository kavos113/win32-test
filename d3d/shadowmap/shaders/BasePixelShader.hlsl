#include "BaseHeader.hlsli"

float4 ps(Output input) : SV_TARGET
{
    float4 ret = tex.Sample(sam, input.uv);

    // grayscale
    // float y = dot(ret.rgb, float3(0.299, 0.587, 0.114));
    // return float4(y, y, y, 1);

    // inverse
    // return float4(1.0f - ret.rgb, ret.a);

    // low tone
    // return float4(ret.rgb - fmod(ret.rgb, 0.25f), ret.a);
    
    float w, h, levels;
    tex.GetDimensions(0, w, h, levels);
    
    float dx = 1.0f / w;
    float dy = 1.0f / h;

    float4 ret2 = float4(0, 0, 0, 0);

    // average
    // ret2 += tex.Sample(sam, input.uv + float2(-2 * dx, -2 * dy));
    // ret2 += tex.Sample(sam, input.uv + float2(      0, -2 * dy));
    // ret2 += tex.Sample(sam, input.uv + float2( 2 * dx, -2 * dy));
    // ret2 += tex.Sample(sam, input.uv + float2(-2 * dx,       0));
    // ret2 += tex.Sample(sam, input.uv + float2(      0,       0));
    // ret2 += tex.Sample(sam, input.uv + float2( 2 * dx,       0));
    // ret2 += tex.Sample(sam, input.uv + float2(-2 * dx,  2 * dy));
    // ret2 += tex.Sample(sam, input.uv + float2(      0,  2 * dy));
    // ret2 += tex.Sample(sam, input.uv + float2( 2 * dx,  2 * dy));
    //
    // ret2 /= 9;

    // emboss
    // ret2 += tex.Sample(sam, input.uv + float2(-2 * dx, -2 * dy)) * 2;
    // ret2 += tex.Sample(sam, input.uv + float2(0, -2 * dy));
    // ret2 += tex.Sample(sam, input.uv + float2(2 * dx, -2 * dy)) * 0;
    // ret2 += tex.Sample(sam, input.uv + float2(-2 * dx, 0));
    // ret2 += tex.Sample(sam, input.uv + float2(0, 0));
    // ret2 += tex.Sample(sam, input.uv + float2(2 * dx, 0)) * -1;
    // ret2 += tex.Sample(sam, input.uv + float2(-2 * dx, 2 * dy));
    // ret2 += tex.Sample(sam, input.uv + float2(0, 2 * dy)) * -1;
    // ret2 += tex.Sample(sam, input.uv + float2(2 * dx, 2 * dy)) * -2;
    //
    // float y = dot(ret2.rgb, float3(0.299, 0.587, 0.114));
    // ret2 = float4(y, y, y, 1);

    // sharpness
    // ret2 += tex.Sample(sam, input.uv + float2(-2 * dx, -2 * dy)) * 0;
    // ret2 += tex.Sample(sam, input.uv + float2(      0, -2 * dy)) * -1;
    // ret2 += tex.Sample(sam, input.uv + float2( 2 * dx, -2 * dy)) * 0;
    // ret2 += tex.Sample(sam, input.uv + float2(-2 * dx,       0)) * -1;
    // ret2 += tex.Sample(sam, input.uv + float2(      0,       0)) * 5;
    // ret2 += tex.Sample(sam, input.uv + float2( 2 * dx,       0)) * -1;
    // ret2 += tex.Sample(sam, input.uv + float2(-2 * dx,  2 * dy)) * 0;
    // ret2 += tex.Sample(sam, input.uv + float2(      0,  2 * dy)) * -1;
    // ret2 += tex.Sample(sam, input.uv + float2( 2 * dx,  2 * dy)) * 0;

    // edge
    // ret2 += tex.Sample(sam, input.uv + float2(-2 * dx, -2 * dy)) * 0;
    // ret2 += tex.Sample(sam, input.uv + float2(0, -2 * dy)) * -1;
    // ret2 += tex.Sample(sam, input.uv + float2(2 * dx, -2 * dy)) * 0;
    // ret2 += tex.Sample(sam, input.uv + float2(-2 * dx, 0)) * -1;
    // ret2 += tex.Sample(sam, input.uv + float2(0, 0)) * 4;
    // ret2 += tex.Sample(sam, input.uv + float2(2 * dx, 0)) * -1;
    // ret2 += tex.Sample(sam, input.uv + float2(-2 * dx, 2 * dy)) * 0;
    // ret2 += tex.Sample(sam, input.uv + float2(0, 2 * dy)) * -1;
    // ret2 += tex.Sample(sam, input.uv + float2(2 * dx, 2 * dy)) * 0;
    //
    // float y = dot(ret2.rgb, float3(0.299, 0.587, 0.114));
    // //y = pow(1.0f - y, 5.0f);
    // y = pow(y, 10.0f);
    // y = 1.0f - y;
    // y = step(0.2f, y);
    // ret2 = float4(y, y, y, 1);

    ret2 += weights[0] * ret;

    for (int i = 0; i < 8; i++)
    {
        ret2 += weights[i >> 2][i % 4] * tex.Sample(sam, input.uv + float2(i * dx, 0));
        ret2 += weights[i >> 2][i % 4] * tex.Sample(sam, input.uv + float2(-i * dx, 0));
    }

    return float4(ret2.rgb, ret.a);
}

float4 vertical(Output input) : SV_TARGET
{
    float w, h, levels;
    tex.GetDimensions(0, w, h, levels);

    float dy = 1.0f / h;

    float4 ret2 = float4(0, 0, 0, 0);

    ret2 += weights[0] * tex.Sample(sam, input.uv);

    for (int i = 0; i < 8; i++)
    {
        ret2 += weights[i >> 2][i % 4] * tex.Sample(sam, input.uv + float2(0, i * dy));
        ret2 += weights[i >> 2][i % 4] * tex.Sample(sam, input.uv + float2(0, -i * dy));
    }

    return float4(ret2.rgb, ret2.a);
}

float4 normal(Output input) : SV_TARGET
{
    return tex.Sample(sam, input.uv);
    // float d = depthTex.Sample(sam, input.uv);
    // d = pow(d, 20);
    // return float4(d, d, d, 1);
}