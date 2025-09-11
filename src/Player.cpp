/**********************************************************************************
* Player.cpp
*
*
*
* LI WENHUI
* 2025/09/10
* *********************************************************************************/

#include "Pch.h"
#include "Player.h"
#include "CommonTypes.h"
#include "BufferUtils.h"

Player::Player() {}
Player::~Player() {}

bool Player::Load(ID3D11Device* device, ID3D11DeviceContext* context) {

    InitVertexData(device, context);
    m_constantBuffer = BufferUtils::CreateConstantBuffer(device, sizeof(PerObjectCB));

    return true;
}
void Player::InitVertexData(ID3D11Device* device, ID3D11DeviceContext* context) {
    Vertex vertices[] =
    {
        // 前 (z=+0.5) - 红色 (R=1, G=0, B=0, A=1)
        { {-0.5f,-0.5f, 0.5f}, {1,0,0,1}, {0.0f,1.0f}, {0,0,1}, { 1,0,0} },
        { {-0.5f, 0.5f, 0.5f}, {1,0,0,1}, {0.0f,0.0f}, {0,0,1}, { 1,0,0} },
        { { 0.5f, 0.5f, 0.5f}, {1,0,0,1}, {1.0f,0.0f}, {0,0,1}, { 1,0,0} },
        { { 0.5f,-0.5f, 0.5f}, {1,0,0,1}, {1.0f,1.0f}, {0,0,1}, { 1,0,0} },

        // 后 (z=-0.5) - 绿色 (R=0, G=1, B=0, A=1)
        { { 0.5f,-0.5f,-0.5f}, {0,1,0,1}, {0.0f,1.0f}, {0,0,-1}, {-1,0,0} },
        { { 0.5f, 0.5f,-0.5f}, {0,1,0,1}, {0.0f,0.0f}, {0,0,-1}, {-1,0,0} },
        { {-0.5f, 0.5f,-0.5f}, {0,1,0,1}, {1.0f,0.0f}, {0,0,-1}, {-1,0,0} },
        { {-0.5f,-0.5f,-0.5f}, {0,1,0,1}, {1.0f,1.0f}, {0,0,-1}, {-1,0,0} },

        // 左 (x=-0.5) - 橙色 (R=1, G=0.5, B=0, A=1)
        { {-0.5f,-0.5f,-0.5f}, {1,0.5,0,1}, {0.0f,1.0f}, {-1,0,0}, {0,0, 1} },
        { {-0.5f, 0.5f,-0.5f}, {1,0.5,0,1}, {0.0f,0.0f}, {-1,0,0}, {0,0, 1} },
        { {-0.5f, 0.5f, 0.5f}, {1,0.5,0,1}, {1.0f,0.0f}, {-1,0,0}, {0,0, 1} },
        { {-0.5f,-0.5f, 0.5f}, {1,0.5,0,1}, {1.0f,1.0f}, {-1,0,0}, {0,0, 1} },

        // 右 (x=+0.5) - 紫色 (R=0.5, G=0, B=0.5, A=1)
        { { 0.5f,-0.5f, 0.5f}, {0.5,0,0.5,1}, {0.0f,1.0f}, {1,0,0}, {0,0,-1} },
        { { 0.5f, 0.5f, 0.5f}, {0.5,0,0.5,1}, {0.0f,0.0f}, {1,0,0}, {0,0,-1} },
        { { 0.5f, 0.5f,-0.5f}, {0.5,0,0.5,1}, {1.0f,0.0f}, {1,0,0}, {0,0,-1} },
        { { 0.5f,-0.5f,-0.5f}, {0.5,0,0.5,1}, {1.0f,1.0f}, {1,0,0}, {0,0,-1} },

        // 上 (y=+0.5) - 蓝色 (R=0, G=0, B=1, A=1)
        { {-0.5f, 0.5f, 0.5f}, {0,0,1,1}, {0.0f,1.0f}, {0,1,0}, { 1,0,0} },
        { {-0.5f, 0.5f,-0.5f}, {0,0,1,1}, {0.0f,0.0f}, {0,1,0}, { 1,0,0} },
        { { 0.5f, 0.5f,-0.5f}, {0,0,1,1}, {1.0f,0.0f}, {0,1,0}, { 1,0,0} },
        { { 0.5f, 0.5f, 0.5f}, {0,0,1,1}, {1.0f,1.0f}, {0,1,0}, { 1,0,0} },

        // 下 (y=-0.5) - 粉色 (R=1, G=0.7, B=0.8, A=1)
        { {-0.5f,-0.5f,-0.5f}, {1,0.7,0.8,1}, {0.0f,1.0f}, {0,-1,0}, { 1,0,0} },
        { {-0.5f,-0.5f, 0.5f}, {1,0.7,0.8,1}, {0.0f,0.0f}, {0,-1,0}, { 1,0,0} },
        { { 0.5f,-0.5f, 0.5f}, {1,0.7,0.8,1}, {1.0f,0.0f}, {0,-1,0}, { 1,0,0} },
        { { 0.5f,-0.5f,-0.5f}, {1,0.7,0.8,1}, {1.0f,1.0f}, {0,-1,0}, { 1,0,0} },
    };
    const UINT indices[] =
    {
        0,1,2, 0,2,3,       // 前
        4,5,6, 4,6,7,       // 后
        8,9,10, 8,10,11,    // 左
        12,13,14, 12,14,15, // 右
        16,17,18, 16,18,19, // 上
        20,21,22, 20,22,23  // 下
    };

    m_vertexBuffer = BufferUtils::CreateDynamicVertexBuffer(device, sizeof(vertices));
    m_indexBuffer = BufferUtils::CreateDynamicIndexBuffer(device, sizeof(indices));
    m_indexCount = _countof(indices);

    D3D11_MAPPED_SUBRESOURCE mapped{};
    context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, vertices, sizeof(vertices));
    context->Unmap(m_vertexBuffer.Get(), 0);

    context->Map(m_indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, indices, sizeof(indices));
    context->Unmap(m_indexBuffer.Get(), 0);

}


