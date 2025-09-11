/**********************************************************************************
 * Camera.cpp
 *
 *
 *
 * LI WENHUI
 * 2025/09/09
 **********************************************************************************/

#include "Pch.h"
#include "Camera.h"


Camera::Camera() {
    
}
Camera::~Camera(){}


void Camera::Rotate(float deltaYaw, float deltaPitch)
{
    m_yaw += deltaYaw;
    m_pitch += deltaPitch;
}

DirectX::XMMATRIX Camera::GetViewMatrix() const
{
    return DirectX::XMLoadFloat4x4(&m_view);
}

void Camera::Update(const DirectX::XMFLOAT3& targetPosition)
{
    const float baseDistance = 12.0f;  
    const float pivotHeight = 1.5f;  

    constexpr float minPitchRadians = DirectX::XMConvertToRadians(-89.0f);
    constexpr float maxPitchRadians = DirectX::XMConvertToRadians(89.0f);
    if (m_pitch < minPitchRadians) m_pitch = minPitchRadians;
    if (m_pitch > maxPitchRadians) m_pitch = maxPitchRadians;

   
    m_focusPos = { targetPosition.x, targetPosition.y + pivotHeight, targetPosition.z };


    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f);
    DirectX::XMVECTOR forwardDirectionVec =
        DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotationMatrix);
    forwardDirectionVec = DirectX::XMVector3Normalize(forwardDirectionVec);


    float upness = 0.5f * (DirectX::XMVectorGetY(forwardDirectionVec) + 1.0f);
    upness = std::clamp(upness, 0.0f, 1.0f);
    float upnessEmphasized = powf(upness, 3.0f);

    const float distanceAdjustStrength = 0.70f;
    float adjustedDistance = baseDistance * (1.0f - distanceAdjustStrength * upnessEmphasized); 
    adjustedDistance = std::clamp(adjustedDistance, baseDistance * 0.30f, baseDistance * 1.45f);


   
    DirectX::XMVECTOR focusPositionVector = DirectX::XMLoadFloat3(&m_focusPos);
    DirectX::XMVECTOR cameraPositionVector = DirectX::XMVectorSubtract(
        focusPositionVector,
        DirectX::XMVectorScale(forwardDirectionVec, adjustedDistance)
    );


    DirectX::XMStoreFloat3(&m_position, cameraPositionVector);

    DirectX::XMVECTOR upDirectionVector = DirectX::XMLoadFloat3(&m_upDirection);
    upDirectionVector = DirectX::XMVector3Normalize(upDirectionVector);

    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(
        cameraPositionVector,
        focusPositionVector,
        upDirectionVector
    );

    DirectX::XMStoreFloat4x4(&m_view, viewMatrix);
}

float Camera::GetYaw() const { return m_yaw; }