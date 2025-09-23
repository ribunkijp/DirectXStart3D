/**********************************************************************************
* Sword.cpp
*
*
*
* LI WENHUI
* 2025/09/22
* *********************************************************************************/

#include "Pch.h"
#include "Sword.h"
#include "BufferUtils.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include "CommonTypes.h"

struct MeshHeader {
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t materialIndex;
};

Sword::Sword() {
    
}
Sword::~Sword() {}

bool Sword::Load(
    ID3D11Device* device,
    const std::string& meshPath,
    const std::string& offsetPath,
    ID3D11InputLayout* inputLayout,
    ID3D11VertexShader* vertexShader,
    ID3D11PixelShader* pixelShader
) {
    std::ifstream meshFile(meshPath, std::ios::binary);
    if (!meshFile.is_open()) {
        MessageBoxW(nullptr, L"load sword mesh 失敗", L"Error", MB_OK);
        return false;
    }

    MeshHeader header;
    meshFile.read(reinterpret_cast<char*>(&header), sizeof(MeshHeader));// 获取顶点和索引大小
    std::vector<modelVertex> vertices(header.vertexCount);
    meshFile.read(reinterpret_cast<char*>(vertices.data()), header.vertexCount * sizeof(modelVertex));
    std::vector<uint32_t>indices(header.indexCount);
    meshFile.read(reinterpret_cast<char*>(indices.data()), header.indexCount * sizeof(uint32_t));
    meshFile.close();

    DirectX::XMFLOAT3 vmin(FLT_MAX, FLT_MAX, FLT_MAX);
    DirectX::XMFLOAT3 vmax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    DirectX::XMFLOAT3 sum(0, 0, 0);

    for (auto& v : vertices) {
        vmin.x = std::min(vmin.x, v.position.x);
        vmin.y = std::min(vmin.y, v.position.y);
        vmin.z = std::min(vmin.z, v.position.z);
        vmax.x = std::max(vmax.x, v.position.x);
        vmax.y = std::max(vmax.y, v.position.y);
        vmax.z = std::max(vmax.z, v.position.z);
        sum.x += v.position.x;
        sum.y += v.position.y;
        sum.z += v.position.z;
    }
    const float invN = vertices.empty() ? 0.0f : (1.0f / vertices.size());
    DirectX::XMFLOAT3 centroid(sum.x * invN, sum.y * invN, sum.z * invN);

    // 写法 A：显式 4×4
   /* m_offsetMatrix = DirectX::XMFLOAT4X4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        -centroid.x, -centroid.y, -centroid.z, 1);*/

    // 方便你在 VS 输出窗口看
    wchar_t buf[256];
    swprintf(buf, 256, L"[Sword Mesh] AABB min(%.3f,%.3f,%.3f)  max(%.3f,%.3f,%.3f)  centroid(%.3f,%.3f,%.3f)\n",
        vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z, centroid.x, centroid.y, centroid.z);
    OutputDebugStringW(buf);

    
    m_indexCount = header.indexCount;
    m_vertexBuffer = BufferUtils::CreateStaticVertexBuffer(device, vertices.data(), vertices.size() * sizeof(modelVertex));
    m_indexBuffer = BufferUtils::CreateStaticIndexBuffer(device, indices.data(), indices.size() * sizeof(uint32_t));

    std::ifstream offsetFile(offsetPath);
    if (!offsetFile.is_open()) {
        MessageBoxW(nullptr, L"load sword_offset.json 失敗", L"Error", MB_OK);
        return false;
    }
    nlohmann::json offsetJson;
    offsetFile >> offsetJson;
    offsetFile.close();

    std::vector<std::vector<float>> matrixData = offsetJson["offset_matrix_4x4"];
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m_offsetMatrix.m[i][j] = matrixData[i][j];
        }
    }


    m_constantBuffer = BufferUtils::CreateConstantBuffer(device, sizeof(PerObjectCB));

    m_inputLayout = inputLayout;
    m_vertexShader = vertexShader;
    m_pixelShader = pixelShader;

    return true;
}

DirectX::XMMATRIX Sword::GetOffsetMatrix() {
    return XMLoadFloat4x4(&m_offsetMatrix);
}

void Sword::UpdateConstantBuffer(ID3D11DeviceContext* context, const DirectX::XMMATRIX& worldMatrix, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& projection, const DirectX::XMFLOAT4& tintColor)
{
    DirectX::XMMATRIX worldIT = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, worldMatrix));

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
    {
        auto* cb = reinterpret_cast<PerObjectCB*>(mappedResource.pData);

        DirectX::XMStoreFloat4x4(&cb->world, worldMatrix);
        DirectX::XMStoreFloat4x4(&cb->view, view);
        DirectX::XMStoreFloat4x4(&cb->projection, projection);

        DirectX::XMStoreFloat4x4(&cb->worldIT, worldIT);

        cb->tintColor = tintColor;


        context->Unmap(m_constantBuffer.Get(), 0);
    }

    ID3D11Buffer* b0[] = { m_constantBuffer.Get() };
    context->VSSetConstantBuffers(0, 1, b0);
    context->PSSetConstantBuffers(0, 1, b0);
}

void Sword::Render(ID3D11DeviceContext* context, const DirectX::XMMATRIX& worldMatrix, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& projection, const DirectX::XMFLOAT4& tintColor)
{
    UpdateConstantBuffer(context, worldMatrix, view, projection, tintColor);

    UINT stride = sizeof(modelVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(m_inputLayout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    context->DrawIndexed(m_indexCount, 0, 0);
}