void Player::Render(ID3D11DeviceContext* context, const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& projection, const DirectX::XMFLOAT4& tintColor) {
    UpdateConstantBuffer(context, view, projection, tintColor);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ID3D11Buffer* vbs[] = { m_vertexBuffer.Get() };
    context->IASetVertexBuffers(0, 1, vbs, &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    ID3D11Buffer* b0[] = { m_constantBuffer.Get() };
    context->VSSetConstantBuffers(0, 1, b0);
    context->PSSetConstantBuffers(0, 1, b0);
    ID3D11ShaderResourceView* srvs[] = { nullptr };
    context->PSSetShaderResources(0, 1, srvs);

    context->DrawIndexed(m_indexCount, 0, 0);
}
void Player::UpdateConstantBuffer(ID3D11DeviceContext* context,
    const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& projection,
    const DirectX::XMFLOAT4& tintColor) 
{
    DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&m_scale));
    DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rotation));
    DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&m_position));
    DirectX::XMMATRIX worldMatrix = scaleMat * rotMat * transMat;

    DirectX::XMMATRIX worldIT = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
    {
        auto* cb = reinterpret_cast<PerObjectCB*>(mappedResource.pData);

        DirectX::XMStoreFloat4x4(&cb->world, DirectX::XMMatrixTranspose(worldMatrix));
        DirectX::XMStoreFloat4x4(&cb->view, DirectX::XMMatrixTranspose(view));
        DirectX::XMStoreFloat4x4(&cb->projection, DirectX::XMMatrixTranspose(projection));
        DirectX::XMStoreFloat4x4(&cb->worldIT, worldIT);
        cb->tintColor = tintColor;
       
        context->Unmap(m_constantBuffer.Get(), 0);
    }
}

void Player::SetRotationY(float yaw)
{
    m_rotation.y = yaw;
}

DirectX::XMFLOAT3 Player::GetPosition() const
{
    return m_position;
}

DirectX::XMFLOAT3 Player::GetRotation() const
{
    return m_rotation;
}

void Player::SetTargetVelocity(const DirectX::XMFLOAT3& targetVelocity)
{
    m_targetVelocity = targetVelocity;
   
}

void Player::Update(float deltaTime)
{
    DirectX::XMVECTOR currentVel = DirectX::XMLoadFloat3(&m_velocity);
    DirectX::XMVECTOR targetVel = DirectX::XMLoadFloat3(&m_targetVelocity);
    // 线性插值(Lerp)
    const float smoothingFactor = 0.15f;
    currentVel = DirectX::XMVectorLerp(currentVel, targetVel, smoothingFactor);

    const float EpsilonSq = (1e-4f) * (1e-4f);
    if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(currentVel)) < EpsilonSq)
    {
        currentVel = DirectX::XMVectorZero();// 清零
    }

    DirectX::XMStoreFloat3(&m_velocity, currentVel);
    
    DirectX::XMVECTOR posVec = DirectX::XMLoadFloat3(&m_position);
    posVec = DirectX::XMVectorAdd(posVec, DirectX::XMVectorScale(currentVel, deltaTime));
    DirectX::XMStoreFloat3(&m_position, posVec);
}