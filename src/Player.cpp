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
#include "nlohmann/json.hpp"
#include "TextureLoader.h"

#include <fstream>
#include <locale>

struct MeshHeader {
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t materialIndex;
};

std::wstring ConvertUtf8ToWstring(const std::string& str);


Player::Player() {}
Player::~Player() {}

bool Player::Load(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& basePath) {
    m_meshes.clear();
    m_materials.clear();

    m_constantBuffer = BufferUtils::CreateConstantBuffer(device, sizeof(PerObjectCB));

    std::string scenePath = basePath + "/scene.json";
    std::ifstream sceneFile(scenePath);
    if (!sceneFile.is_open()) {
        MessageBoxW(nullptr, L"load scene.json 失敗", L"Error", MB_OK);
        return false;
    }
    nlohmann::json sceneJson;
    sceneFile >> sceneJson; // 自动读取整个文件
    sceneFile.close();

    // load material texture
    auto materialsNode = sceneJson["materials"]; 
    for (const auto& materialNode : materialsNode) {
        std::string materialFileName = materialNode;
        std::string materialFullPath = basePath + "/" + materialFileName;

        std::ifstream materialFile(materialFullPath);
        if (!materialFile.is_open()) {
            MessageBoxW(nullptr, L"load materialFullPath 失敗", L"Error", MB_OK);
            continue;
        }
        nlohmann::json materialJson;
        materialFile >> materialJson;
        materialFile.close();

        Material newMaterial;
        if (materialJson.contains("diffuseTexture")) {
            std::string textureFileName = materialJson["diffuseTexture"];
            std::string textureFullPath = basePath + "/" + textureFileName;

            std::wstring widePath = ConvertUtf8ToWstring(textureFullPath);

            HRESULT hr = TextureLoader::LoadTextureAndCreateSRV(
                device,                    
                widePath.c_str(),           
                newMaterial.diffuseTexture, 
                nullptr,                    
                nullptr                     
            );
            if (FAILED(hr))
            {
                MessageBoxW(nullptr, L"texture load 失敗", L"Error", MB_OK);
            }
        }
        m_materials.push_back(std::move(newMaterial));
    }

    // load mesh
    auto meshesNode = sceneJson["meshes"];
    for (const auto& meshNode : meshesNode) {
        std::string meshFileName = meshNode["file"];
        uint32_t materialIndex = meshNode["materialIndex"];
        std::string meshFullPath = basePath + "/" + meshFileName;

        std::ifstream meshFile(meshFullPath, std::ios::binary);
        if (!meshFile.is_open()) {
            MessageBoxW(nullptr, L"load mesh 失敗", L"Error", MB_OK);
        }

        MeshHeader header;
        meshFile.read(reinterpret_cast<char*>(&header), sizeof(MeshHeader));// 获取顶点和索引大小
        std::vector<modelVertex> vertices(header.vertexCount);
        meshFile.read(reinterpret_cast<char*>(vertices.data()), header.vertexCount * sizeof(modelVertex));
        std::vector<uint32_t>indices(header.indexCount);
        meshFile.read(reinterpret_cast<char*>(indices.data()), header.indexCount * sizeof(uint32_t));
        meshFile.close();

        Mesh newMesh;
        newMesh.indexCount = header.indexCount;
        newMesh.materialIndex = materialIndex;
        newMesh.vertexBuffer = BufferUtils::CreateStaticVertexBuffer(device, vertices.data(), header.vertexCount * sizeof(modelVertex));
        newMesh.indexBuffer = BufferUtils::CreateStaticIndexBuffer(device, indices.data(), header.indexCount * sizeof(uint32_t));

        if (!newMesh.vertexBuffer || !newMesh.indexBuffer) return false;

        m_meshes.push_back(std::move(newMesh));
    }

    return true;
}



void Player::Render(ID3D11DeviceContext* context, const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& projection, const DirectX::XMFLOAT4& tintColor) {
    UpdateConstantBuffer(context, view, projection, tintColor);

    UINT stride = sizeof(modelVertex);
    UINT offset = 0;

    for (const auto& mesh : m_meshes) {
        ID3D11Buffer* vbs[] = { mesh.vertexBuffer.Get() };
        context->IASetVertexBuffers(0, 1, vbs, &stride, &offset);
        context->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        if (mesh.materialIndex < m_materials.size())
        {
            const auto& mat = m_materials[mesh.materialIndex];
            ID3D11ShaderResourceView* srvs[] = { mat.diffuseTexture.Get() };
            context->PSSetShaderResources(0, 1, srvs);
        }

        context->DrawIndexed(mesh.indexCount, 0, 0);
    }
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

    ID3D11Buffer* b0[] = { m_constantBuffer.Get() };
    context->VSSetConstantBuffers(0, 1, b0);
    context->PSSetConstantBuffers(0, 1, b0);
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

DirectX::XMFLOAT3 Player::GetVelocity() const {
    return m_velocity;
}

DirectX::XMFLOAT3 Player::GetTargetVelocity() const {
    return m_targetVelocity;
}


std::wstring ConvertUtf8ToWstring(const std::string& str)
{
    if (str.empty()) {
        return std::wstring();
    }

    // 第一步：调用 MultiByteToWideChar 计算转换后的字符串需要多大的缓冲区。
    // 我们传入 NULL 作为输出缓冲区，函数会返回所需的字符数。
    int required_size = MultiByteToWideChar(
        CP_UTF8,       // 源字符串是UTF-8编码
        0,             // 默认标志
        str.c_str(),   // 指向源字符串的指针
        (int)str.size(), // 源字符串的长度（字节）
        NULL,          // 输出缓冲区指针，这里为NULL表示查询大小
        0              // 输出缓冲区大小，这里为0
    );

    // 如果计算失败，返回空字符串
    if (required_size == 0) {
        return std::wstring();
    }

    // 第二步：创建一个足够大的 wstring 来接收转换后的数据。
    std::wstring wstr(required_size, 0);

    // 第三步：再次调用 MultiByteToWideChar，这次执行真正的转换。
    MultiByteToWideChar(
        CP_UTF8,
        0,
        str.c_str(),
        (int)str.size(),
        &wstr[0],      // 指向目标wstring缓冲区的指针
        required_size  // 目标缓冲区的大小
    );

    return wstr;
}