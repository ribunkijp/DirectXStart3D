/**********************************************************************************
 * CommenTypes.h
 *
 *
 *
 * LI WENHUI
 * 2025/09/10
 **********************************************************************************/

#ifndef COMMONTYPES_H
#define COMMONTYPES_H

struct Vertex
{
    DirectX::XMFLOAT3 position;  
    DirectX::XMFLOAT4 color;     
    DirectX::XMFLOAT2 texcoord;  
    DirectX::XMFLOAT3 normal;   
    DirectX::XMFLOAT3 tangent;//切线 (Tangent) 的方向 = 纹理U坐标增加的方向  
};

struct PresentVertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
    DirectX::XMFLOAT2 texcoord;
};

struct PerObjectCB
{
    DirectX::XMFLOAT4X4 world;      
    DirectX::XMFLOAT4X4 view;       
    DirectX::XMFLOAT4X4 projection; 
    DirectX::XMFLOAT4X4 worldIT; // world 的逆转置，用于法线/切线
    DirectX::XMFLOAT4   tintColor; // 染色/透明度
};

struct PerFrameCB
{
    DirectX::XMFLOAT3 lightDirWS; float _pad0 = 0.0f;
    DirectX::XMFLOAT4 ambientColor{ 0.1f,0.1f,0.1f,1.0f }; // 环境光
    DirectX::XMFLOAT4 lightColor{ 1.0f,1.0f,1.0f,1.0f };
};


#endif