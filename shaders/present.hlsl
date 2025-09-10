/**********************************************************************************
 * present.hlsl
 *
 *
 *
 *
 * LI WENHUI
 * 2025/09/10
 **********************************************************************************/

Texture2D CanvasTex : register(t0);
SamplerState SamplerClamp : register(s0);

struct VSIn
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD;
};

struct VSOUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD;
};

VSOUT VSQuad(VSIn vin)
{
    VSOUT o;
    o.position = float4(vin.position, 1.0);
    o.color = vin.color;
    o.texCoord = vin.texCoord;
    return o;
}

float4 PSPresent(VSOUT pin) : SV_TARGET
{
    return CanvasTex.Sample(SamplerClamp, pin.texCoord);
}