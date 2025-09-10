/**********************************************************************************
 * shader.hlsl
 *
 *
 *
 *
 * LI WENHUI
 * 2025/09/10
 **********************************************************************************/

// 常量缓冲（b0）
cbuffer PerObjectCB : register(b0)
{
    float4x4 world; // 世界矩阵
    float4x4 view; // 视图矩阵
    float4x4 projection; // 投影矩阵
    float4x4 worldIT; // 世界矩阵的逆转置（Inverse-Transpose），用于法线/切线
    float4 tintColor; // 染色/透明度
};

// 常量缓冲（b1）
cbuffer PerFrameCB : register(b1)
{
    float3 lightDirWS;
    float _pad0; 
    float4 ambientColor; // 环境光颜色/强度
    float4 lightColor; // 主光颜色/强度
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
    float4x4 worldViewProj = mul(mul(world, view), projection);
    output.position = mul(localPos, worldViewProj);

    // 用 worldIT 的上3x3来变换法线/切线（支持非均匀缩放/镜像）
    float3x3 normalMat = (float3x3) worldIT;
    output.worldNormal = normalize(mul(input.normal, normalMat));
    output.worldTangent = normalize(mul(input.tangent, normalMat));

    output.texCoord = input.texCoord;
    output.color = input.color;

    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    // 采样基础颜色
    float4 baseColor = albedoTexture.Sample(defaultSampler, input.texCoord);

    // 世界空间法线与光照方向（按约定：lightDirWS 为光线前进方向，所以漫反射用 N·(-L)）
    float3 normalWS = normalize(input.worldNormal);
    float3 lightDirWSNorm = normalize(lightDirWS);
    float diffuseFactor = saturate(dot(normalWS, -lightDirWSNorm));

    // 环境 + 漫反射（按颜色调制）
    float3 lightingRGB = ambientColor.rgb + diffuseFactor * lightColor.rgb;

    // 最终颜色：纹理 × 顶点色 × 光照 × 对象tint
    float4 finalColor = baseColor * input.color * float4(lightingRGB, 1.0f) * tintColor;

    //return finalColor;
    
    return input.color;
}
