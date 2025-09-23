/**********************************************************************************
* Ground.cpp
*
*
*
* LI WENHUI
* 2025/09/12
* *********************************************************************************/

#include "Pch.h"
#include "Ground.h"
#include "CommonTypes.h"
#include "BufferUtils.h"

Ground::Ground() {
	DirectX::XMStoreFloat4x4(&m_worldMatrix, DirectX::XMMatrixIdentity());
}
Ground::~Ground() {

}

// Ground.cpp

// ... (Ground::Ground() 和 Ground::~Ground() 保持不变) ...

bool Ground::Initialize(ID3D11Device* device, int numLines, float size)
{
    if (numLines % 2 != 0) {
        numLines++;
    }

    std::vector<Vertex> vertices;
    std::vector<UINT> indices;

    float halfSize = size / 2.0f;
    float lineSpacing = size / numLines;
    DirectX::XMFLOAT4 lineColor(1.0f, 0.5f, 0.0f, 1.0f);

    for (int i = 0; i <= numLines; ++i)
    {
        float pos = -halfSize + i * lineSpacing;

        vertices.push_back({ {pos, 0.0f, -halfSize}, lineColor, {0,0}, {0,1,0}, {1,0,0} });
        vertices.push_back({ {pos, 0.0f,  halfSize}, lineColor, {0,0}, {0,1,0}, {1,0,0} });

        vertices.push_back({ {-halfSize, 0.0f, pos}, lineColor, {0,0}, {0,1,0}, {1,0,0} });
        vertices.push_back({ { halfSize, 0.0f, pos}, lineColor, {0,0}, {0,1,0}, {1,0,0} });
    }

    m_indexCount = static_cast<UINT>(vertices.size());
    for (UINT i = 0; i < m_indexCount; ++i) {
        indices.push_back(i);
    }

    m_vertexBuffer = BufferUtils::CreateStaticVertexBuffer(device, vertices.data(), sizeof(Vertex) * vertices.size());
    m_indexBuffer = BufferUtils::CreateStaticIndexBuffer(device, indices.data(), sizeof(UINT) * indices.size());
    if (!m_vertexBuffer || !m_indexBuffer) return false;

    m_constantBuffer = BufferUtils::CreateConstantBuffer(device, sizeof(PerObjectCB));
    if (!m_constantBuffer) return false;

    return true;
}

void Ground::Render(ID3D11DeviceContext* context,
    const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& projection,
    const DirectX::XMFLOAT4& tintColor)
{
    UpdateConstantBuffer(context, view, projection, tintColor);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    context->DrawIndexed(m_indexCount, 0, 0);
}

void Ground::UpdateConstantBuffer(ID3D11DeviceContext* context,
    const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& projection,
    const DirectX::XMFLOAT4& tintColor)
{
    DirectX::XMMATRIX worldMatrix = DirectX::XMLoadFloat4x4(&m_worldMatrix);

    DirectX::XMMATRIX worldIT = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, worldMatrix));

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

    ID3D11Buffer* b0[] = { m_constantBuffer.Get() };
    context->VSSetConstantBuffers(0, 1, b0);
    context->PSSetConstantBuffers(0, 1, b0);
}
