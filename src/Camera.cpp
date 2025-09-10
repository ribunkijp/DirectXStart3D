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


Camera::Camera():
    m_position(DirectX::XMFLOAT3(0.0f, 1.0f, -5.0f)),
    m_focusPoint(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f)),
    m_upDirection(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f))
{
    UpdateViewMatrix();
	
}
Camera::~Camera(){}

void Camera::UpdateViewMatrix()
{
    DirectX::XMVECTOR eyePos = XMLoadFloat3(&m_position);
    DirectX::XMVECTOR focusPos = XMLoadFloat3(&m_focusPoint);
    DirectX::XMVECTOR upDir = XMLoadFloat3(&m_upDirection);

    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eyePos, focusPos, upDir);
    XMStoreFloat4x4(&m_viewMatrix, view);
}

DirectX::XMMATRIX Camera::GetViewMatrix() const
{
    return XMLoadFloat4x4(&m_viewMatrix);
}