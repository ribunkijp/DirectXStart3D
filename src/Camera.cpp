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
    UpdateViewMatrix();
}
Camera::~Camera(){}

void Camera::UpdateViewMatrix()
{
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0);
    DirectX::XMVECTOR forward = DirectX::XMVector3TransformCoord({ 0.0f, 0.0f, 1.0f, 0.0f }, rotationMatrix);
    DirectX::XMVECTOR eyePos = DirectX::XMLoadFloat3(&m_position);
    DirectX::XMVECTOR focusPos = DirectX::XMVectorAdd(eyePos, forward);
    DirectX::XMVECTOR upDirection = DirectX::XMLoadFloat3(&m_upDirection);

    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eyePos, focusPos, upDirection);
    DirectX::XMStoreFloat4x4(&m_view, view);
}

void Camera::Rotate(float deltaYaw, float deltaPitch)
{
    //m_yaw += deltaYaw;
    m_pitch += deltaPitch;

    constexpr float pitchLimit = DirectX::XM_PIDIV2 - 0.01f;
    m_pitch = std::clamp(m_pitch, -pitchLimit, pitchLimit);
}

DirectX::XMMATRIX Camera::GetViewMatrix() const
{
    return DirectX::XMLoadFloat4x4(&m_view);
}

void Camera::UpdateThirdPerson(const DirectX::XMFLOAT3& playerPosition, float playerYaw)
{
    float distance = 8.0f; // 像机和玩家距离
    float height = 3.0f;   // 像机和玩家高度差

    m_position.x = playerPosition.x - sinf(playerYaw) * distance;
    m_position.z = playerPosition.z - cosf(playerYaw) * distance;
    m_position.y = playerPosition.y + height;

    DirectX::XMFLOAT3 focus = playerPosition;
    focus.y += 1.5f;

    DirectX::XMVECTOR eyePos = DirectX::XMLoadFloat3(&m_position);
    DirectX::XMVECTOR focusPos = DirectX::XMLoadFloat3(&focus);
    DirectX::XMVECTOR upDirection = DirectX::XMLoadFloat3(&m_upDirection); // {0,1,0}

    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eyePos, focusPos, upDirection);

    DirectX::XMMATRIX pitchRotation = DirectX::XMMatrixRotationX(m_pitch);
    view = DirectX::XMMatrixMultiply(pitchRotation, view);

    DirectX::XMStoreFloat4x4(&m_view, view);
}
