/**********************************************************************************
 * shader.hlsl
 *
 *
 *
 *
 * LI WENHUI
 * 2025/09/10
 **********************************************************************************/

cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    
    float4 tintColor;
    float3 lightDir;
};

Texture2D albedoTexture : register(t0);
SamplerState defaultSampler : register(s0);

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD;
    float3 worldNormal : NORMAL;
    float3 worldTangent : TANGENT;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;

    // 计算裁剪空间位置
    float4 localPos = float4(input.position, 1.0f);
    matrix worldViewProj = mul(mul(world, view), projection);
    output.position = mul(localPos, worldViewProj);
    
    // 将法线和切线变换到世界空间
    output.worldNormal = normalize(mul(input.normal, (float3x3) world));
    output.worldTangent = normalize(mul(input.tangent, (float3x3) world));
    
    // 传递其他数据
    output.texCoord = input.texCoord;
    output.color = input.color;
    
    return output;
}


float4 PSMain(PS_INPUT input) : SV_TARGET
{
    
    // 采样基础颜色
    float4 albedo = albedoTexture.Sample(defaultSampler, input.texCoord);
    
    // 准备光照向量
    float3 normal = normalize(input.worldNormal);
    float3 lightDirection = normalize(lightDir);
    
    // 计算光照强度
    float lightIntensity = saturate(dot(normal, -lightDirection));
    float ambient = 0.1f;
    lightIntensity = ambient + lightIntensity;

    // 最终颜色
    float4 finalColor = albedo * input.color * lightIntensity * tintColor;
    
    
    return finalColor;
}