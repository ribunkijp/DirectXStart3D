/**********************************************************************************
 * player.hlsl
 *
 *
 *
 *
 * LI WENHUI
 * 2025/09/19
 **********************************************************************************/
#pragma pack_matrix(row_major)
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
// 蒙皮
cbuffer SkinningConstants : register(b2)
{
    float4x4 gBoneTransforms[128];
};

Texture2D albedoTexture : register(t0);
SamplerState defaultSampler : register(s0);

struct VS_INPUT
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    int4 boneIDs : BONEIDS;
    float4 weights : WEIGHTS;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 worldNormal : NORMAL;
    float3 worldTangent : TANGENT;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;

    // 计算裁剪空间位置
    float4 localPos = float4(input.position, 1.0f); //扩展成float4齐次坐标
    float3 localNormal = input.normal;
    float3 localTangent = input.tangent;
    
    float4 finalPos = float4(0, 0, 0, 0);
    float3 finalNormal = float3(0, 0, 0);
    float3 finalTangent = float3(0, 0, 0);
    
    float totalWeight = 0.0f;
    for (int i = 0; i < 4; ++i) // 一个顶点最多受到4根骨骼的影响
    {
        if (input.boneIDs[i] >= 0 && input.weights[i] > 0.0)
        {
            float4x4 boneTransform = gBoneTransforms[input.boneIDs[i]];
            float weight = input.weights[i];
            
            finalPos += mul(localPos, boneTransform) * weight;
            finalNormal += mul(localNormal, (float3x3) boneTransform) * weight;
            finalTangent += mul(localTangent, (float3x3) boneTransform) * weight;
            
            totalWeight += weight;
        }
    }
    if (totalWeight < 0.001f)
    {
        finalPos = localPos;
        finalNormal = localNormal;
        finalTangent = localTangent;
    }
    
    float4x4 worldViewProj = mul(mul(world, view), projection); //世界、观察、投影三个矩阵预乘，得到WVP复合矩阵
    output.position = mul(finalPos, worldViewProj);

    // 用 worldIT 的上3x3来变换法线/切线（支持非均匀缩放/镜像）
    float3x3 normalMat = (float3x3) worldIT;
    output.worldNormal = normalize(mul(normalize(finalNormal), normalMat));
    output.worldTangent = normalize(mul(normalize(finalTangent), normalMat));

    output.texCoord = input.texCoord;

    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    // 采样基础颜色
    float4 baseColor = albedoTexture.Sample(defaultSampler, input.texCoord);

    // 计算表面法线和光照方向的反方向之间的夹角 Lambertian Lighting Model
    float3 normalWS = normalize(input.worldNormal);
    float3 lightDirWSNorm = normalize(lightDirWS);
    float diffuseFactor = saturate(dot(normalWS, -lightDirWSNorm));

    // 环境光 + 漫反射
    float3 lightingRGB = ambientColor.rgb + diffuseFactor * lightColor.rgb;

    // 最终颜色：纹理 × 顶点色 × 光照 × 对象tint
    float4 finalColor = baseColor * float4(lightingRGB, 1.0f) * tintColor;

    return finalColor;
}
