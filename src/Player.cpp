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
#include "Sword.h"

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

bool Player::Load(
    ID3D11Device* device, 
    ID3D11DeviceContext* context, 
    const std::string& basePath,
    ID3D11InputLayout* inputLayout,
    ID3D11VertexShader* vertexShader,
    ID3D11PixelShader* pixelShader
) {
    m_meshes.clear();
    m_materials.clear();

    m_constantBuffer = BufferUtils::CreateConstantBuffer(device, sizeof(PerObjectCB));
    m_skinningConstantBuffer = BufferUtils::CreateConstantBuffer(device, sizeof(SkinningCB));

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
            return false;
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
                return false;
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

        if (meshFileName.find("mesh_1") != std::string::npos)
        {
            auto sword = std::make_unique<Sword>();
            std::string offsetFilePath = basePath + "/sword_offset.json";

            if (sword->Load(device, meshFullPath, offsetFilePath, inputLayout, vertexShader, pixelShader)) {
                m_sword = std::move(sword);
            }
           else {
                MessageBoxW(nullptr, L"Failed to load sword", L"Error", MB_OK);
                return false;
            }
        }
        else {
            std::ifstream meshFile(meshFullPath, std::ios::binary);
            if (!meshFile.is_open()) {
                MessageBoxW(nullptr, L"load mesh 失敗", L"Error", MB_OK);
                return false;
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


    }

    // load skeleton
    m_skeleton.bones.clear();
    std::string skeletonPath = basePath + "/skeleton.json";
    std::ifstream skeletonFile(skeletonPath);
    if (!skeletonFile.is_open()) {
        MessageBoxW(nullptr, L"skeleton load 失敗", L"Error", MB_OK);
        return false;
    }
    nlohmann::json skeletonJson;
    skeletonFile >> skeletonJson;
    skeletonFile.close();

    for (const auto& boneNode : skeletonJson["bones"])
    {
        Bone newBone;
        newBone.name = boneNode["name"];
        newBone.parentIndex = boneNode["parentId"];

        std::vector<float> mat_values = boneNode["offset"];
        if (mat_values.size() == 16) {
            memcpy(&newBone.offsetMatrix, mat_values.data(), sizeof(float) * 16);// memcpy(target, source, size)直接复制内存块
        }
        m_skeleton.bones.push_back(newBone);
    }

    

    // load anim
    m_animations.clear();
    int animCount = sceneJson.value("animation_count", 0);// 默认为0
    for (int i = 0; i < animCount; ++i)
    {
        std::string animFilename = "anim_" + std::to_string(i) + ".anim";
        std::string animFullPath = basePath + "/" + animFilename;

        std::ifstream animFile(animFullPath);
        if (!animFile.is_open()) {
            MessageBoxW(nullptr, L"anim load 失敗", L"Error", MB_OK);
            return false;
        }

        nlohmann::json animJson;
        animFile >> animJson;
        animFile.close();

        AnimationClip newClip;
        newClip.name = animJson["name"];
        newClip.duration = animJson["duration"];
        newClip.ticksPerSecond = animJson["ticksPerSecond"];

        if (newClip.name == "Attack0" || newClip.name == "Jump" || newClip.name == "AttackRo") {
            newClip.isLooping = false;
        }
        else {
            newClip.isLooping = true;
        }

        for (const auto& channelNode : animJson["channels"])
        {
            BoneAnimation newChannel;
            newChannel.boneName = channelNode["bone"];
            size_t keyCount = channelNode["posKeys"].size();
            newChannel.keyframes.resize(keyCount);

            for (size_t k = 0; k < keyCount; ++k)
            {
                auto& key = newChannel.keyframes[k];
                const auto& posKey = channelNode["posKeys"][k];
                const auto& rotKey = channelNode["rotKeys"][k];
                const auto& scaleKey = channelNode["scaleKeys"][k];

                key.timeStamp = posKey["t"];
                key.translation = { posKey["x"], posKey["y"], posKey["z"] };
                key.rotationQuaternion = { rotKey["x"], rotKey["y"], rotKey["z"], rotKey["w"] };
                key.scale = { scaleKey["x"], scaleKey["y"], scaleKey["z"] };
            }
            newClip.channels.push_back(newChannel);
        }
        m_animations.push_back(newClip);
    }

    if (!m_skeleton.bones.empty())
    {
        m_finalBoneMatrices.resize(m_skeleton.bones.size());
        for (size_t i = 0; i < m_skeleton.bones.size(); ++i)
        {
            DirectX::XMStoreFloat4x4(&m_finalBoneMatrices[i], DirectX::XMMatrixIdentity());
        }
    }


    SetState(PlayerState::Idle);

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

    if (m_sword && !m_globalTransforms.empty()) {
        DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&m_scale));
        DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rotation));
        DirectX::XMMATRIX modelCorrectionMat = DirectX::XMMatrixRotationY(DirectX::XM_PI);
        DirectX::XMMATRIX finalRotMat = modelCorrectionMat * rotMat;
        DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&m_position));
        DirectX::XMMATRIX playerWorldMatrix = scaleMat * finalRotMat * transMat;

        int handIdx = -1;
        for (int i = 0; i < (int)m_skeleton.bones.size(); ++i) {
            const std::string& n = m_skeleton.bones[i].name;
            if (n.find("RightHand") != std::string::npos || n.find("mixamorig:RightHand") != std::string::npos) {
                handIdx = i;
                break;
            }
        }
        
        DirectX::XMFLOAT4X4 storedHandMatrix = m_globalTransforms[handIdx];
        DirectX::XMMATRIX handModelMatrix = DirectX::XMLoadFloat4x4(&storedHandMatrix);

        DirectX::XMVECTOR S_h, R_h, T_h;
        DirectX::XMMatrixDecompose(&S_h, &R_h, &T_h, handModelMatrix);
        DirectX::XMMATRIX handNoScale =
            DirectX::XMMatrixAffineTransformation(
                DirectX::XMVectorSet(1.f, 1.f, 1.f, 0.f), 
                DirectX::XMVectorZero(),
                R_h, T_h);

      

        DirectX::XMMATRIX offM = m_sword->GetOffsetMatrix();
        DirectX::XMMATRIX Cswap( // Y <-> Z
            1, 0, 0, 0,
            0, 0, 1, 0,   
            0, 1, 0, 0,   
            0, 0, 0, 1
        );
        offM = Cswap * offM * DirectX::XMMatrixInverse(nullptr, Cswap);
        DirectX::XMVECTOR S_o, R_o, T_o;
        DirectX::XMMatrixDecompose(&S_o, &R_o, &T_o, offM);
        DirectX::XMMATRIX offNoScale =
            DirectX::XMMatrixAffineTransformation(
                DirectX::XMVectorSet(1.f, 1.f, 1.f, 0.f),
                DirectX::XMVectorZero(),
                R_o, T_o);

        // 最终
        DirectX::XMMATRIX finalSwordMatrix = offNoScale * handNoScale * playerWorldMatrix;

        m_sword->Render(context, finalSwordMatrix, view, projection, tintColor);
        
    }

    

    
}
void Player::UpdateConstantBuffer(ID3D11DeviceContext* context,
    const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& projection,
    const DirectX::XMFLOAT4& tintColor) 
{
    DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&m_scale));
    DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rotation));
    DirectX::XMMATRIX modelCorrectionMat = DirectX::XMMatrixRotationY(DirectX::XM_PI);
    DirectX::XMMATRIX finalRotMat = modelCorrectionMat * rotMat;
    DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&m_position));
    
    DirectX::XMMATRIX worldMatrix = scaleMat * finalRotMat * transMat;

    DirectX::XMMATRIX worldIT = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));

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



    if (!m_finalBoneMatrices.empty())
    {
        D3D11_MAPPED_SUBRESOURCE mappedSkinning;
        if (SUCCEEDED(context->Map(m_skinningConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSkinning)))
        {
            memcpy(mappedSkinning.pData, m_finalBoneMatrices.data(), m_finalBoneMatrices.size() * sizeof(DirectX::XMFLOAT4X4));
            context->Unmap(m_skinningConstantBuffer.Get(), 0);
        }

        ID3D11Buffer* b2[] = { m_skinningConstantBuffer.Get() };
        context->VSSetConstantBuffers(2, 1, b2);
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


    UpdateAnimation(deltaTime);
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

    int required_size = MultiByteToWideChar(
        CP_UTF8,       // 源字符串是UTF-8编码
        0,             // 默认标志
        str.c_str(),   // 指向源字符串的指针
        (int)str.size(), // 源字符串的长度（字节）
        NULL,          // 输出缓冲区指针，这里为NULL表示查询大小
        0              // 输出缓冲区大小，这里为0
    );

    if (required_size == 0) {
        return std::wstring();
    }

    std::wstring wstr(required_size, 0);

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

int Player::FindKeyframeIndexBefore(float animationTime, const std::vector<Keyframe>& keyframes)
{
    for (int i = (int)keyframes.size() - 1; i >= 0; --i) {
        if (keyframes[i].timeStamp <= animationTime) {
            return i;
        }
    }
    return (int)keyframes.size() - 1;
}

void Player::UpdateAnimation(float deltaTime)
{
    if (m_currentAnimationClipIndex < 0 || m_currentAnimationClipIndex >= m_animations.size()) {
        return;
    }

    if (m_blendFactor < 1.0f) {
        m_blendFactor += deltaTime / m_blendDuration;

        if (m_blendFactor > 1.0f) {
            m_blendFactor = 1.0f;
            m_previousAnimationClipIndex = -1;
        }
    }

    // 骨骼局部变换
    const auto& clip = m_animations[m_currentAnimationClipIndex];

    m_animationTime += clip.ticksPerSecond * deltaTime;
    
    if (!clip.isLooping) {
        if (m_animationTime > clip.duration) {
            m_animationTime = clip.duration;  
            m_isCurrentAnimationFinished = true;
        }
    }
    else {
        if (m_animationTime > clip.duration) {
            m_animationTime = fmod(m_animationTime, clip.duration);
        }
    }
    std::vector<DirectX::XMMATRIX>  currentLocalTransforms(m_skeleton.bones.size());
    for (size_t i = 0; i < m_skeleton.bones.size(); ++i) {
        currentLocalTransforms[i] = DirectX::XMMatrixIdentity();
    }
    for (const auto& channel : clip.channels)
    {
        int boneIndex = -1;
        for (int i = 0; i < m_skeleton.bones.size(); ++i) {
            if (m_skeleton.bones[i].name == channel.boneName) {
                boneIndex = i;
                break;
            }
        }
        if (boneIndex == -1 || channel.keyframes.empty()) continue;

      
        DirectX::XMVECTOR S, R, T;
        if (channel.keyframes.size() == 1) {
            // 默认第一帧
            const auto& firstFrame = channel.keyframes[0];
            T = DirectX::XMLoadFloat3(&firstFrame.translation);
            R = DirectX::XMLoadFloat4(&firstFrame.rotationQuaternion);
            S = DirectX::XMLoadFloat3(&firstFrame.scale);
        }
        else {
            int idx0 = FindKeyframeIndexBefore(m_animationTime, channel.keyframes);
            int idx1 = (idx0 + 1) % channel.keyframes.size();
            const auto& key0 = channel.keyframes[idx0];
            const auto& key1 = channel.keyframes[idx1];
            float deltaTimeKeys = key1.timeStamp - key0.timeStamp;
            if (deltaTimeKeys < 0.0f) deltaTimeKeys += clip.duration;
            float factor = 0.0f;
            if (deltaTimeKeys > 1e-6f) {
                float progress = m_animationTime - key0.timeStamp;
                if (progress < 0.0f) progress += clip.duration;
                factor = progress / deltaTimeKeys;
            }
            factor = std::clamp(factor, 0.0f, 1.0f);
            T = DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&key0.translation), DirectX::XMLoadFloat3(&key1.translation), factor);// (1.0 - t) * V0 + t * V1
            R = DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&key0.rotationQuaternion), DirectX::XMLoadFloat4(&key1.rotationQuaternion), factor);
            S = DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&key0.scale), DirectX::XMLoadFloat3(&key1.scale), factor);
            
        }

        R = DirectX::XMQuaternionNormalize(R);// 消除浮点数累积误差
        DirectX::XMMATRIX localTransform = DirectX::XMMatrixScalingFromVector(S) *
            DirectX::XMMatrixRotationQuaternion(R) *
            DirectX::XMMatrixTranslationFromVector(T);

        currentLocalTransforms[boneIndex] = localTransform;
    }

    // blend
    if (m_blendFactor < 1.0f && m_previousAnimationClipIndex >= 0) {
        const auto& prevClip = m_animations[m_previousAnimationClipIndex];
        std::vector<DirectX::XMMATRIX>  prevLocalTransforms(m_skeleton.bones.size());

        for (size_t i = 0; i < m_skeleton.bones.size(); ++i) {
            prevLocalTransforms[i] = DirectX::XMMatrixIdentity();
        }
        for (const auto& channel : prevClip.channels)
        {
            int boneIndex = -1;
            for (int i = 0; i < m_skeleton.bones.size(); ++i) {
                if (m_skeleton.bones[i].name == channel.boneName) {
                    boneIndex = i;
                    break;
                }
            }
            if (boneIndex == -1 || channel.keyframes.empty()) continue;


            DirectX::XMVECTOR S, R, T;
            if (channel.keyframes.size() == 1) {
                // 默认第一帧
                const auto& firstFrame = channel.keyframes[0];
                T = DirectX::XMLoadFloat3(&firstFrame.translation);
                R = DirectX::XMLoadFloat4(&firstFrame.rotationQuaternion);
                S = DirectX::XMLoadFloat3(&firstFrame.scale);
            }
            else {
                int idx0 = FindKeyframeIndexBefore(m_previousAnimationTime, channel.keyframes);
                int idx1 = (idx0 + 1) % channel.keyframes.size();
                const auto& key0 = channel.keyframes[idx0];
                const auto& key1 = channel.keyframes[idx1];
                float deltaTimeKeys = key1.timeStamp - key0.timeStamp;
                if (deltaTimeKeys < 0.0f) deltaTimeKeys += prevClip.duration;
                float factor = 0.0f;
                if (deltaTimeKeys > 1e-6f) {
                    float progress = m_previousAnimationTime - key0.timeStamp;
                    if (progress < 0.0f) progress += prevClip.duration;
                    factor = progress / deltaTimeKeys;
                }
                factor = std::clamp(factor, 0.0f, 1.0f);
                T = DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&key0.translation), DirectX::XMLoadFloat3(&key1.translation), factor);// (1.0 - t) * V0 + t * V1
                R = DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&key0.rotationQuaternion), DirectX::XMLoadFloat4(&key1.rotationQuaternion), factor);
                S = DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&key0.scale), DirectX::XMLoadFloat3(&key1.scale), factor);

            }

            R = DirectX::XMQuaternionNormalize(R);// 消除浮点数累积误差
            DirectX::XMMATRIX localTransform = DirectX::XMMatrixScalingFromVector(S) *
                DirectX::XMMatrixRotationQuaternion(R) *
                DirectX::XMMatrixTranslationFromVector(T);

            prevLocalTransforms[boneIndex] = localTransform;
        }
        
        
        for (size_t i = 0; i < m_skeleton.bones.size(); i++) {
            DirectX::XMVECTOR prevS, prevR, prevT;
            DirectX::XMMatrixDecompose(&prevS, &prevR, &prevT, prevLocalTransforms[i]);

            DirectX::XMVECTOR currentS, currentR, currentT;
            DirectX::XMMatrixDecompose(&currentS, &currentR, &currentT, currentLocalTransforms[i]);

            DirectX::XMVECTOR finalS = DirectX::XMVectorLerp(prevS, currentS, m_blendFactor);
            DirectX::XMVECTOR finalR = DirectX::XMQuaternionSlerp(prevR, currentR, m_blendFactor);
            DirectX::XMVECTOR finalT = DirectX::XMVectorLerp(prevT, currentT, m_blendFactor);

            currentLocalTransforms[i] = DirectX::XMMatrixScalingFromVector(finalS) * DirectX::XMMatrixRotationQuaternion(finalR) *
                DirectX::XMMatrixTranslationFromVector(finalT);
        }
    }




    // 全局变换(父骨骼继承变换）
    std::vector<DirectX::XMMATRIX> globalTransforms(m_skeleton.bones.size());
    for (int i = 0; i < m_skeleton.bones.size(); ++i)
    {
        int parentIndex = m_skeleton.bones[i].parentIndex;
        if (parentIndex == -1) {
            globalTransforms[i] = currentLocalTransforms[i];
        }
        else {
            globalTransforms[i] = currentLocalTransforms[i] * globalTransforms[parentIndex];
        }
    }

    m_globalTransforms.resize(globalTransforms.size());
    for (size_t i = 0; i < globalTransforms.size(); ++i)
    {
        DirectX::XMStoreFloat4x4(&m_globalTransforms[i], globalTransforms[i]);
    }


    // 发送给GPU的矩阵
    m_finalBoneMatrices.resize(m_skeleton.bones.size());
    for (int i = 0; i < m_skeleton.bones.size(); ++i)
    {
        DirectX::XMMATRIX offset = DirectX::XMLoadFloat4x4(&m_skeleton.bones[i].offsetMatrix);
        DirectX::XMStoreFloat4x4(&m_finalBoneMatrices[i], offset * globalTransforms[i]);
    }
}

void Player::SetState(PlayerState newState) {
    if (newState == m_currentState) return;

    m_previousAnimationClipIndex = m_currentAnimationClipIndex;
    m_previousAnimationTime = m_animationTime;

    std::string targetClipName;
    m_currentState = newState;

    switch (newState) {
        case PlayerState::Idle:
            targetClipName = "Idle";
            break;
        case PlayerState::Run:
            targetClipName = "Run";
            break;
        case PlayerState::Attack0:
            targetClipName = "Attack0";
            break;
        case PlayerState::AttackRo:
            targetClipName = "AttackRo";
            break;
    }

    for (int i = 0; i < m_animations.size(); ++i)
    {
        if (m_animations[i].name == targetClipName)
        {
            m_currentAnimationClipIndex = i;
            m_animationTime = 0.0f;
            m_blendFactor = 0.0f;
            return;
        }
    }   
}


PlayerState Player::GetCurrentState() const {
    return m_currentState;
}

bool Player::IsAnimationFinished() const {
    return m_isCurrentAnimationFinished;
}
void Player::ResetAnimationFinishedFlag() {
    m_isCurrentAnimationFinished = false;
}