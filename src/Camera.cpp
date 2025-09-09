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

using namespace DirectX;

Camera::Camera():
    m_position(XMFLOAT3(0.0f, 1.0f, -5.0f)),
    m_focusPoint(XMFLOAT3(0.0f, 0.0f, 0.0f)),
    m_upDirection(XMFLOAT3(0.0f, 1.0f, 0.0f))
{
    UpdateViewMatrix();
	
}
Camera::~Camera(){}

void Camera::UpdateViewMatrix()
{
    XMVECTOR eyePos = XMLoadFloat3(&m_position);
    XMVECTOR focusPos = XMLoadFloat3(&m_focusPoint);
    XMVECTOR upDir = XMLoadFloat3(&m_upDirection);

    XMMATRIX view = XMMatrixLookAtLH(eyePos, focusPos, upDir);
    XMStoreFloat4x4(&m_viewMatrix, view);
}

XMMATRIX Camera::GetViewMatrix() const
{
    return XMLoadFloat4x4(&m_viewMatrix);
}